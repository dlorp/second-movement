/*
 * MIT License
 * Copyright (c) 2026 Diego Perez
 *
 * ORACLE FACE
 *
 * A daily 2-word phrase derived from natural cycles and personal biorhythm.
 * Meant to be checked at the start of active hours as a "sense" for the day.
 *
 * Natural inputs:
 *   - Moon phase (0-7): Governs Word B pool (the force, what's moving)
 *   - Day length (hours): From lat/lon via sunriset — seasonal bias
 *   - Day of year (1-365): Drift/variance within each state
 *   - Circadian score (0-100): Governs Word A tier (the quality, the tone)
 *
 * Birth chart inputs (personal bias — set once, subtle forever):
 *   - Sun sign element (Fire/Earth/Air/Water): derived from birth month+day
 *     Modifies Word A selection within the circadian tier
 *   - Birth moon phase (0-7): derived from full birth date
 *     Shifts Word B selection within the moon phase pool
 *
 * Birthday:
 *   On user's birthday, face opens with " BDAY " message.
 *   ALARM cycles to the normal oracle phrase for the day.
 *
 * Display (ALARM cycles):
 *   [birthday] BDAY → Word A → Word B → Info
 *   [normal]   Word A → Word B → Info
 *
 * LIGHT long press → birthday setup (birth year + month + day)
 */

#pragma once

#include "movement.h"

typedef enum {
    ORACLE_VIEW_BDAY  = 0,    // Birthday message (only on birthday)
    ORACLE_VIEW_WORD_A,       // First word (quality, element-biased)
    ORACLE_VIEW_WORD_B,       // Second word (force, birth-moon-biased)
    ORACLE_VIEW_INFO,         // Moon name + CS score
    ORACLE_VIEW_COUNT
} oracle_view_t;

typedef enum {
    ORACLE_SETTINGS_IDLE = 0,   // Normal mode
    ORACLE_SETTINGS_YEAR,       // Setting birth year
    ORACLE_SETTINGS_MONTH,      // Setting birth month
    ORACLE_SETTINGS_DAY,        // Setting birth day
} oracle_settings_mode_t;

typedef enum {
    ELEMENT_FIRE  = 0,  // Aries, Leo, Sagittarius  — bold, vivid, forward
    ELEMENT_EARTH = 1,  // Taurus, Virgo, Capricorn  — grounded, still, deep
    ELEMENT_AIR   = 2,  // Gemini, Libra, Aquarius   — clear, swift, light
    ELEMENT_WATER = 3,  // Cancer, Scorpio, Pisces   — quiet, calm, deep
} zodiac_element_t;

// Birth chart data packed into BKUP[3] lower 16 bits
// bits [0:3]  = birth month (1-12)                    4 bits
// bits [4:8]  = birth day (1-31)                      5 bits
// bits [9:15] = birth year offset from 1960 (0-127)   7 bits (covers 1960-2087)
typedef union {
    struct {
        uint16_t month    : 4;  // 1-12
        uint16_t day      : 5;  // 1-31
        uint16_t year_off : 7;  // year - 1960
    } bit;
    uint16_t reg;
} oracle_birthdate_t;

typedef struct {
    oracle_view_t view;               // Current display view
    oracle_settings_mode_t settings;  // Settings mode
    uint8_t moon_phase;               // Today's moon phase 0-7
    uint8_t birth_moon_phase;         // Moon phase on birth date
    zodiac_element_t sun_element;     // Fire/Earth/Air/Water from sun sign
    uint16_t day_length_min;          // Day length in minutes (lat/lon via sunriset)
    uint8_t day_of_year;              // 1-255 (truncated for drift math)
    uint8_t circadian_score;          // 0-100 from circadian_score library
    uint8_t word_a_idx;               // Selected word A index
    uint8_t word_b_idx;               // Selected word B index
    oracle_birthdate_t birthdate;     // Birth date packed for BKUP[3]
    bool birthdate_set;               // True if birthdate has been configured
    bool is_birthday_today;           // True on user's birthday
    bool needs_update;                // Recompute on next activate
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
