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
#include "watch.h"

#ifdef HAS_IR_SENSOR
#include "adc.h"

// RX decoder constants
#define RX_SYNC_BYTE 0xAA                // Sync pattern (10101010)
#define RX_MAX_PACKET_SIZE 68            // Max packet: SYNC + LEN + TYPE + DATA(64) + CRC8
#define RX_BIT_TIMEOUT_TICKS 20          // 312ms @ 64 Hz (missed bit timeout, ~5x bit period at 16 bps)
#define RX_PACKET_TIMEOUT_TICKS 7680     // 2 minutes @ 64 Hz

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

// Calibrate threshold (sample for 1 second, find midpoint)
void optical_rx_calibrate(comms_face_state_t *state) {
    uint32_t sum = 0;
    uint16_t samples[RX_CALIBRATION_SAMPLES];
    
    // Collect samples
    for (uint8_t i = 0; i < RX_CALIBRATION_SAMPLES; i++) {
        samples[i] = read_light_level();
        sum += samples[i];
        delay_ms(16);  // ~64 Hz sampling
    }
    
    // Calculate mean
    uint16_t mean = sum / RX_CALIBRATION_SAMPLES;
    
    // Set threshold to mean (midpoint between light/dark)
    state->rx_state.light_threshold = mean;
}

// Decode Manchester bit from two half-bits
// Returns: 0, 1, or -1 (error)
static int8_t decode_manchester_bit(bool first_half, bool second_half) {
    if (!first_half && second_half) {
        return 1;  // LOW → HIGH = 1
    } else if (first_half && !second_half) {
        return 0;  // HIGH → LOW = 0
    } else {
        return -1;  // Invalid transition (error)
    }
}

// Start RX reception
void optical_rx_start(comms_face_state_t *state) {
    // Reset RX state
    memset(&state->rx_state, 0, sizeof(optical_rx_state_t));
    state->bytes_received = 0;
    
    // Enable light sensor
    optical_rx_enable(state);
    
    // Calibrate threshold
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
    
    uint8_t len = state->rx_state.rx_buffer[0];
    uint8_t type = state->rx_state.rx_buffer[1];
    
    // Validate length
    if (len > 64 || len + 3 != state->bytes_received) {
        state->mode = COMMS_MODE_RX_ERROR;
        return;
    }
    
    // Validate CRC8
    uint8_t expected_crc = state->rx_state.rx_buffer[state->bytes_received - 1];
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
                    ((uint32_t)state->rx_state.rx_buffer[2] << 0) |
                    ((uint32_t)state->rx_state.rx_buffer[3] << 8) |
                    ((uint32_t)state->rx_state.rx_buffer[4] << 16) |
                    ((uint32_t)state->rx_state.rx_buffer[5] << 24);
                
                // Extract timezone offset (2 bytes, little-endian, signed)
                int16_t tz_offset = 
                    ((int16_t)state->rx_state.rx_buffer[6] << 0) |
                    ((int16_t)state->rx_state.rx_buffer[7] << 8);
                
                // Apply time sync (Unix timestamp includes timezone offset)
                // Note: timestamp from phone should already be adjusted for timezone
                watch_rtc_set_unix_time(timestamp + (tz_offset * 60));
                
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

// Poll light sensor and decode bits (called on EVENT_TICK @ 64 Hz)
void optical_rx_poll(comms_face_state_t *state) {
    if (state->mode != COMMS_MODE_RX_ACTIVE) return;
    
    // Read light level
    uint16_t light = read_light_level();
    bool current_state = (light > state->rx_state.light_threshold);
    
    // Detect edge
    bool edge = (current_state != state->rx_state.last_state);
    state->rx_state.last_state = current_state;
    
    if (!state->rx_state.synced) {
        // Looking for sync pattern (0xAA = 10101010)
        // This should produce regular alternating transitions
        
        if (edge) {
            // Reset timeout
            state->rx_state.rx_timeout = 0;
            
            // Accumulate bits
            state->rx_state.bit_buffer = (state->rx_state.bit_buffer << 1) | (current_state ? 1 : 0);
            state->rx_state.bit_count++;
            
            // Check for sync byte
            if (state->rx_state.bit_count >= 8 && state->rx_state.bit_buffer == RX_SYNC_BYTE) {
                state->rx_state.synced = true;
                state->rx_state.bit_count = 0;
                state->rx_state.bit_buffer = 0;
            }
        } else {
            // No edge, increment timeout
            state->rx_state.rx_timeout++;
            
            if (state->rx_state.rx_timeout > RX_SYNC_TIMEOUT_TICKS) {
                // Sync timeout
                state->mode = COMMS_MODE_RX_ERROR;
            }
        }
    } else {
        // Synced, receiving data
        
        if (edge) {
            // Reset bit timeout
            state->rx_state.rx_timeout = 0;
            
            // Accumulate bit
            state->rx_state.bit_buffer = (state->rx_state.bit_buffer << 1) | (current_state ? 1 : 0);
            state->rx_state.bit_count++;
            
            // Check if we have a complete byte
            if (state->rx_state.bit_count == 8) {
                // Store byte
                if (state->rx_state.rx_index < sizeof(state->rx_state.rx_buffer)) {
                    state->rx_state.rx_buffer[state->rx_state.rx_index++] = state->rx_state.bit_buffer;
                    state->bytes_received++;
                    
                    // Check if packet is complete
                    if (state->bytes_received >= 3) {
                        uint8_t expected_len = state->rx_state.rx_buffer[0];
                        if (state->bytes_received == expected_len + 3) {
                            // Packet complete
                            process_packet(state);
                            return;
                        }
                    }
                } else {
                    // Buffer overflow
                    state->mode = COMMS_MODE_RX_ERROR;
                    return;
                }
                
                // Reset for next byte
                state->rx_state.bit_count = 0;
                state->rx_state.bit_buffer = 0;
            }
        } else {
            // No edge, check timeouts
            state->rx_state.rx_timeout++;
            
            if (state->rx_state.rx_timeout > RX_BIT_TIMEOUT_TICKS) {
                // Bit timeout (missed transition)
                state->mode = COMMS_MODE_RX_ERROR;
            } else if (state->rx_state.rx_timeout > RX_PACKET_TIMEOUT_TICKS) {
                // Packet timeout
                state->mode = COMMS_MODE_RX_ERROR;
            }
        }
    }
}

#endif // HAS_IR_SENSOR
