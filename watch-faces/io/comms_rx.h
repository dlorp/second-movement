#pragma once
/*
 * MIT License
 * Copyright (c) 2026 Diego Perez
 *
 * Optical RX — ambient light sensor receive functions for comms_face.
 */

#include "comms_face.h"

#ifdef HAS_IR_SENSOR

void optical_rx_enable(comms_face_state_t *state);
void optical_rx_disable(comms_face_state_t *state);
void optical_rx_calibrate(comms_face_state_t *state);
void optical_rx_start(comms_face_state_t *state);
void optical_rx_stop(comms_face_state_t *state);
void optical_rx_poll(comms_face_state_t *state);

#endif /* HAS_IR_SENSOR */
