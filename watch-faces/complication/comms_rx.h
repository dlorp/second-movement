/*
 * MIT License
 *
 * Copyright (c) 2026 Diego Perez
 *
 * Optical RX Decoder - Manchester Decoding Header
 * Receives time sync and config updates via phone screen flashing
 */

#ifndef COMMS_RX_H
#define COMMS_RX_H

#include "comms_face.h"

#ifdef HAS_IR_SENSOR

void optical_rx_enable(comms_face_state_t *state);
void optical_rx_disable(comms_face_state_t *state);
void optical_rx_calibrate(comms_face_state_t *state);
void optical_rx_start(comms_face_state_t *state);
void optical_rx_stop(comms_face_state_t *state);
void optical_rx_poll(comms_face_state_t *state);

#endif // HAS_IR_SENSOR

#endif // COMMS_RX_H
