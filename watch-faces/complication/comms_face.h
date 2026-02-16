/*
 * MIT License
 *
 * Copyright (c) 2026 Diego Perez
 *
 * Unified Communications Face
 * 
 * Implements Phase 1 of the Sensor Watch Pro Unified Comms Architecture:
 * - TX mode: Acoustic data transmission via FESK (Frequency-Encoded Shift Keying)
 * - Uses chirpy_tx library by Gabor L Ugray for FSK encoding
 * - Exports sleep/circadian data (287 bytes) to companion app via buzzer
 *
 * Protocol:
 * - Packet format: SYNC(0xA5) + HDR + LEN + PAYLOAD(max 60B) + CRC8
 * - Target rate: ~10 bytes/sec
 * - Full export: ~35 seconds
 *
 * Future phases (not yet implemented):
 * - RX mode: Optical data reception via light sensor (BlinkyReceiver protocol)
 * - Status display: Last sync age, pending bytes
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "movement.h"
#include "chirpy_tx.h"

/*
 * Unified Communications Face
 *
 * Phase 1: TX Mode Only
 * - ALARM button: Start TX / Cancel
 * - MODE button: Next face (when not transmitting)
 * - Display: "TX" + progress (segments show % complete)
 *
 * Transmits 287 bytes of circadian data via buzzer chirps (~35 seconds)
 * Companion app (phone mic) receives and decodes FESK packets
 */

typedef enum {
    COMMS_MODE_IDLE,       // Waiting to start
    COMMS_MODE_TX_ACTIVE,  // Transmitting data
    COMMS_MODE_TX_DONE,    // Transmission complete
} comms_mode_t;

// Packet header bitfield
typedef struct {
    uint8_t direction : 2;    // 00 = watch→app
    uint8_t stream_type : 2;  // 00 = sleep, 01 = light, 10 = activity, 11 = control
    uint8_t sequence : 4;     // 0-15, wraps
} comms_packet_hdr_t;

typedef struct {
    comms_mode_t mode;
    chirpy_encoder_state_t encoder;
    chirpy_tick_state_t tick_state;
    
    // TX state
    uint8_t export_buffer[287];  // Full circadian export
    uint16_t export_size;        // Actual bytes to send
    uint16_t bytes_sent;         // Progress tracking
    uint8_t packet_seq;          // Current packet sequence number
    
    // Current packet being transmitted
    uint8_t packet_buffer[64];   // SYNC + HDR + LEN + PAYLOAD(60) + CRC8
    uint8_t packet_size;
    uint8_t packet_pos;          // Position in packet for get_next_byte callback
    
    bool buzzer_active;
} comms_face_state_t;

void comms_face_setup(uint8_t watch_face_index, void **context_ptr);
void comms_face_activate(void *context);
bool comms_face_loop(movement_event_t event, void *context);
void comms_face_resign(void *context);

#define comms_face ((const watch_face_t){ \
    comms_face_setup, \
    comms_face_activate, \
    comms_face_loop, \
    comms_face_resign, \
    NULL \
})
