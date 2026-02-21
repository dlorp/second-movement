/*
 * MIT License
 *
 * Copyright (c) 2026 Diego Perez
 *
 * Weighted Playlist Controller: Zone-based face rotation
 * 
 * This module:
 * 1. Maps phase score (0-100) to zones (Emergence, Momentum, Active, Descent)
 * 2. Computes weighted relevance for each metric per zone
 * 3. Builds sorted face rotation list
 * 4. Handles zone transitions with hysteresis
 * 5. Manages auto-advance and manual cycling
 */

#include "playlist.h"

#ifdef PHASE_ENGINE_ENABLED

// Zone weight tables (stored in flash)
// Columns: SD, EM, WK, Energy, Comfort (5 metrics, JL deferred to Phase 4)
static const uint8_t zone_weights[4][5] = {
    {30, 25,  5, 10, 30},  // EMERGENCE: SD + Comfort priority
    {20, 20, 30, 10, 20},  // MOMENTUM: WK is key
    {15, 20,  5, 40, 20},  // ACTIVE: Energy dominates
    {10, 35,  0, 10, 45},  // DESCENT: EM + Comfort for wind-down
};

// Default auto-advance interval (30 seconds = 30 ticks in 1Hz tick mode)
#define DEFAULT_DWELL_LIMIT 30

// Hysteresis requirement: 3 consecutive readings in new zone
#define ZONE_HYSTERESIS_COUNT 3

// Minimum relevance threshold to include metric in rotation
#define MIN_RELEVANCE 10

/**
 * Determine zone from phase score.
 * 
 * @param phase_score Phase score (0-100)
 * @return Zone (0-3)
 */
static phase_zone_t determine_zone(uint16_t phase_score) {
    if (phase_score <= 25) return ZONE_EMERGENCE;
    if (phase_score <= 50) return ZONE_MOMENTUM;
    if (phase_score <= 75) return ZONE_ACTIVE;
    return ZONE_DESCENT;
}

/**
 * Compute relevance score for a metric.
 * Metrics near 50 (neutral) have low relevance.
 * Metrics at extremes (0 or 100) surface strongly.
 * 
 * @param weight Zone weight for this metric (0-100)
 * @param metric_value Metric value (0-100)
 * @return Relevance score (0-100)
 */
static uint8_t compute_relevance(uint8_t weight, uint8_t metric_value) {
    // Compute deviation from neutral (50)
    int16_t deviation;
    if (metric_value > 50) {
        deviation = metric_value - 50;
    } else {
        deviation = 50 - metric_value;
    }
    
    // Scale by weight: relevance = weight * deviation / 50
    // Range: 0-100 (when weight=100 and deviation=50)
    return (uint8_t)((weight * deviation) / 50);
}

/**
 * Rebuild face rotation based on current zone and metrics.
 * Computes relevance for each metric, sorts by relevance (descending),
 * and excludes metrics with relevance < threshold.
 * 
 * @param state Playlist state (updated in-place)
 * @param metrics Current metric snapshot
 */
static void rebuild_rotation(playlist_state_t *state, 
                              const metrics_snapshot_t *metrics) {
    uint8_t zone = state->zone;
    
    // Compute relevance for each metric
    uint8_t relevances[5];
    relevances[0] = compute_relevance(zone_weights[zone][0], metrics->sd);
    relevances[1] = compute_relevance(zone_weights[zone][1], metrics->em);
    relevances[2] = compute_relevance(zone_weights[zone][2], metrics->wk);
    relevances[3] = compute_relevance(zone_weights[zone][3], metrics->energy);
    relevances[4] = compute_relevance(zone_weights[zone][4], metrics->comfort);
    
    // Build list of metrics that meet relevance threshold
    state->face_count = 0;
    for (uint8_t i = 0; i < 5; i++) {
        if (relevances[i] >= MIN_RELEVANCE) {
            state->face_indices[state->face_count++] = i;
        }
    }
    
    // Bubble sort by relevance (descending)
    // Guard against underflow when face_count is 0
    if (state->face_count > 1) {
        for (uint8_t i = 0; i < state->face_count - 1; i++) {
            for (uint8_t j = i + 1; j < state->face_count; j++) {
                uint8_t idx_i = state->face_indices[i];
                uint8_t idx_j = state->face_indices[j];
                if (relevances[idx_j] > relevances[idx_i]) {
                    // Swap - metric j is more relevant than metric i
                    state->face_indices[i] = idx_j;
                    state->face_indices[j] = idx_i;
                }
            }
        }
    }
    
    // Reset to first (most relevant) face
    state->current_face = 0;
    state->dwell_ticks = 0;
}

void playlist_init(playlist_state_t *state) {
    state->zone = ZONE_EMERGENCE;  // Default to Emergence zone
    state->face_count = 0;
    state->current_face = 0;
    state->dwell_ticks = 0;
    state->dwell_limit = DEFAULT_DWELL_LIMIT;
    state->pending_zone = ZONE_EMERGENCE;
    state->consecutive_count = 0;
}

void playlist_update(playlist_state_t *state, uint16_t phase_score,
                     const metrics_snapshot_t *metrics) {
    phase_zone_t new_zone = determine_zone(phase_score);
    
    // Hysteresis: require 3 consecutive readings in new zone
    if (new_zone != state->zone) {
        if (new_zone == state->pending_zone) {
            state->consecutive_count++;
            if (state->consecutive_count >= ZONE_HYSTERESIS_COUNT) {
                // Zone change confirmed - rebuild rotation with new weights
                state->zone = new_zone;
                state->consecutive_count = 0;
                rebuild_rotation(state, metrics);
            }
        } else {
            // Different zone detected - start counting
            state->pending_zone = new_zone;
            state->consecutive_count = 1;
        }
        return;  // Don't update dwell during zone transition
    }
    
    // Same zone: reset hysteresis state
    state->pending_zone = state->zone;
    state->consecutive_count = 0;
    
    // Update dwell time and auto-advance if needed
    state->dwell_ticks++;
    if (state->dwell_ticks >= state->dwell_limit) {
        playlist_advance(state);
    }
}

uint8_t playlist_get_current_face(const playlist_state_t *state) {
    if (state->face_count == 0) {
        return 0;  // Default to SD metric if no faces in rotation
    }
    return state->face_indices[state->current_face];
}

void playlist_advance(playlist_state_t *state) {
    if (state->face_count == 0) {
        return;  // No faces to cycle through
    }
    
    // Advance to next face (wrap around)
    state->current_face++;
    if (state->current_face >= state->face_count) {
        state->current_face = 0;
    }
    
    // Reset dwell timer
    state->dwell_ticks = 0;
}

phase_zone_t playlist_get_zone(const playlist_state_t *state) {
    return (phase_zone_t)state->zone;
}

#endif // PHASE_ENGINE_ENABLED
