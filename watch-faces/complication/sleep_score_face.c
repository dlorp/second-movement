/*
 * MIT License
 *
 * Copyright (c) 2026 Diego Perez
 */

#include <stdlib.h>
#include <string.h>
#include "sleep_score_face.h"
#include "circadian_score.h"

static circadian_data_t global_sleep_data = {0};
static bool data_loaded = false;

static circadian_sleep_night_t* _get_last_night(void) {
    if (!data_loaded) {
        circadian_data_load_from_flash(&global_sleep_data);
        data_loaded = true;
    }
    
    // Most recent night is at (write_index - 1) % 7
    uint8_t last_index = (global_sleep_data.write_index + 6) % 7;
    return &global_sleep_data.nights[last_index];
}

static void _sleep_score_face_update_display(sleep_score_face_state_t *state) {
    char buf[7] = {0};
    circadian_sleep_night_t *last_night = _get_last_night();
    
    if (!last_night->valid) {
        watch_display_text(WATCH_POSITION_FULL, "SL  --");
        return;
    }
    
    switch (state->mode) {
        case SLFACE_MODE_SL: {
            uint8_t score = circadian_score_calculate_sleep_score(last_night);
            sprintf(buf, "SL  %2d", score);
            break;
        }
        case SLFACE_MODE_DU: {
            uint8_t hours = last_night->duration_min / 60;
            uint8_t mins = last_night->duration_min % 60;
            sprintf(buf, "DU%2d%02d", hours, mins);
            break;
        }
        case SLFACE_MODE_EF: {
            sprintf(buf, "EF  %2d", last_night->efficiency);
            break;
        }
        case SLFACE_MODE_WA: {
            sprintf(buf, "WA %3d", last_night->waso_min);
            break;
        }
        case SLFACE_MODE_AW: {
            sprintf(buf, "AW  %2d", last_night->awakenings);
            break;
        }
        default:
            sprintf(buf, "SL  --");
            break;
    }
    
    watch_display_text(WATCH_POSITION_FULL, buf);
}

void sleep_score_face_setup(uint8_t watch_face_index, void **context_ptr) {
    (void) watch_face_index;
    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(sleep_score_face_state_t));
        memset(*context_ptr, 0, sizeof(sleep_score_face_state_t));
    }
}

void sleep_score_face_activate(void *context) {
    sleep_score_face_state_t *state = (sleep_score_face_state_t *)context;
    state->mode = SLFACE_MODE_SL;  // Always start with overall score
}

bool sleep_score_face_loop(movement_event_t event, void *context) {
    sleep_score_face_state_t *state = (sleep_score_face_state_t *)context;
    
    switch (event.event_type) {
        case EVENT_ACTIVATE:
        case EVENT_TICK:
            _sleep_score_face_update_display(state);
            break;
        case EVENT_ALARM_BUTTON_UP:
            // Cycle through modes
            state->mode = (state->mode + 1) % SLFACE_MODE_COUNT;
            _sleep_score_face_update_display(state);
            break;
        case EVENT_TIMEOUT:
            movement_move_to_face(0);  // Return to watch face
            break;
        default:
            return movement_default_loop_handler(event);
    }
    
    return true;
}

void sleep_score_face_resign(void *context) {
    (void) context;
    // Nothing to clean up
}
