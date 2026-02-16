/*
 * MIT License
 *
 * Copyright (c) 2026 Diego Perez
 *
 * Unified Communications Face - Phase 1 Implementation
 * Uses FESK library by Eirik S. Morland (PR #139 to second-movement)
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "comms_face.h"
#include "circadian_score.h"

// Hex encoding lookup
static const char hex_chars[] = "0123456789ABCDEF";

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
    (void)user_data;
    watch_set_indicator(WATCH_INDICATOR_BELL);
}

static void _on_error(fesk_result_t error, void *user_data) {
    (void)error;
    comms_face_state_t *state = (comms_face_state_t *)user_data;
    state->mode = COMMS_MODE_IDLE;
    state->transmission_active = false;
    watch_clear_indicator(WATCH_INDICATOR_BELL);
}

static void _start_transmission(comms_face_state_t *state) {
    // Load circadian data for export
    circadian_data_t circadian_data;
    circadian_data_load_from_flash(&circadian_data);
    
    state->export_size = circadian_data_export_binary(&circadian_data, 
                                                       state->export_buffer, 
                                                       sizeof(state->export_buffer));
    
    if (state->export_size == 0) {
        // No data or buffer too small
        watch_display_text(WATCH_POSITION_BOTTOM, "NO DAT");
        return;
    }
    
    // Hex-encode binary data
    _hex_encode(state->export_buffer, state->export_size, state->hex_buffer);
    
    // Configure FESK session
    fesk_session_config_t config = fesk_session_config_defaults();
    config.static_message = state->hex_buffer;
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
    watch_clear_indicator(WATCH_INDICATOR_BELL);
}

static void _update_display(comms_face_state_t *state) {
    switch (state->mode) {
        case COMMS_MODE_IDLE:
            watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "CO", "Comms");
            watch_display_text(WATCH_POSITION_BOTTOM, " RDY  ");
            break;
        case COMMS_MODE_TX_ACTIVE:
            watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "TX", "Trans");
            watch_display_text(WATCH_POSITION_BOTTOM, "      ");
            break;
        case COMMS_MODE_TX_DONE:
            watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "OK", "Done");
            watch_display_text(WATCH_POSITION_BOTTOM, "      ");
            break;
    }
}

void comms_face_setup(uint8_t watch_face_index, void **context_ptr) {
    (void) watch_face_index;
    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(comms_face_state_t));
        memset(*context_ptr, 0, sizeof(comms_face_state_t));
    }
}

void comms_face_activate(void *context) {
    comms_face_state_t *state = (comms_face_state_t *)context;
    state->mode = COMMS_MODE_IDLE;
    state->transmission_active = false;
    _update_display(state);
}

bool comms_face_loop(movement_event_t event, void *context) {
    comms_face_state_t *state = (comms_face_state_t *)context;
    
    switch (event.event_type) {
        case EVENT_ACTIVATE:
        case EVENT_TICK:
            _update_display(state);
            break;
        case EVENT_ALARM_BUTTON_UP:
            if (state->mode == COMMS_MODE_IDLE) {
                _start_transmission(state);
                _update_display(state);
            } else if (state->mode == COMMS_MODE_TX_ACTIVE) {
                _stop_transmission(state);
                _update_display(state);
            } else if (state->mode == COMMS_MODE_TX_DONE) {
                state->mode = COMMS_MODE_IDLE;
                _update_display(state);
            }
            break;
        case EVENT_LIGHT_BUTTON_DOWN:
            movement_illuminate_led();
            break;
        case EVENT_TIMEOUT:
            if (state->mode != COMMS_MODE_TX_ACTIVE) {
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
}
