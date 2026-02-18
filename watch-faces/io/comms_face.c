/*
 * MIT License
 *
 * Copyright (c) 2026 Diego Perez
 *
 * Unified Communications Face - Phase 2a Implementation (TX + RX Foundation)
 * TX: Uses FESK library by Eirik S. Morland (PR #139 to second-movement)
 * RX: Optical data reception via ambient light sensor (Manchester encoding)
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "comms_face.h"
#include "circadian_score.h"

#ifdef HAS_IR_SENSOR
#include "adc.h"
#endif

// Hex encoding lookup
static const char hex_chars[] = "0123456789ABCDEF";

/* Static TX buffers: allocated once in BSS, not per-session on heap.
 * 112 bytes binary + 225 bytes hex (224 chars + NUL) = 337 bytes total.
 * Safe on embedded: zeroed at startup, reused each transmission. */
static uint8_t _export_buffer[112];
static char    _hex_buffer[225];

static void _hex_encode(const uint8_t *data, size_t len, char *out) {
    for (size_t i = 0; i < len; i++) {
        out[i * 2] = hex_chars[(data[i] >> 4) & 0xF];
        out[i * 2 + 1] = hex_chars[data[i] & 0xF];
    }
    out[len * 2] = '\0';
}

static void _on_transmission_end(void *user_data) {
    comms_face_state_t *state = (comms_face_state_t *)user_data;
    state->mode = COMMS_MODE_TX_DONE;
    state->transmission_active = false;
    watch_clear_indicator(WATCH_INDICATOR_BELL);
}

static void _on_transmission_start(void *user_data) {
    comms_face_state_t *state = (comms_face_state_t *)user_data;
    state->tx_elapsed_seconds = 0;
    watch_set_indicator(WATCH_INDICATOR_BELL);
}

static void _on_error(fesk_result_t error, void *user_data) {
    (void)error;
    comms_face_state_t *state = (comms_face_state_t *)user_data;
    state->mode = COMMS_MODE_IDLE;
    state->transmission_active = false;
    state->tx_elapsed_seconds = 0;
    watch_clear_indicator(WATCH_INDICATOR_BELL);
}

static void _start_transmission(comms_face_state_t *state) {
    // Load circadian data for export
    circadian_data_t circadian_data;
    circadian_data_load_from_flash(&circadian_data);
    
    state->export_size = circadian_data_export_binary(&circadian_data, 
                                                       _export_buffer, 
                                                       sizeof(_export_buffer));
    
    if (state->export_size == 0) {
        // No data or buffer too small
        watch_display_text(WATCH_POSITION_BOTTOM, "NO DAT");
        return;
    }
    
    // Hex-encode binary data
    _hex_encode(_export_buffer, state->export_size, _hex_buffer);
    
    // Configure FESK session
    fesk_session_config_t config = fesk_session_config_defaults();
    config.static_message = _hex_buffer;
    config.mode = FESK_MODE_4FSK;
    config.enable_countdown = false;  // No countdown, start immediately
    config.show_bell_indicator = false;  // We manage it manually
    config.on_transmission_start = _on_transmission_start;
    config.on_transmission_end = _on_transmission_end;
    config.on_error = _on_error;
    config.user_data = state;
    
    fesk_session_init(&state->fesk_session, &config);
    
    if (fesk_session_start(&state->fesk_session)) {
        state->mode = COMMS_MODE_TX_ACTIVE;
        state->transmission_active = true;
    } else {
        watch_display_text(WATCH_POSITION_BOTTOM, "BUSY  ");
        fesk_session_dispose(&state->fesk_session);
    }
}

static void _stop_transmission(comms_face_state_t *state) {
    fesk_session_cancel(&state->fesk_session);
    fesk_session_dispose(&state->fesk_session);
    state->mode = COMMS_MODE_IDLE;
    state->transmission_active = false;
    state->tx_elapsed_seconds = 0;
    watch_clear_indicator(WATCH_INDICATOR_BELL);
}

