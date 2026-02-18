/*
 * MIT License
 *
 * Copyright (c) 2026 Diego Perez
 *
 * Optical RX Decoder - Manchester Decoding Implementation
 * Receives time sync and config updates via phone screen flashing
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "comms_face.h"
#include "comms_rx.h"
#include "watch.h"

#ifdef HAS_IR_SENSOR
#include "adc.h"

// RX decoder constants
#define RX_SYNC_BYTE 0xAA                // Sync pattern (10101010)
#define RX_MAX_PACKET_SIZE 68            // Max packet: SYNC + LEN + TYPE + DATA(64) + CRC8
#define RX_BIT_TIMEOUT_TICKS 32          // 500ms @ 64 Hz (missed bit timeout)
#define RX_PACKET_TIMEOUT_TICKS 7680     // 2 minutes @ 64 Hz
#define RX_CALIBRATION_SAMPLES 64        // Samples collected during light threshold calibration
#define RX_SYNC_TIMEOUT_TICKS 640        // 10 seconds @ 64 Hz (sync search timeout)

// Sync detection parameters.
// Manchester at 16 bps / 64 Hz: midpoint transitions occur every 4 ticks (one per bit period).
#define SYNC_TICKS_PER_EDGE 4            // Expected ticks between consecutive sync transitions
#define SYNC_TICKS_TOLERANCE 1           // Acceptable jitter (+/- ticks)
#define SYNC_EDGES_NEEDED 6              // Consecutive good-spaced edges required for sync lock

// Packet types
#define PACKET_TYPE_TIME_SYNC 0x01
#define PACKET_TYPE_CONFIG 0x02
#define PACKET_TYPE_ACK 0x03

// CRC-8/MAXIM calculation
static uint8_t crc8(const uint8_t *data, size_t len) {
    uint8_t crc = 0;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x01) {
                crc = (crc >> 1) ^ 0x8C;  // Polynomial 0x31 reversed
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

// Enable light sensor
void optical_rx_enable(comms_face_state_t *state) {
#ifdef HAS_IR_SENSOR
    HAL_GPIO_IR_ENABLE_out();
    HAL_GPIO_IR_ENABLE_clr();
    HAL_GPIO_IRSENSE_pmuxen(HAL_GPIO_PMUX_ADC);
    adc_init();
    adc_enable();
    state->light_sensor_active = true;
#else
    (void)state;
#endif
}

// Disable light sensor
void optical_rx_disable(comms_face_state_t *state) {
#ifdef HAS_IR_SENSOR
    adc_disable();
    HAL_GPIO_IRSENSE_pmuxdis();
    HAL_GPIO_IRSENSE_off();
    HAL_GPIO_IR_ENABLE_off();
    state->light_sensor_active = false;
#else
    (void)state;
#endif
}

// Read light sensor value
static uint16_t read_light_level(void) {
#ifdef HAS_IR_SENSOR
    return adc_get_analog_value(HAL_GPIO_IRSENSE_pin());
#else
    return 0;
#endif
}

// Calibrate threshold: sample once at startup, use current reading as midpoint.
// The previous implementation had a 64-iteration delay_ms(16) loop that blocked
// for ~1024 ms; a single snapshot is sufficient and avoids blocking the scheduler.
void optical_rx_calibrate(comms_face_state_t *state) {
    state->rx_state.light_threshold = read_light_level();
}

// Decode Manchester bit from first and second half samples.
// Manchester convention (IEEE 802.3): LOW->HIGH = 1, HIGH->LOW = 0.
// Returns: 1, 0, or -1 (no midpoint transition -- framing error).
static int8_t decode_manchester_bit(bool first_half, bool second_half) {
    if (!first_half && second_half) {
        return 1;  // LOW -> HIGH = 1
    } else if (first_half && !second_half) {
        return 0;  // HIGH -> LOW = 0
    } else {
        return -1;  // No transition at midpoint -- framing error
    }
}

// Start RX reception
void optical_rx_start(comms_face_state_t *state) {
    // Reset RX state
    memset(&state->rx_state, 0, sizeof(optical_rx_state_t));
    state->bytes_received = 0;

    // Enable light sensor
    optical_rx_enable(state);

    // Calibrate threshold (single sample -- non-blocking)
    optical_rx_calibrate(state);

    // Set mode to active
    state->mode = COMMS_MODE_RX_ACTIVE;

    // Request fast tick rate for polling
    movement_request_tick_frequency(64);  // 64 Hz for light sampling
}

// Stop RX reception
void optical_rx_stop(comms_face_state_t *state) {
    // Disable light sensor
    optical_rx_disable(state);

    // Reset tick frequency
    movement_request_tick_frequency(1);  // Back to 1 Hz

    // Return to idle
    state->mode = COMMS_MODE_IDLE;
}

// Process received packet
static void process_packet(comms_face_state_t *state) {
    if (state->bytes_received < 3) {
        // Too short (need at least LEN + TYPE + CRC8)
        state->mode = COMMS_MODE_RX_ERROR;
        return;
    }

    uint8_t len  = state->rx_state.rx_buffer[0];
    uint8_t type = state->rx_state.rx_buffer[1];

    // Validate length
    if (len > 64 || len + 3 != state->bytes_received) {
        state->mode = COMMS_MODE_RX_ERROR;
        return;
    }

    // Validate CRC8
    uint8_t expected_crc   = state->rx_state.rx_buffer[state->bytes_received - 1];
    uint8_t calculated_crc = crc8(state->rx_state.rx_buffer, state->bytes_received - 1);

    if (expected_crc != calculated_crc) {
        state->mode = COMMS_MODE_RX_ERROR;
        return;
    }

    // Process based on type
    switch (type) {
        case PACKET_TYPE_TIME_SYNC:
            if (len == 6) {
                // Extract timestamp (4 bytes, little-endian)
                uint32_t timestamp =
                    ((uint32_t)state->rx_state.rx_buffer[2] <<  0) |
                    ((uint32_t)state->rx_state.rx_buffer[3] <<  8) |
                    ((uint32_t)state->rx_state.rx_buffer[4] << 16) |
                    ((uint32_t)state->rx_state.rx_buffer[5] << 24);

                // Extract timezone offset (2 bytes, little-endian, signed)
                int16_t tz_offset =
                    ((int16_t)state->rx_state.rx_buffer[6] << 0) |
                    ((int16_t)state->rx_state.rx_buffer[7] << 8);

                // tz_offset is received from the sender but NOT applied here.
                // watch_rtc_set_unix_time() expects a raw UTC Unix timestamp;
                // the RTC hardware converts to local time internally.  Adding
                // tz_offset here would cause double-correction (e.g. in AKST
                // the watch would show 01:00 instead of 10:00).
                (void)tz_offset;  // retained for future use / logging

                // Validate hour and minute before committing to RTC.
                // A Unix timestamp is always non-negative and fully determined, so
                // an invalid hour/minute indicates corrupt data -- reject the write.
                uint32_t time_of_day = timestamp % 86400UL;
                uint8_t  hour        = (uint8_t)(time_of_day / 3600U);
                uint8_t  minute      = (uint8_t)((time_of_day % 3600U) / 60U);

                if (hour > 23 || minute > 59) {
                    state->mode = COMMS_MODE_RX_ERROR;
                    break;
                }

                watch_rtc_set_unix_time(timestamp);  // pass raw UTC
                state->mode = COMMS_MODE_RX_DONE;
            } else {
                state->mode = COMMS_MODE_RX_ERROR;
            }
            break;

        case PACKET_TYPE_CONFIG:
            // Config update (not yet implemented)
            state->mode = COMMS_MODE_RX_DONE;
            break;

        case PACKET_TYPE_ACK:
            // ACK packet (not yet implemented)
            state->mode = COMMS_MODE_RX_DONE;
            break;

        default:
            // Unknown packet type
            state->mode = COMMS_MODE_RX_ERROR;
            break;
    }
}

// Poll light sensor and decode Manchester bits.
// Called on EVENT_TICK at 64 Hz (16 bps => 4 ticks per bit).
//
// State machine:
//
//   SYNCING (!synced):
//     Count edges spaced SYNC_TICKS_PER_EDGE ticks apart.  After
//     SYNC_EDGES_NEEDED consecutive good-spaced edges, lock bit phase and
//     transition to RECEIVING.  Timeout after RX_SYNC_TIMEOUT_TICKS.
//
//   RECEIVING (synced):
//     decode_phase counts 0..3 within each 4-tick bit period.
//       Phase 0 -- start of bit:  sample first half into last_state.
//       Phase 2 -- bit midpoint:  sample second half; decode bit via
//                                 midpoint transition direction (Manchester).
//     Accumulate decoded bits into bit_buffer; on 8 bits store a byte.
//     Check packet-complete condition after each stored byte.
//     Timeout via rx_timeout if no valid midpoint sample for > RX_BIT_TIMEOUT_TICKS.
void optical_rx_poll(comms_face_state_t *state) {
    if (state->mode != COMMS_MODE_RX_ACTIVE) return;

    uint16_t light   = read_light_level();
    bool     current = (light > state->rx_state.light_threshold);

    if (!state->rx_state.synced) {
        // --- SYNCING ---
        // Look for regular edges spaced SYNC_TICKS_PER_EDGE (+/- TOLERANCE) ticks apart.
        bool edge = (current != state->rx_state.last_state);
        state->rx_state.last_state = current;
        state->rx_state.rx_timeout++;

        if (edge) {
            uint16_t interval        = state->rx_state.rx_timeout;
            state->rx_state.rx_timeout = 0;

            bool good_edge =
                (interval >= (SYNC_TICKS_PER_EDGE - SYNC_TICKS_TOLERANCE)) &&
                (interval <= (SYNC_TICKS_PER_EDGE + SYNC_TICKS_TOLERANCE));

            if (good_edge) {
                state->rx_state.bit_count++;
                if (state->rx_state.bit_count >= SYNC_EDGES_NEEDED) {
                    // Phase locked.  The last edge was a midpoint transition
                    // (tick 2 of the final sync bit).  Set decode_phase = 2 so
                    // that the next two ticks advance to 3 then 0, aligning
                    // phase 0 with the start of the first data bit.
                    state->rx_state.synced       = true;
                    state->rx_state.decode_phase = 2;
                    state->rx_state.bit_count    = 0;
                    state->rx_state.bit_buffer   = 0;
                    state->rx_state.rx_timeout   = 0;
                }
            } else {
                // Bad spacing: restart edge count.
                state->rx_state.bit_count = 0;
            }
        } else if (state->rx_state.rx_timeout > RX_SYNC_TIMEOUT_TICKS) {
            state->mode = COMMS_MODE_RX_ERROR;
        }

    } else {
        // --- RECEIVING ---
        // Advance the 0..3 tick counter within the current bit period.
        state->rx_state.decode_phase = (state->rx_state.decode_phase + 1) & 3;
        state->rx_state.rx_timeout++;

        if (state->rx_state.decode_phase == 0) {
            // Start of bit: capture first-half sample into last_state.
            state->rx_state.last_state = current;

        } else if (state->rx_state.decode_phase == 2) {
            // Bit midpoint: determine bit value from transition direction.
            int8_t bit = decode_manchester_bit(state->rx_state.last_state, current);
            if (bit < 0) {
                // No transition at midpoint -- framing error.
                state->mode = COMMS_MODE_RX_ERROR;
                return;
            }

            // Successful bit -- reset timeout.
            state->rx_state.rx_timeout = 0;

            // Accumulate bit MSB-first.
            state->rx_state.bit_buffer = (state->rx_state.bit_buffer << 1) | (uint8_t)bit;
            state->rx_state.bit_count++;

            if (state->rx_state.bit_count == 8) {
                // Complete byte received.
                if (state->rx_state.rx_index >= sizeof(state->rx_state.rx_buffer)) {
                    // Buffer overflow.
                    state->mode = COMMS_MODE_RX_ERROR;
                    return;
                }
                state->rx_state.rx_buffer[state->rx_state.rx_index++] = state->rx_state.bit_buffer;
                state->bytes_received++;
                state->rx_state.bit_count  = 0;
                state->rx_state.bit_buffer = 0;

                // Check if packet is complete: LEN + TYPE + DATA[LEN] + CRC8
                if (state->bytes_received >= 3) {
                    uint8_t expected_len = state->rx_state.rx_buffer[0];
                    if (state->bytes_received == (uint8_t)(expected_len + 3)) {
                        process_packet(state);
                        return;
                    }
                }
            }
        }

        // Bit-level timeout: signal has been flat too long -- missed transition.
        if (state->rx_state.rx_timeout > RX_BIT_TIMEOUT_TICKS) {
            state->mode = COMMS_MODE_RX_ERROR;
        }
    }
}

#endif // HAS_IR_SENSOR
