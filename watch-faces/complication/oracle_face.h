/*
 * MIT License
 * Copyright (c) 2026 Diego Perez
 *
 * ORACLE FACE
 *
 * A daily 2-word phrase derived from natural cycles and personal biorhythm.
 * Meant to be checked at the start of active hours as a "sense" for the day.
 *
 * Inputs:
 *   - Moon phase (0-7): Governs the second word (the force, what's moving)
 *   - Day length (hours): From lat/lon via sunriset library
 *   - Day of year (1-365): Seasonal drift/variance within each state
 *   - Circadian score (0-100): Governs the first word (the quality, the tone)
 *
 * Display:
 *   ALARM button cycles: WORD A → WORD B → combined seed info
 *   LIGHT long press: show moon phase name + day length
 *
 * Example phrases:
 *   QUIET / TIDE   (calm day, waxing moon, moderate circadian)
 *   GOLD  / SURGE  (great sleep, full moon, long summer day)
 *   HEAVY / SEED   (poor sleep, new moon, short winter day)
 *   CRISP / DRIFT  (good sleep, last quarter, autumn)
 */

#pragma once

#include "movement.h"

typedef enum {
    ORACLE_VIEW_WORD_A = 0,   // First word (quality)
    ORACLE_VIEW_WORD_B,       // Second word (force)
    ORACLE_VIEW_INFO,         // Seed info (moon + score)
    ORACLE_VIEW_COUNT
} oracle_view_t;

typedef struct {
    oracle_view_t view;       // Current display view
    uint8_t moon_phase;       // 0-7 (0=new, 4=full)
    uint16_t day_length_min;  // Day length in minutes
    uint8_t day_of_year;      // 1-255 (truncated, enough for drift)
    uint8_t circadian_score;  // 0-100 from circadian_score library
    uint8_t word_a_idx;       // Selected word A index
    uint8_t word_b_idx;       // Selected word B index
    bool needs_update;        // Recompute on next activate
} oracle_face_state_t;

void oracle_face_setup(uint8_t watch_face_index, void **context_ptr);
void oracle_face_activate(void *context);
bool oracle_face_loop(movement_event_t event, void *context);
void oracle_face_resign(void *context);

#define oracle_face ((const watch_face_t){ \
    oracle_face_setup, \
    oracle_face_activate, \
    oracle_face_loop, \
    oracle_face_resign, \
    NULL \
})
