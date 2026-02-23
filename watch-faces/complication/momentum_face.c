/*
 * MIT License
 *
 * Copyright (c) 2026 Diego Perez
 */

#include <stdlib.h>
#include <string.h>
#include "momentum_face.h"

#ifdef PHASE_ENGINE_ENABLED
#include "../../lib/metrics/metrics.h"
#include "../../lib/phase/zone_words.h"

// Forward declaration - metrics_get will be called from the metrics module
extern void metrics_get(const metrics_engine_t *engine, metrics_snapshot_t *out);

static void _momentum_face_update_display(momentum_face_state_t *state) {
    char buf[11] = {0};
    
    // Get current metrics
    metrics_snapshot_t metrics = {0};
    metrics_get(NULL, &metrics);
    
    // Zone indicator in top-left
    watch_display_text(WATCH_POSITION_TOP_LEFT, "MO");
    
    // Display based on mode
    switch (state->display_mode) {
        case 0:  // Word 1
            watch_display_text(WATCH_POSITION_BOTTOM, state->selected_words[0]);
            break;
            
        case 1:  // Word 2
            watch_display_text(WATCH_POSITION_BOTTOM, state->selected_words[1]);
            break;
            
        case 2: {  // Stats: "W Lv2 3d"
            word_level_t level = zone_words_get_level(metrics.wk);
            snprintf(buf, sizeof(buf), "W Lv%d %dd", level, state->streak_days);
            watch_display_text(WATCH_POSITION_BOTTOM, buf);
            break;
        }
            
        default:
            state->display_mode = 0;
            watch_display_text(WATCH_POSITION_BOTTOM, state->selected_words[0]);
            break;
    }
}

void momentum_face_setup(uint8_t watch_face_index, void **context_ptr) {
    (void) watch_face_index;
    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(momentum_face_state_t));
        memset(*context_ptr, 0, sizeof(momentum_face_state_t));
    }
}

void momentum_face_activate(void *context) {
    momentum_face_state_t *state = (momentum_face_state_t *)context;
    state->display_mode = 0;  // Start with word 1
    
    // Get current date for streak tracking
    watch_date_time date_time = watch_rtc_get_date_time();
    uint16_t day_of_year = date_time.unit.day;
    
    // Check if this is a new day
    if (state->last_check_day != day_of_year) {
        uint8_t prev_day = state->last_check_day;
        state->last_check_day = day_of_year;
        
        if (day_of_year == prev_day + 1 || (prev_day == 0)) {
            state->streak_days++;
        } else {
            state->streak_days = 1;
        }
    }
    
    // Get current WK metric for word level
    metrics_snapshot_t metrics = {0};
    metrics_get(NULL, &metrics);
    word_level_t level = zone_words_get_level(metrics.wk);
    
    // Generate random seed from current time
    uint32_t seed = (date_time.reg << 16) | (date_time.unit.hour << 8) | date_time.unit.minute;
    
    // Select words for this activation
    zone_words_select(ZONE_WK, level, seed, 
                     state->selected_words[0], 
                     state->selected_words[1]);
}

bool momentum_face_loop(movement_event_t event, void *context) {
    momentum_face_state_t *state = (momentum_face_state_t *)context;
    
    switch (event.event_type) {
        case EVENT_ACTIVATE:
        case EVENT_TICK:
            _momentum_face_update_display(state);
            break;
            
        case EVENT_ALARM_BUTTON_UP:
            // Cycle through display modes: word1 → word2 → stats → word1
            state->display_mode = (state->display_mode + 1) % 3;
            _momentum_face_update_display(state);
            break;
        
        case EVENT_MODE_LONG_PRESS:
            // Long-press MODE → exit playlist mode, return to clock
            movement_move_to_face(1);  // Clock face (index 1)
            break;
            
        case EVENT_LIGHT_BUTTON_UP:
            movement_illuminate_led();
            break;
            
        case EVENT_TIMEOUT:
            movement_move_to_face(1);  // Return to clock
            break;
            
        default:
            return movement_default_loop_handler(event);
    }
    
    return true;
}

void momentum_face_resign(void *context) {
    (void) context;
    // Nothing to clean up
}

#endif // PHASE_ENGINE_ENABLED
