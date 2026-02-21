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

#ifndef PLAYLIST_H_
#define PLAYLIST_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef PHASE_ENGINE_ENABLED

#include "metrics.h"

/**
 * Weighted Playlist Controller: Zone-based face rotation system
 * 
 * Maps phase score (0-100) to circadian zones, computes weighted relevance
 * for each metric, and manages dynamic face rotation based on biological state.
 */

// Phase zones (based on phase score)
typedef enum {
    ZONE_EMERGENCE = 0,  // 0-25:  Waking, orienting
    ZONE_MOMENTUM = 1,   // 26-50: Building energy
    ZONE_ACTIVE = 2,     // 51-75: Peak output
    ZONE_DESCENT = 3     // 76-100: Winding down
} phase_zone_t;

// Playlist state (manages face rotation)
typedef struct {
    uint8_t zone;                    // Current zone (0-3)
    uint8_t face_count;              // Faces in rotation (0-6)
    uint8_t face_indices[6];         // Sorted by relevance (metric indices)
    uint8_t current_face;            // Index into face_indices
    uint16_t dwell_ticks;            // Time on current face
    uint16_t dwell_limit;            // Auto-advance threshold (ticks)
    // Hysteresis state (prevent zone flicker)
    uint8_t pending_zone;
    uint8_t consecutive_count;
} playlist_state_t;

/**
 * Initialize playlist controller.
 * Call once at startup.
 * 
 * @param state Playlist state (zeroed by caller)
 */
void playlist_init(playlist_state_t *state);

/**
 * Update playlist based on current phase score and metrics.
 * Handles zone transitions with hysteresis and auto-advance logic.
 * 
 * @param state Playlist state (updated in-place)
 * @param phase_score Current phase score (0-100)
 * @param metrics Current metric snapshot
 */
void playlist_update(playlist_state_t *state, uint16_t phase_score, 
                     const metrics_snapshot_t *metrics);

/**
 * Get current face index from playlist.
 * 
 * @param state Playlist state
 * @return Metric index for current face (0-4: SD, EM, WK, Energy, Comfort)
 */
uint8_t playlist_get_current_face(const playlist_state_t *state);

/**
 * Manually advance to next face in rotation.
 * Call on ALARM button press.
 * 
 * @param state Playlist state (updated in-place)
 */
void playlist_advance(playlist_state_t *state);

/**
 * Get current zone.
 * 
 * @param state Playlist state
 * @return Current zone (0-3)
 */
phase_zone_t playlist_get_zone(const playlist_state_t *state);

#endif // PHASE_ENGINE_ENABLED

#endif // PLAYLIST_H_
