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

#include <stdlib.h>
#include <string.h>
#include "sleep_tracker_face.h"
#include "circadian_score.h"

// Cole-Kripke algorithm weights (empirically validated from 1992 paper)
// 11-minute sliding window: [t-5, t-4, t-3, t-2, t-1, t, t+1, t+2, t+3, t+4, t+5]
static const int16_t COLE_KRIPKE_WEIGHTS[COLE_KRIPKE_WINDOW_SIZE] = {
    404,   // t-5
    598,   // t-4
    326,   // t-3
    441,   // t-2
    1408,  // t-1 (highest weight - most predictive)
    598,   // t (current)
    326,   // t+1
    441,   // t+2
    404,   // t+3
    598,   // t+4
    0      // t+5
};

// Default light threshold modifiers (can be tuned during validation)
static const int16_t DEFAULT_LIGHT_MODIFIERS[4] = {
    -200,  // DARK: lower threshold (bias toward sleep)
    -50,   // DIM: slightly lower threshold
    +100,  // MODERATE: raise threshold (bias toward wake)
    +400   // BRIGHT: significantly raise threshold (strong wake bias)
};

//
// Utility Functions
//

static void _sleep_tracker_display_duration(sleep_tracker_state_t *state) {
    char buf[8];
    uint8_t hours = state->total_sleep_minutes / 60;
    uint8_t minutes = state->total_sleep_minutes % 60;
    
    if (hours > 0) {
        snprintf(buf, sizeof(buf), "%dh%02d  ", hours, minutes);
    } else {
        snprintf(buf, sizeof(buf), "%d  ", minutes);
        watch_display_text(WATCH_POSITION_TOP_RIGHT, "min");
    }
    
    watch_display_text(WATCH_POSITION_BOTTOM, buf);
}

static void _sleep_tracker_display_efficiency(sleep_tracker_state_t *state) {
    uint16_t efficiency = sleep_tracker_calculate_efficiency(state);
    char buf[7];
    snprintf(buf, sizeof(buf), "%d%% ", efficiency);
    watch_display_text(WATCH_POSITION_BOTTOM, buf);
}

static void _sleep_tracker_display_waso(sleep_tracker_state_t *state) {
    char buf[7];
    snprintf(buf, sizeof(buf), "%d  ", state->total_wake_minutes);
    watch_display_text(WATCH_POSITION_TOP_RIGHT, "min");
    watch_display_text(WATCH_POSITION_BOTTOM, buf);
}

static void _sleep_tracker_display_awakenings(sleep_tracker_state_t *state) {
    char buf[7];
    snprintf(buf, sizeof(buf), "%d  ", state->num_awakenings);
    watch_display_text(WATCH_POSITION_BOTTOM, buf);
}

static void _sleep_tracker_display_score(sleep_tracker_state_t *state) {
    // Calculate single-night sleep score (only when session is complete)
    if (!state->session_complete) {
        watch_display_text(WATCH_POSITION_FULL, "SL  --");
        return;
    }
    
    // Build night data structure for scoring
    uint16_t total_minutes = state->total_sleep_minutes + state->total_wake_minutes;
    uint8_t efficiency = (total_minutes > 0) ? 
        (state->total_sleep_minutes * 100 / total_minutes) : 0;
    uint8_t light_quality = (total_minutes > 0) ?
        (state->total_dark_minutes * 100 / total_minutes) : 0;
    
    circadian_sleep_night_t night = {
        .onset_timestamp = state->sleep_onset_time,
        .offset_timestamp = state->sleep_offset_time,
        .duration_min = state->total_sleep_minutes,
        .efficiency = efficiency,
        .waso_min = state->total_wake_minutes,
        .awakenings = state->num_awakenings,
        .light_quality = light_quality,
        .valid = true
    };
    
    uint8_t score = circadian_score_calculate_sleep_score(&night);
    
    char buf[8];
    snprintf(buf, sizeof(buf), "SL  %2d", score);
    watch_display_text(WATCH_POSITION_FULL, buf);
}

//
// Core Algorithm Functions
//

light_class_t sleep_tracker_classify_light(uint8_t light_level) {
    if (light_level < LIGHT_THRESHOLD_DARK) {
        return LIGHT_CLASS_DARK;
    } else if (light_level < LIGHT_THRESHOLD_DIM) {
        return LIGHT_CLASS_DIM;
    } else if (light_level < LIGHT_THRESHOLD_MODERATE) {
        return LIGHT_CLASS_MODERATE;
    } else {
        return LIGHT_CLASS_BRIGHT;
    }
}