// RX functions (implemented in comms_rx.c)
#ifdef HAS_IR_SENSOR
extern void optical_rx_start(comms_face_state_t *state);
extern void optical_rx_stop(comms_face_state_t *state);
extern void optical_rx_poll(comms_face_state_t *state);
#endif

static void _update_display(comms_face_state_t *state) {
    char buf[16];
    
    switch (state->mode) {
        case COMMS_MODE_IDLE:
            if (state->active_mode == COMMS_TX) {
                watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "TX", "Trans");
            } else {
                watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "RX", "Recv");
            }
            watch_display_text(WATCH_POSITION_BOTTOM, " RDY  ");
            break;
        case COMMS_MODE_TX_ACTIVE: {
            watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "TX", "Trans");

            // Calculate remaining time
            // FESK 4-FSK rate: 26 bps (bits per second) = 3.25 bytes/second
            // Hex encoding: 287 bytes -> 574 hex chars
            // Total bits: 574 * 8 = 4592 bits
            // Total time: 4592 / 26 ~= 177 seconds (~3 minutes)
            uint16_t hex_bytes = state->export_size * 2;
            uint16_t total_bits = hex_bytes * 6;
            uint16_t total_seconds = (total_bits + 25) / 26;
            uint16_t elapsed = state->tx_elapsed_seconds;
            int16_t remaining = (int16_t)total_seconds - (int16_t)elapsed;

            if (remaining < 0) remaining = 0;

            if (remaining < 100) {
                snprintf(buf, sizeof(buf), " %2ds  ", remaining);
            } else {
                snprintf(buf, sizeof(buf), "%3ds  ", remaining);
            }
            watch_display_text(WATCH_POSITION_BOTTOM, buf);
            break;
        }
        case COMMS_MODE_TX_DONE:
            watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "TX", "Trans");
            watch_display_text(WATCH_POSITION_BOTTOM, " END  ");
            break;
        case COMMS_MODE_RX_ACTIVE:
            watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "RX", "Recv");
            if (state->rx_state.synced) {
                // Receiving data - show elapsed time
                snprintf(buf, sizeof(buf), " %2ds  ", state->rx_seconds_elapsed);
                watch_display_text(WATCH_POSITION_BOTTOM, buf);
            } else {
                // Looking for sync
                watch_display_text(WATCH_POSITION_BOTTOM, " SYNC ");
            }
            break;
        case COMMS_MODE_RX_DONE:
            watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "RX", "Recv");
            watch_display_text(WATCH_POSITION_BOTTOM, "  OK  ");
            break;
        case COMMS_MODE_RX_ERROR:
            watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "RX", "Recv");
            switch (state->rx_error_code) {
                case RX_ERROR_SYNC_TIMEOUT:    watch_display_text(WATCH_POSITION_BOTTOM, "ER SYN"); break;
                case RX_ERROR_CRC_FAIL:        watch_display_text(WATCH_POSITION_BOTTOM, "ER CRC"); break;
                case RX_ERROR_PACKET_TIMEOUT:  watch_display_text(WATCH_POSITION_BOTTOM, "ER TMO"); break;
                case RX_ERROR_BIT_TIMEOUT:     watch_display_text(WATCH_POSITION_BOTTOM, "ER BIT"); break;
                case RX_ERROR_BUFFER_OVERFLOW: watch_display_text(WATCH_POSITION_BOTTOM, "ER BUF"); break;
                case RX_ERROR_INVALID_LENGTH:  watch_display_text(WATCH_POSITION_BOTTOM, "ER LEN"); break;
                case RX_ERROR_INVALID_TYPE:    watch_display_text(WATCH_POSITION_BOTTOM, "ER TYP"); break;
                default:                       watch_display_text(WATCH_POSITION_BOTTOM, " ERR  "); break;
            }
            break;
    }
}

void comms_face_setup(uint8_t watch_face_index, void **context_ptr) {
    (void) watch_face_index;
    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(comms_face_state_t));
        if (*context_ptr == NULL) return;
        memset(*context_ptr, 0, sizeof(comms_face_state_t));
    }
}

