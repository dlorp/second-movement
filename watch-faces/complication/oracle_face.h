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
    ORACLE_VIEW_INFO,         // Seed info (moon + CS score)
    ORACLE_VIEW_COUNT
} oracle_view_t;

typedef enum {
    ORACLE_SETTINGS_IDLE = 0,  // Normal mode
    ORACLE_SETTINGS_MONTH,     // Setting birth month
    ORACLE_SETTINGS_DAY,       // Setting birth day
} oracle_settings_mode_t;

// Birthday packed for BKUP[3] lower 16 bits
// bits [0:3]  = birth month (1-12)
// bits [4:8]  = birth day (1-31)
// bit  [9]    = birthday set flag
typedef union {
    struct {
        uint16_t month : 4;   // 1-12
        uint16_t day   : 5;   // 1-31
        uint16_t is_set: 1;   // 0 = not configured
        uint16_t _pad  : 6;
    } bit;
    uint16_t reg;
} oracle_birthday_t;

typedef struct {
    oracle_view_t view;              // Current display view
    oracle_settings_mode_t settings; // Settings mode
    uint8_t moon_phase;              // 0-7 (0=new, 4=full)
    uint8_t birth_moon_phase;        // Moon phase on user's birthday this year
    uint16_t day_length_min;         // Day length in minutes
    uint8_t day_of_year;             // 1-255 (truncated, enough for drift)
    uint8_t circadian_score;         // 0-100 from circadian_score library
    uint8_t word_a_idx;              // Selected word A index
    uint8_t word_b_idx;              // Selected word B index
    oracle_birthday_t birthday;      // Birth month + day (from BKUP[3])
    bool is_birthday_today;          // True on user's birthday
    bool needs_update;               // Recompute on next activate
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
