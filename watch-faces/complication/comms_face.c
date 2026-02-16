/*
 * MIT License
 *
 * Copyright (c) 2026 Diego Perez
 *
 * Unified Communications Face - Phase 1 Implementation
 * Uses chirpy_tx library by Gabor L Ugray for acoustic FSK transmission
 */

#include <stdlib.h>
#include <string.h>
#include "comms_face.h"
#include "circadian_score.h"

#define PACKET_SYNC 0xA5
#define MAX_PAYLOAD_SIZE 60

static comms_face_state_t *g_state = NULL;

// Packet assembly helpers
static uint8_t _pack_header(uint8_t direction, uint8_t stream_type, uint8_t sequence) {
    return ((direction & 0x03) << 6) | ((stream_type & 0x03) << 4) | (sequence & 0x0F);
}

static void _build_packet(comms_face_state_t *state, const uint8_t *payload, uint8_t payload_len) {
    uint8_t *pkt = state->packet_buffer;
    uint8_t idx = 0;
    
    // SYNC byte
    pkt[idx++] = PACKET_SYNC;
    
    // HDR (direction=00, stream_type=00 for sleep, sequence)
    uint8_t hdr = _pack_header(0, 0, state->packet_seq);
    pkt[idx++] = hdr;
    
    // LEN
    pkt[idx++] = payload_len;
    
    // PAYLOAD
    memcpy(&pkt[idx], payload, payload_len);
    idx += payload_len;
    
    // CRC8 over HDR + LEN + PAYLOAD
    uint8_t crc = chirpy_crc8(&pkt[1], 2 + payload_len);
    pkt[idx++] = crc;
    
    state->packet_size = idx;
    state->packet_pos = 0;
    state->packet_seq = (state->packet_seq + 1) % 16;
}

// Callback for chirpy_tx to get next byte
static uint8_t _get_next_byte(uint8_t *next_byte) {
    if (!g_state) return 0;
    
    if (g_state->packet_pos < g_state->packet_size) {
        *next_byte = g_state->packet_buffer[g_state->packet_pos++];
        return 1;
    }
    
    return 0;  // End of packet
}

// Buzzer tick callback (called at ~20Hz for chirping)
static void _tx_tick(void *context) {
    (void) context;
    if (!g_state || !g_state->buzzer_active) return;
    
    uint8_t tone = chirpy_get_next_tone(&g_state->encoder);
    
    if (tone == 255) {
        // Packet transmission complete
        g_state->buzzer_active = false;
        watch_set_buzzer_off();
        
        // Check if more data to send
        uint16_t remaining = g_state->export_size - g_state->bytes_sent;
        
        if (remaining > 0) {
            // Build next packet
            uint8_t chunk_size = (remaining > MAX_PAYLOAD_SIZE) ? MAX_PAYLOAD_SIZE : remaining;
            _build_packet(g_state, &g_state->export_buffer[g_state->bytes_sent], chunk_size);
            g_state->bytes_sent += chunk_size;
            
            // Start next packet transmission
            chirpy_init_encoder(&g_state->encoder, _get_next_byte);
            g_state->buzzer_active = true;
        } else {
            // All done
            g_state->mode = COMMS_MODE_TX_DONE;
        }
    } else {
        // Buzz the tone
        uint16_t period = chirpy_get_tone_period(tone);
        watch_set_buzzer_period(period);
        watch_set_buzzer_on();
    }
}

// Main tick callback (64Hz) - delegates to chirpy tick
static void _comms_tick(void *context) {
    if (!g_state) return;
    
    g_state->tick_state.tick_count++;
    if (g_state->tick_state.tick_count >= g_state->tick_state.tick_compare) {
        g_state->tick_state.tick_count = 0;
        if (g_state->tick_state.tick_fun) {
            g_state->tick_state.tick_fun(context);
        }
    }
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
        watch_display_string("NO DAT", 0);
        return;
    }
    
    // Initialize transmission
    state->mode = COMMS_MODE_TX_ACTIVE;
    state->bytes_sent = 0;
    state->packet_seq = 0;
    
    // Build first packet
    uint8_t first_chunk = (state->export_size > MAX_PAYLOAD_SIZE) ? MAX_PAYLOAD_SIZE : state->export_size;
    _build_packet(state, state->export_buffer, first_chunk);
    state->bytes_sent = first_chunk;
    
    // Initialize encoder
    chirpy_init_encoder(&state->encoder, _get_next_byte);
    state->buzzer_active = true;
    
    // Set up tick handler
    state->tick_state.tick_count = 0;
    state->tick_state.tick_compare = 3;  // 64Hz / 3 ≈ 21Hz tone rate
    state->tick_state.tick_fun = _tx_tick;
    
    // Start buzzing
    movement_request_tick_frequency(64);
}

static void _stop_transmission(comms_face_state_t *state) {
    state->mode = COMMS_MODE_IDLE;
    state->buzzer_active = false;
    watch_set_buzzer_off();
    movement_request_tick_frequency(1);
}

static void _update_display(comms_face_state_t *state) {
    char buf[7] = {0};
    
    switch (state->mode) {
        case COMMS_MODE_IDLE:
            sprintf(buf, "TX RDY");
            break;
        case COMMS_MODE_TX_ACTIVE: {
            // Show progress as percentage
            uint8_t pct = (state->bytes_sent * 100) / state->export_size;
            sprintf(buf, "TX %3d", pct);
            break;
        }
        case COMMS_MODE_TX_DONE:
            sprintf(buf, "TX END");
            break;
    }
    
    watch_display_string(buf, 0);
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
    g_state = state;  // Set global for callbacks
    state->mode = COMMS_MODE_IDLE;
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
                _comms_tick(context);
                _update_display(state);
            }
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
    if (state->mode == COMMS_MODE_TX_ACTIVE) {
        _stop_transmission(state);
    }
    g_state = NULL;
}
