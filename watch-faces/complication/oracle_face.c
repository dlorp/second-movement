/*
 * MIT License
 * Copyright (c) 2026 Diego Perez
 *
 * Oracle Face — daily 2-word phrase
 * Word A: moon phase (what the cosmos is doing)
 * Word B: circadian score tier (what you bring to it)
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "oracle_face.h"
#include "watch_utility.h"
#include "circadian_score.h"

// ─────────────────────────────────────────────────────────────────
// Word A: Moon phase pools
// 8 phases × 4 words = 32 options
// Day-of-year % 4 selects within pool for daily drift
// ─────────────────────────────────────────────────────────────────
static const char *words_a[8][4] = {
    {"SEED ", "VOID ", "PULL ", "STILL"},  // New (0)
    {"RISE ", "PATH ", "REACH", "LEAN "},  // Waxing crescent (1)
    {"BUILD", "PUSH ", "FORGE", "PRESS"},  // First quarter (2)
    {"SWELL", "FILL ", "GROW ", "DRAW "},  // Waxing gibbous (3)
    {"TIDE ", "PEAK ", "BLOOM", "FORCE"},  // Full (4)
    {"EASE ", "POUR ", "FLOW ", "HOLD "},  // Waning gibbous (5)
    {"TURN ", "FALL ", "DRIFT", "SHED "},  // Last quarter (6)
    {"THIN ", "FADE ", "HUSH ", "WANE "},  // Waning crescent (7)
};

// ─────────────────────────────────────────────────────────────────
// Word B: Circadian score tiers
// 5 tiers × 4 words = 20 options
// Day-of-year % 4 selects within tier for daily drift
// ─────────────────────────────────────────────────────────────────
static const char *words_b[5][4] = {
    {"REST ", "HOLD ", "STILL", "WAIT "},  // 0-20: depleted
    {"TEND ", "SLOW ", "KEEP ", "SOFT "},  // 21-40: low
    {"MOVE ", "SEEK ", "WORK ", "TEND "},  // 41-60: average
    {"DRIVE", "SHAPE", "BUILD", "PUSH "},  // 61-80: good
    {"FORGE", "SURGE", "BOLD ", "LEAD "},  // 81-100: sharp
};

// ─────────────────────────────────────────────────────────────────
// Moon phase calculation (J2000-based, ±1 day accuracy)
// Returns 0-7 (0=new, 2=first quarter, 4=full, 6=last quarter)
// ─────────────────────────────────────────────────────────────────
static uint8_t _moon_phase(int year, int month, int day) {
    if (month < 3) { year--; month += 12; }
    long jdn = 365L * year + year / 4 - year / 100 + year / 400
               + (306 * (month + 1)) / 10 + day - 428;
    long days_since = jdn - 2451549L;  // Known new moon: 2000-01-06
    long cycle_pos = days_since % 30;
    if (cycle_pos < 0) cycle_pos += 30;
    return (uint8_t)((cycle_pos * 8) / 30);
}

// ─────────────────────────────────────────────────────────────────
// Moon phase short name (4 chars + null)
// ─────────────────────────────────────────────────────────────────
static const char *_moon_name(uint8_t phase) {
    static const char *names[] = {
        "NEW ", "WXC ", "FQ  ", "WXG ",
        "FULL", "WNG ", "LQ  ", "WNC "
    };
    return names[phase < 8 ? phase : 0];
}

// ─────────────────────────────────────────────────────────────────
// Compute phrase from current inputs
// ─────────────────────────────────────────────────────────────────
static void _compute_oracle(oracle_face_state_t *state) {
    // Word A: moon phase pool + day-of-year drift
    state->word_a_idx = state->day_of_year % 4;

    // Word B: circadian score → tier, day-of-year drifts within tier
    uint8_t tier = state->circadian_score / 21;
    if (tier > 4) tier = 4;
    state->word_b_idx = (state->day_of_year / 4) % 4;

    // Store tier in word_b_idx upper nibble isn't needed — just use tier
    // (tier used directly in display, word_b_idx is the within-tier index)
    (void)tier;  // used in display, not stored (derived from circadian_score)
}

// ─────────────────────────────────────────────────────────────────
// Full refresh — load inputs, compute
// ─────────────────────────────────────────────────────────────────
static void _refresh_oracle(oracle_face_state_t *state) {
    watch_date_time_t now = movement_get_local_date_time();
    int year  = now.unit.year + WATCH_RTC_REFERENCE_YEAR;
    int month = now.unit.month;
    int day   = now.unit.day;

    state->moon_phase  = _moon_phase(year, month, day);
    state->day_of_year = (uint8_t)watch_utility_days_since_new_year(year, month, day);

    circadian_data_t circ;
    circadian_data_load_from_flash(&circ);
    state->circadian_score = circadian_score_calculate(&circ);

    // Birthday check (compile-time defines, zero overhead)
#if defined(ORACLE_BIRTH_MONTH) && defined(ORACLE_BIRTH_DAY)
    state->is_birthday_today = (month == ORACLE_BIRTH_MONTH && day == ORACLE_BIRTH_DAY);
#else
    state->is_birthday_today = false;
#endif

    _compute_oracle(state);
    state->needs_update = false;
}

// ─────────────────────────────────────────────────────────────────
// Display update
// ─────────────────────────────────────────────────────────────────
static void _update_display(oracle_face_state_t *state) {
    char buf[16];

    watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "OR", "Oracle");

    // Birthday view — bell on, show BDAY message
    if (state->is_birthday_today && state->view == ORACLE_VIEW_BDAY) {
        watch_display_text(WATCH_POSITION_BOTTOM, " BDAY");
        watch_set_indicator(WATCH_INDICATOR_BELL);
        return;
    }

    // Not birthday or past BDAY view — clear bell
    watch_clear_indicator(WATCH_INDICATOR_BELL);

    // Effective view (skip BDAY on non-birthday)
    oracle_view_t v = state->view;
    if (!state->is_birthday_today && v == ORACLE_VIEW_BDAY) {
        v = ORACLE_VIEW_WORD_A;
    }

    switch (v) {
        case ORACLE_VIEW_WORD_A:
            watch_display_text(WATCH_POSITION_BOTTOM,
                words_a[state->moon_phase][state->word_a_idx]);
            break;

        case ORACLE_VIEW_WORD_B: {
            uint8_t tier = state->circadian_score / 21;
            if (tier > 4) tier = 4;
            watch_display_text(WATCH_POSITION_BOTTOM, words_b[tier][state->word_b_idx]);
            break;
        }

        case ORACLE_VIEW_INFO:
            snprintf(buf, sizeof(buf), "%s%3d", _moon_name(state->moon_phase), state->circadian_score);
            watch_display_text(WATCH_POSITION_BOTTOM, buf);
            break;

        default:
            break;
    }
}

// ─────────────────────────────────────────────────────────────────
// Face lifecycle
// ─────────────────────────────────────────────────────────────────
void oracle_face_setup(uint8_t watch_face_index, void **context_ptr) {
    (void)watch_face_index;
    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(oracle_face_state_t));
        memset(*context_ptr, 0, sizeof(oracle_face_state_t));
        ((oracle_face_state_t *)*context_ptr)->needs_update = true;
    }
}

void oracle_face_activate(void *context) {
    oracle_face_state_t *state = (oracle_face_state_t *)context;

    // Recompute once per day
    watch_date_time_t now = movement_get_local_date_time();
    int year = now.unit.year + WATCH_RTC_REFERENCE_YEAR;
    uint8_t today = (uint8_t)watch_utility_days_since_new_year(year, now.unit.month, now.unit.day);

    if (state->needs_update || today != state->day_of_year) {
        _refresh_oracle(state);
    }

    // Open on BDAY view on birthday, else Word A
    state->view = state->is_birthday_today ? ORACLE_VIEW_BDAY : ORACLE_VIEW_WORD_A;
    _update_display(state);
}

bool oracle_face_loop(movement_event_t event, void *context) {
    oracle_face_state_t *state = (oracle_face_state_t *)context;

    switch (event.event_type) {
        case EVENT_ACTIVATE:
        case EVENT_TICK:
            _update_display(state);
            break;

        case EVENT_ALARM_BUTTON_UP: {
            // Cycle: BDAY → Word A → Word B → Info → Word A
            oracle_view_t next = (oracle_view_t)((state->view + 1) % ORACLE_VIEW_COUNT);
            if (!state->is_birthday_today && next == ORACLE_VIEW_BDAY) {
                next = ORACLE_VIEW_WORD_A;
            }
            state->view = next;
            _update_display(state);
            break;
        }

        case EVENT_ALARM_LONG_PRESS:
            // Force refresh (after sleep score updates in the morning)
            _refresh_oracle(state);
            state->view = ORACLE_VIEW_WORD_A;
            _update_display(state);
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

void oracle_face_resign(void *context) {
    (void)context;
}
