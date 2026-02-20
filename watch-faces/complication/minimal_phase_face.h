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
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 */

#ifndef MINIMAL_PHASE_FACE_H_
#define MINIMAL_PHASE_FACE_H_

#include "movement.h"

/*
 * MINIMAL PHASE FACE
 * 
 * Displays real-time circadian phase score (0-100) using the phase engine.
 * 
 * Requires homebase configuration (location + timezone) to generate
 * seasonal data table. Without homebase, displays "--".
 * 
 * Display modes (cycle with ALARM button):
 * 1. Current phase score: "PH  75"
 * 2. 6-hour trend: "TR +15" (improving) or "TR -08" (declining)
 * 3. Recommendation: "RC ACT" (rest/moderate/active/peak)
 * 
 * Phase score interpretation:
 *   0-25:  Poor alignment - rest recommended
 *  26-50:  Below average - light activity okay
 *  51-75:  Good alignment - normal activity
 *  76-100: Excellent - peak performance time
 * 
 * See lib/phase/INTEGRATION_GUIDE.md for integration details.
 */

typedef enum {
    PHASE_MODE_SCORE = 0,      // Display current score
    PHASE_MODE_TREND,          // Display 6-hour trend
    PHASE_MODE_RECOMMENDATION, // Display recommendation
    PHASE_MODE_LAST            // Sentinel (not used)
} minimal_phase_mode_t;

typedef struct {
#ifdef PHASE_ENGINE_ENABLED
    phase_state_t phase;       // Phase engine state
#endif
    minimal_phase_mode_t mode; // Current display mode
    uint8_t last_minute;       // Last update minute (to avoid duplicate updates)
} minimal_phase_state_t;

void minimal_phase_face_setup(movement_settings_t *settings, uint8_t watch_face_index, void **context_ptr);
void minimal_phase_face_activate(movement_settings_t *settings, void *context);
bool minimal_phase_face_loop(movement_event_t event, movement_settings_t *settings, void *context);
void minimal_phase_face_resign(movement_settings_t *settings, void *context);

#define minimal_phase_face ((const watch_face_t){ \
    minimal_phase_face_setup, \
    minimal_phase_face_activate, \
    minimal_phase_face_loop, \
    minimal_phase_face_resign, \
    NULL, \
})

#endif // MINIMAL_PHASE_FACE_H_
