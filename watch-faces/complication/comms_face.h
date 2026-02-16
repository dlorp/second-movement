/*
 * MIT License
 *
 * Copyright (c) 2026 Diego Perez
 *
 * Unified Communications Face
 * 
 * Implements Phase 1 of the Sensor Watch Pro Unified Comms Architecture:
 * - TX mode: Acoustic data transmission via FESK (Frequency-Encoded Shift Keying)
 * - Uses FESK library by Eirik S. Morland (PR #139 to second-movement)
 * - Exports sleep/circadian data (287 bytes hex-encoded) to companion app via buzzer
 *
 * Protocol:
 * - Binary data hex-encoded to FESK text format (42-char alphabet)
 * - 287 bytes → 574 hex chars
 * - FESK 4-FSK mode: 26 bps (bits per second)
 * - Built-in CRC8, START/END markers
 * - Full export: ~177 seconds (~3 minutes)
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
#include "fesk_tx.h"
#include "fesk_session.h"

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
    fesk_session_t fesk_session;
    
    // TX state
    uint8_t export_buffer[287];  // Full circadian export (binary)
    char hex_buffer[575];        // Hex-encoded + null terminator
    uint16_t export_size;        // Actual bytes exported
    uint16_t tx_elapsed_seconds; // Elapsed time during transmission
    bool transmission_active;
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