int32_t sleep_tracker_apply_cole_kripke(sleep_tracker_state_t *state) {
    int32_t score = 0;
    
    for (int i = 0; i < COLE_KRIPKE_WINDOW_SIZE; i++) {
        // Get activity count from sliding window
        int window_idx = (state->window_index + i) % COLE_KRIPKE_WINDOW_SIZE;
        uint16_t activity = state->activity_counts[window_idx];
        
        // Apply weight
        score += (int32_t)COLE_KRIPKE_WEIGHTS[i] * (int32_t)activity;
    }
    
    // Scale to match 1.0 threshold (multiplied by 1000)
    return score;
}

bool sleep_tracker_classify_epoch(sleep_tracker_state_t *state, uint16_t activity_count, uint8_t light_level) {
    // 1. Update sliding window with new activity count
    state->activity_counts[state->window_index] = activity_count;
    state->window_index = (state->window_index + 1) % COLE_KRIPKE_WINDOW_SIZE;
    
    // 2. Calculate Cole-Kripke score
    int32_t score = sleep_tracker_apply_cole_kripke(state);
    
    // 3. Classify light level
    light_class_t light_class = sleep_tracker_classify_light(light_level);
    state->last_light_class = light_class;
    
    // 4. Apply light-based threshold adjustment
    int32_t threshold = COLE_KRIPKE_BASE_THRESHOLD + state->light_modifiers[light_class];
    
    // 5. Classify: score < threshold → SLEEP, score >= threshold → WAKE
    bool is_asleep = (score < threshold);
    
    return is_asleep;
}

void sleep_tracker_set_sleep_bit(sleep_tracker_state_t *state, uint16_t epoch, bool is_asleep) {
    if (epoch >= MAX_SLEEP_EPOCHS) return;
    
    uint8_t byte_index = epoch / 8;
    uint8_t bit_offset = epoch % 8;
    
    if (is_asleep) {
        state->sleep_wake_log[byte_index] |= (1 << bit_offset);
    } else {
        state->sleep_wake_log[byte_index] &= ~(1 << bit_offset);
    }
}

bool sleep_tracker_get_sleep_bit(sleep_tracker_state_t *state, uint16_t epoch) {
    if (epoch >= MAX_SLEEP_EPOCHS) return false;
    
    uint8_t byte_index = epoch / 8;
    uint8_t bit_offset = epoch % 8;
    
    return (state->sleep_wake_log[byte_index] & (1 << bit_offset)) != 0;
}

void sleep_tracker_update_metrics(sleep_tracker_state_t *state, bool is_asleep) {
    // Set current epoch in log
    sleep_tracker_set_sleep_bit(state, state->current_epoch, is_asleep);
    
    // Track sleep onset (first sleep epoch)
    if (is_asleep && state->sleep_onset_time == 0) {
        state->sleep_onset_time = watch_rtc_get_counter();
    }
    
    // Track sleep offset (last sleep epoch)
    if (is_asleep) {
        state->sleep_offset_time = watch_rtc_get_counter();
    }
    
    // Count sleep/wake minutes
    if (is_asleep) {
        state->total_sleep_minutes++;
        
        // Track dark minutes during sleep
        if (state->last_light_class == LIGHT_CLASS_DARK) {
            state->total_dark_minutes++;
        }
    } else {
        state->total_wake_minutes++;
    }
    
    // Count awakenings (transition from sleep to wake bout >= 5 min)
    if (!is_asleep && state->current_epoch >= AWAKENING_THRESHOLD_MIN) {
        // Check if previous AWAKENING_THRESHOLD_MIN epochs were wake
        bool is_awakening = true;
        for (int i = 1; i <= AWAKENING_THRESHOLD_MIN; i++) {
            if (state->current_epoch < i) {
                is_awakening = false;
                break;
            }
            if (sleep_tracker_get_sleep_bit(state, state->current_epoch - i)) {
                is_awakening = false;
                break;
            }
        }
        
        // Check if epoch before wake bout was sleep (transition)
        if (is_awakening && state->current_epoch > AWAKENING_THRESHOLD_MIN) {
            if (sleep_tracker_get_sleep_bit(state, state->current_epoch - AWAKENING_THRESHOLD_MIN - 1)) {
                state->num_awakenings++;
            }
        }
    }
    
    // Advance epoch counter
    state->current_epoch++;
    if (state->current_epoch >= MAX_SLEEP_EPOCHS) {
        // Wrap around (for very long sleep sessions)
        state->current_epoch = 0;
    }
}

uint16_t sleep_tracker_calculate_efficiency(sleep_tracker_state_t *state) {
    if (state->sleep_onset_time == 0 || state->sleep_offset_time == 0) {
        return 0;
    }
    
    uint32_t time_in_bed = (state->sleep_offset_time - state->sleep_onset_time) / 60;  // seconds to minutes
    if (time_in_bed == 0) return 0;
    
    uint16_t efficiency = (state->total_sleep_minutes * 100) / time_in_bed;
    
    // Cap at 100%
    if (efficiency > 100) efficiency = 100;
    
    return efficiency;
}

