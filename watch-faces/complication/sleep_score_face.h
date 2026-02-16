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

#pragma once

#include "movement.h"

/*
 * Sleep Score (SL) Face
 *
 * Displays single-night sleep quality score (0-100):
 * - 50% duration (7-8h optimal)
 * - 30% efficiency (% time asleep in bed)
 * - 20% light exposure (% time in darkness)
 *
 * ALARM button: cycle through last night's metrics
 *   SL (overall) → DU (duration) → EF (efficiency) → WA (WASO) → AW (awakenings)
 *
 * MODE button: next face
 */

typedef enum {
    SLFACE_MODE_SL,    // Overall Sleep Score
    SLFACE_MODE_DU,    // Duration (hours:minutes)
    SLFACE_MODE_EF,    // Efficiency (%)
    SLFACE_MODE_WA,    // WASO (Wake After Sleep Onset, minutes)
    SLFACE_MODE_AW,    // Awakenings (count)
    SLFACE_MODE_COUNT
} sleep_score_face_mode_t;

typedef struct {
    sleep_score_face_mode_t mode;
} sleep_score_face_state_t;

void sleep_score_face_setup(uint8_t watch_face_index, void **context_ptr);
void sleep_score_face_activate(void *context);
bool sleep_score_face_loop(movement_event_t event, void *context);
void sleep_score_face_resign(void *context);

#define sleep_score_face ((const watch_face_t){ \
    sleep_score_face_setup, \
    sleep_score_face_activate, \
    sleep_score_face_loop, \
    sleep_score_face_resign, \
    NULL \
})
