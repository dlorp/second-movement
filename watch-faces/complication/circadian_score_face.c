/*
 * MIT License
 *
 * Copyright (c) 2026 Diego Perez
 */

#include <stdlib.h>
#include <string.h>
#include "circadian_score_face.h"
#include "circadian_score.h"

static circadian_data_t global_circadian_data = {0};
static bool data_loaded = false;

static void _circadian_score_face_update_display(circadian_score_face_state_t *state) {
    char buf[7] = {0};
    
    // Load data from flash if not yet loaded
    if (!data_loaded) {
        circadian_data_load_from_flash(&global_circadian_data);
        data_loaded = true;
    }
    
    // Calculate components
    circadian_score_components_t components;
    circadian_score_calculate_components(&global_circadian_data, &components);
    
    // Display based on current mode
    switch (state->mode) {
        case CSFACE_MODE_CS:
            sprintf(buf, "CS  %2d", components.overall_score);
            break;
        case CSFACE_MODE_TI:
            sprintf(buf, "TI  %2d", components.timing_score);
            break;
        case CSFACE_MODE_DU:
            sprintf(buf, "DU  %2d", components.duration_score);
            break;
        case CSFACE_MODE_EF:
            sprintf(buf, "EF  %2d", components.efficiency_score);
            break;
        case CSFACE_MODE_AH:
            sprintf(buf, "AH  %2d", components.compliance_score);
            break;
        case CSFACE_MODE_LI:
            sprintf(buf, "LI  %2d", components.light_score);
            break;
        default:
            sprintf(buf, "CS  --");
            break;
    }
    
    watch_display_string(buf, 0);
}

void circadian_score_face_setup(uint8_t watch_face_index, void **context_ptr) {
    (void) watch_face_index;
    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(circadian_score_face_state_t));
        memset(*context_ptr, 0, sizeof(circadian_score_face_state_t));
    }
}

void circadian_score_face_activate(void *context) {
    circadian_score_face_state_t *state = (circadian_score_face_state_t *)context;
    state->mode = CSFACE_MODE_CS;  // Always start with overall score
}

bool circadian_score_face_loop(movement_event_t event, void *context) {
    circadian_score_face_state_t *state = (circadian_score_face_state_t *)context;
    
    switch (event.event_type) {
        case EVENT_ACTIVATE:
        case EVENT_TICK:
            _circadian_score_face_update_display(state);
            break;
        case EVENT_ALARM_BUTTON_UP:
            // Cycle through modes
            state->mode = (state->mode + 1) % CSFACE_MODE_COUNT;
            _circadian_score_face_update_display(state);
            break;
        case EVENT_TIMEOUT:
            movement_move_to_face(0);  // Return to watch face
            break;
        default:
            return movement_default_loop_handler(event);
    }
    
    return true;
}

void circadian_score_face_resign(void *context) {
    (void) context;
    // Nothing to clean up
}