//
// Session Management
//

void sleep_tracker_start_session(sleep_tracker_state_t *state) {
    sleep_tracker_reset(state);
    state->tracking_active = true;
    state->session_complete = false;
}

void sleep_tracker_end_session(sleep_tracker_state_t *state) {
    state->tracking_active = false;
    state->session_complete = true;
}

void sleep_tracker_reset(sleep_tracker_state_t *state) {
    // Clear sliding window
    memset(state->activity_counts, 0, sizeof(state->activity_counts));
    state->window_index = 0;
    
    // Clear sleep/wake log
    memset(state->sleep_wake_log, 0, sizeof(state->sleep_wake_log));
    state->current_epoch = 0;
    
    // Reset metrics
    state->sleep_onset_time = 0;
    state->sleep_offset_time = 0;
    state->total_sleep_minutes = 0;
    state->total_wake_minutes = 0;
    state->num_awakenings = 0;
    state->total_dark_minutes = 0;
}

//
// Watch Face Functions
//

void sleep_tracker_face_setup(uint8_t watch_face_index, void **context_ptr) {
    (void) watch_face_index;
    
    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(sleep_tracker_state_t));
        sleep_tracker_state_t *state = (sleep_tracker_state_t *)*context_ptr;
        memset(*context_ptr, 0, sizeof(sleep_tracker_state_t));
        
        // Initialize with default light modifiers
        memcpy(state->light_modifiers, DEFAULT_LIGHT_MODIFIERS, sizeof(DEFAULT_LIGHT_MODIFIERS));
        
        // Start with duration display
        state->display_mode = SLEEP_DISPLAY_DURATION;
        state->tracking_active = false;
        state->session_complete = false;
    }
}

void sleep_tracker_face_activate(void *context) {
    (void) context;
    watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "SLP", "SL");
}

bool sleep_tracker_face_loop(movement_event_t event, void *context) {
    sleep_tracker_state_t *state = (sleep_tracker_state_t *)context;
    
    switch (event.event_type) {
        case EVENT_ACTIVATE:
            watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "SLP", "SL");
            
            // Display current metrics
            switch (state->display_mode) {
                case SLEEP_DISPLAY_SCORE:
                    _sleep_tracker_display_score(state);
                    break;
                case SLEEP_DISPLAY_DURATION:
                    _sleep_tracker_display_duration(state);
                    break;
                case SLEEP_DISPLAY_EFFICIENCY:
                    _sleep_tracker_display_efficiency(state);
                    break;
                case SLEEP_DISPLAY_WASO:
                    _sleep_tracker_display_waso(state);
                    break;
                case SLEEP_DISPLAY_AWAKENINGS:
                    _sleep_tracker_display_awakenings(state);
                    break;
            }
            break;
        
        case EVENT_TICK:
            // If tracking is active, this would be called every minute
            // In actual implementation, this would be triggered by movement.c
            // during Active Hours sleep window
            break;
        
        case EVENT_ALARM_BUTTON_UP:
            // Cycle display mode
            // If tracking active, skip SCORE mode (only show when session complete)
            do {
                state->display_mode = (state->display_mode + 1) % 5;
            } while (state->tracking_active && state->display_mode == SLEEP_DISPLAY_SCORE);
            
            switch (state->display_mode) {
                case SLEEP_DISPLAY_SCORE:
                    _sleep_tracker_display_score(state);
                    break;
                case SLEEP_DISPLAY_DURATION:
                    _sleep_tracker_display_duration(state);
                    break;
                case SLEEP_DISPLAY_EFFICIENCY:
                    _sleep_tracker_display_efficiency(state);
                    break;
                case SLEEP_DISPLAY_WASO:
                    _sleep_tracker_display_waso(state);
                    break;
                case SLEEP_DISPLAY_AWAKENINGS:
                    _sleep_tracker_display_awakenings(state);
                    break;
            }
            break;
        
        case EVENT_ALARM_LONG_PRESS:
            // Manual start/stop tracking (for naps, testing, validation)
            if (state->tracking_active) {
                sleep_tracker_end_session(state);
                // Show "END" briefly
                watch_display_text(WATCH_POSITION_FULL, "END   ");
                movement_illuminate_led();
                // Will update to metrics on next tick
            } else {
                sleep_tracker_start_session(state);
                // Show "START" briefly
                watch_display_text(WATCH_POSITION_FULL, "START ");
                movement_illuminate_led();
                // Will update to duration on next tick
            }
            break;
        
        case EVENT_LIGHT_BUTTON_DOWN:
            movement_illuminate_led();
            break;
        
        case EVENT_TIMEOUT:
            movement_move_to_face(0);
            break;
        
        default:
            return movement_default_loop_handler(event);
    }
    
    return true;
}

void sleep_tracker_face_resign(void *context) {
    (void) context;
}
