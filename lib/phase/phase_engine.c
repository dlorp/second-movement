/*
 * MIT License
 *
 * Copyright (c) 2026 Diego Perez
 *
 * Phase Engine implementation (stub version for Phase 1)
 * 
 * This file contains stub implementations that will be replaced
 * in Phase 2 (metrics computation). Current functions return
 * safe default values to verify zero-cost compilation when disabled.
 */

#include "phase_engine.h"

#ifdef PHASE_ENGINE_ENABLED

#include "homebase.h"
#include <string.h>

void phase_engine_init(phase_state_t *state) {
    // Clear all state
    memset(state, 0, sizeof(phase_state_t));
    state->initialized = true;
}

uint16_t phase_compute(phase_state_t *state,
                       uint8_t hour,
                       uint8_t day_of_year,
                       uint16_t activity_level,
                       int16_t temp_c10,
                       uint16_t light_lux) {
    // Stub implementation - returns neutral score
    // Phase 2 will implement actual computation using:
    // 1. homebase_get_entry(day_of_year) for seasonal baseline
    // 2. Circadian curve (cosine approximation via LUT)
    // 3. Activity/temp/light deviation scoring
    
    (void)activity_level;  // Suppress unused warnings
    (void)temp_c10;
    (void)light_lux;
    
    if (!state->initialized) {
        phase_engine_init(state);
    }
    
    state->last_hour = hour;
    state->last_day_of_year = day_of_year;
    state->last_phase_score = 50;  // Neutral score
    
    // Update circular buffer
    state->phase_history[state->history_index] = 50;
    state->history_index = (state->history_index + 1) % 24;
    
    // Update cumulative sum with overflow protection
    // Prevent overflow: if adding new score would exceed UINT16_MAX,
    // subtract oldest score first to make room
    uint8_t oldest_score = state->phase_history[state->history_index];
    if (state->cumulative_phase > UINT16_MAX - state->last_phase_score) {
        // Would overflow - subtract old value first
        if (state->cumulative_phase >= oldest_score) {
            state->cumulative_phase -= oldest_score;
        } else {
            state->cumulative_phase = 0;  // Safety reset on underflow
        }
    }
    state->cumulative_phase += state->last_phase_score;
    
    return 50;
}

int16_t phase_get_trend(const phase_state_t *state, uint8_t hours) {
    // Stub implementation - returns no trend
    (void)state;
    (void)hours;
    return 0;
}

uint8_t phase_get_recommendation(uint16_t phase_score, uint8_t hour) {
    // Stub implementation - returns moderate activity
    (void)phase_score;
    (void)hour;
    return 1;  // Moderate activity
}

#endif // PHASE_ENGINE_ENABLED
