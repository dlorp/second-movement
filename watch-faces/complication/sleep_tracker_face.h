/*
 * MIT License
 *
 * Copyright (c) 2026 dlorp
 * Based on research: Cole-Kripke (1992) + Light Enhancement
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

/*
 * SLEEP TRACKER FACE
 *
 * Implements Cole-Kripke (1992) sleep/wake detection algorithm enhanced with
 * light sensor disambiguation. Designed for Sensor Watch Pro hardware.
 *
 * Algorithm:
 *   - Samples LIS2DW12 wake-on-motion interrupts every minute
 *   - Applies weighted sum over 11-minute sliding window
 *   - Adjusts threshold based on ambient light level
 *   - Stores sleep/wake classification in circular buffer
 *
 * Features:
 *   - Sleep/wake detection (85-90% accuracy target)
 *   - Total sleep duration
 *   - Sleep efficiency (% time asleep in bed)
 *   - WASO (Wake After Sleep Onset)
 *   - Number of awakenings
 *   - Light exposure during sleep
 *   - Integration with Active Hours and Circadian Score
 *
 * Display:
 *   - TOP: "SLP" (Sleep)
 *   - BOTTOM: Duration (e.g., "7h30m") or efficiency (e.g., "92%")
 *   - ALARM button toggles display mode
 *
 * Power:
 *   - ~4-5µA during sleep tracking (LIS2DW12 low-power + light ADC)
 *   - Well under 20µA budget
 *
 * Storage:
 *   - 60 bytes for 8 hours of 1-minute epochs (1 bit per minute)
 *   - 22 bytes for sliding window and state
 *   - Total: ~82 bytes RAM
 */

#include "movement.h"

// Cole-Kripke algorithm constants
#define COLE_KRIPKE_WINDOW_SIZE 11
#define COLE_KRIPKE_BASE_THRESHOLD 1000  // 1.0 scaled by 1000

// Light classification thresholds (0-255 scale)
#define LIGHT_THRESHOLD_DARK 10
#define LIGHT_THRESHOLD_DIM 50
#define LIGHT_THRESHOLD_MODERATE 150

// Sleep tracking constants
#define MAX_SLEEP_EPOCHS 480  // 8 hours at 1-minute resolution
#define AWAKENING_THRESHOLD_MIN 5  // Minimum 5 consecutive wake minutes for awakening

// Light classification enum
typedef enum {
    LIGHT_CLASS_DARK = 0,      // 0-10: pitch black, sleeping
    LIGHT_CLASS_DIM = 1,       // 11-50: night light, bathroom
    LIGHT_CLASS_MODERATE = 2,  // 51-150: dim room, phone screen
    LIGHT_CLASS_BRIGHT = 3     // 151-255: full room light, outdoor
} light_class_t;

// Display mode enum
typedef enum {
    SLEEP_DISPLAY_SCORE = 0,      // Show overall sleep score (0-100) when session complete
    SLEEP_DISPLAY_DURATION = 1,   // Show sleep duration
    SLEEP_DISPLAY_EFFICIENCY = 2, // Show sleep efficiency %
    SLEEP_DISPLAY_WASO = 3,       // Show wake after sleep onset
    SLEEP_DISPLAY_AWAKENINGS = 4  // Show number of awakenings
} sleep_display_mode_t;

// Sleep tracker state
typedef struct sleep_tracker_state_s {
    // Cole-Kripke sliding window
    uint16_t activity_counts[COLE_KRIPKE_WINDOW_SIZE];
    uint8_t window_index;
    
    // Sleep/wake log (1 bit per minute, 480 minutes = 60 bytes)
    uint8_t sleep_wake_log[MAX_SLEEP_EPOCHS / 8];
    uint16_t current_epoch;
    
    // Sleep session metadata
    uint32_t sleep_onset_time;   // Unix timestamp of first sleep epoch
    uint32_t sleep_offset_time;  // Unix timestamp of last sleep epoch
    uint16_t total_sleep_minutes;
    uint16_t total_wake_minutes;
    uint8_t num_awakenings;
    
    // Light exposure tracking
    uint32_t total_dark_minutes;  // Minutes in darkness during sleep
    uint8_t last_light_class;
    
    // UI state
    sleep_display_mode_t display_mode;
    bool tracking_active;  // True when sleep tracking is running
    bool session_complete; // True when sleep session ended (for review)
    
    // Tuning parameters (can be adjusted for validation)
    int16_t light_modifiers[4];  // Threshold adjustments per light class
} sleep_tracker_state_t;

// Function declarations
void sleep_tracker_face_setup(uint8_t watch_face_index, void **context_ptr);
void sleep_tracker_face_activate(void *context);
bool sleep_tracker_face_loop(movement_event_t event, void *context);
void sleep_tracker_face_resign(void *context);

// Core algorithm functions
bool sleep_tracker_classify_epoch(sleep_tracker_state_t *state, uint16_t activity_count, uint8_t light_level);
void sleep_tracker_update_metrics(sleep_tracker_state_t *state, bool is_asleep);
light_class_t sleep_tracker_classify_light(uint8_t light_level);
int32_t sleep_tracker_apply_cole_kripke(sleep_tracker_state_t *state);

// Session management
void sleep_tracker_start_session(sleep_tracker_state_t *state);
void sleep_tracker_end_session(sleep_tracker_state_t *state);
void sleep_tracker_reset(sleep_tracker_state_t *state);

// Utility functions
void sleep_tracker_set_sleep_bit(sleep_tracker_state_t *state, uint16_t epoch, bool is_asleep);
bool sleep_tracker_get_sleep_bit(sleep_tracker_state_t *state, uint16_t epoch);
uint16_t sleep_tracker_calculate_efficiency(sleep_tracker_state_t *state);

#define sleep_tracker_face ((const watch_face_t){ \
    sleep_tracker_face_setup, \
    sleep_tracker_face_activate, \
    sleep_tracker_face_loop, \
    sleep_tracker_face_resign, \
    NULL \
})
