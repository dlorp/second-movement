/*
 * MIT License
 *
 * Copyright (c) 2026 Diego Perez
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

#ifndef SENSORS_H_
#define SENSORS_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef PHASE_ENGINE_ENABLED

#define SENSOR_MOTION_BUFFER_SIZE 5
#define SENSOR_INACTIVITY_MIN 15

// Forward-declared in metrics.h
struct sensor_state_t {
    bool     motion_active;
    uint8_t  inactivity_minutes;
    uint16_t motion_magnitude;
    uint16_t motion_buffer[SENSOR_MOTION_BUFFER_SIZE];
    uint8_t  motion_buf_idx;
    uint8_t  motion_buf_count;
    uint16_t motion_variance;
    uint16_t motion_intensity;
    bool     has_accelerometer;
    bool     initialized;
};

void sensors_init(struct sensor_state_t *state, bool has_accel);
void sensors_configure_accel(struct sensor_state_t *state);
void sensors_update(struct sensor_state_t *state);
uint16_t sensors_get_motion_variance(const struct sensor_state_t *state);
uint16_t sensors_get_motion_intensity(const struct sensor_state_t *state);
bool sensors_is_motion_active(const struct sensor_state_t *state);

#endif // PHASE_ENGINE_ENABLED
#endif // SENSORS_H_