void comms_face_activate(void *context) {
    comms_face_state_t *state = (comms_face_state_t *)context;
    state->mode = COMMS_MODE_IDLE;
    state->transmission_active = false;
    state->tx_elapsed_seconds = 0;
    state->active_mode = COMMS_RX;  // Default to RX mode
    state->light_sensor_active = false;
    _update_display(state);
}

bool comms_face_loop(movement_event_t event, void *context) {
    comms_face_state_t *state = (comms_face_state_t *)context;
    
    switch (event.event_type) {
        case EVENT_ACTIVATE:
            _update_display(state);
            break;
        case EVENT_TICK:
            if (state->mode == COMMS_MODE_TX_ACTIVE) {
                state->tx_elapsed_seconds++;
            }
            // Poll RX if active
            #ifdef HAS_IR_SENSOR
            if (state->mode == COMMS_MODE_RX_ACTIVE) {
                optical_rx_poll(state);

                // Track elapsed time (increment every 64 ticks = 1 second @ 64 Hz)
                if (state->rx_state.synced) {
                    state->rx_tick_counter++;
                    if (state->rx_tick_counter >= 64) {
                        // Prevent overflow (uint16_t supports up to 18 hours)
                        if (state->rx_seconds_elapsed < UINT16_MAX) {
                            state->rx_seconds_elapsed++;
                        }
                        state->rx_tick_counter = 0;
                    }
                }
            }
            #endif
            _update_display(state);
            break;
        case EVENT_ALARM_BUTTON_UP:
            if (state->mode == COMMS_MODE_IDLE) {
                // Start TX or RX depending on mode
                if (state->active_mode == COMMS_TX) {
                    _start_transmission(state);
                } else {
                    #ifdef HAS_IR_SENSOR
                    optical_rx_start(state);
                    #endif
                }
                _update_display(state);
            } else if (state->mode == COMMS_MODE_TX_ACTIVE) {
                _stop_transmission(state);
                _update_display(state);
            } else if (state->mode == COMMS_MODE_RX_ACTIVE) {
                #ifdef HAS_IR_SENSOR
                optical_rx_stop(state);
                #endif
                _update_display(state);
            } else if (state->mode == COMMS_MODE_TX_DONE || 
                       state->mode == COMMS_MODE_RX_DONE || 
                       state->mode == COMMS_MODE_RX_ERROR) {
                state->mode = COMMS_MODE_IDLE;
                state->tx_elapsed_seconds = 0;
                _update_display(state);
            }
            break;
        case EVENT_LIGHT_BUTTON_UP:
            // Light cycles TX ↔ RX when idle (suppress LED in RX to protect light sensor)
            if (state->mode == COMMS_MODE_IDLE) {
                state->active_mode = (state->active_mode == COMMS_TX) ? COMMS_RX : COMMS_TX;
                _update_display(state);
            } else if (state->active_mode != COMMS_RX) {
                movement_illuminate_led();
            }
            break;
        case EVENT_ALARM_LONG_PRESS:
            // Hold alarm swaps TX ↔ RX mode when idle
            if (state->mode == COMMS_MODE_IDLE) {
                state->active_mode = (state->active_mode == COMMS_TX) ? COMMS_RX : COMMS_TX;
                _update_display(state);
            }
            break;
        case EVENT_TIMEOUT:
            if (state->mode != COMMS_MODE_TX_ACTIVE && state->mode != COMMS_MODE_RX_ACTIVE) {
                movement_move_to_face(0);
            }
            break;
        default:
            return movement_default_loop_handler(event);
    }
    
    return true;
}

void comms_face_resign(void *context) {
    comms_face_state_t *state = (comms_face_state_t *)context;
    if (state->transmission_active) {
        _stop_transmission(state);
    }
    #ifdef HAS_IR_SENSOR
    if (state->light_sensor_active) {
        optical_rx_stop(state);
    }
    #endif
}
