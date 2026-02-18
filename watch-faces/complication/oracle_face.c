/*
 * MIT License
 * Copyright (c) 2026 Diego Perez
 *
 * Oracle Face - Daily 2-word phrase from natural cycles + biorhythm
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "oracle_face.h"
#include "watch_utility.h"
#include "sunriset.h"
#include "circadian_score.h"

// ─────────────────────────────────────────────────────────────────
// Word Lists
// Word A: Quality/state — governed by circadian score tier
// Word B: Force/movement — governed by moon phase
// All words: 4-5 chars, stored in flash (const)
// ─────────────────────────────────────────────────────────────────

// Word A: Tier 0 (0-20 score) — depleted, heavy
static const char *words_a_t0[] = {"HEAVY", "DENSE", "DULL ", "WORN ", "MUTE ", "GRAY ", "DARK ", "STILL"};

// Word A: Tier 1 (21-40 score) — low energy
static const char *words_a_t1[] = {"SLOW ", "PALE ", "COOL ", "HAZE ", "SOFT ", "DAMP ", "BLUR ", "THIN "};

// Word A: Tier 2 (41-60 score) — average
static const char *words_a_t2[] = {"CALM ", "EVEN ", "MILD ", "FAIR ", "LEAN ", "OPEN ", "QUIET", "CLEAR"};

// Word A: Tier 3 (61-80 score) — good
static const char *words_a_t3[] = {"CRISP", "SHARP", "KEEN ", "BOLD ", "WARM ", "CLEAN", "WIDE ", "TAUT "};

// Word A: Tier 4 (81-100 score) — excellent
static const char *words_a_t4[] = {"GOLD ", "SWIFT", "LIGHT", "ALIVE", "FREE ", "CLEAR", "WIDE ", "KEEN "};

static const char **words_a_tiers[] = {words_a_t0, words_a_t1, words_a_t2, words_a_t3, words_a_t4};
static const uint8_t words_a_counts[] = {8, 8, 8, 8, 8};

// Word B: Governed by moon phase (0-7)
// Phase 0: New moon — potential, seed, void
static const char *words_b_m0[] = {"SEED ", "VOID ", "WELL ", "ROOT ", "PULL ", "DEEP ", "STILL", "DRAW "};

// Phase 1: Waxing crescent — path, beginning, seeking
static const char *words_b_m1[] = {"PATH ", "RISE ", "REACH", "CLIMB", "LEAN ", "SEEK ", "TRAIL", "PUSH "};

// Phase 2: First quarter — building, will, drive
static const char *words_b_m2[] = {"DRIVE", "FORGE", "BUILD", "WILL ", "FORM ", "WORK ", "PRESS", "HOLD "};

// Phase 3: Waxing gibbous — swelling, filling, drawing in
static const char *words_b_m3[] = {"SWELL", "FILL ", "DRAW ", "BLOOM", "WEIGH", "LOAD ", "PULL ", "GROW "};

// Phase 4: Full moon — tide, force, peak, flood
static const char *words_b_m4[] = {"TIDE ", "FLOOD", "PEAK ", "BLOOM", "FORCE", "SURGE", "BURST", "FLOW "};

// Phase 5: Waning gibbous — pour, ease, hold
static const char *words_b_m5[] = {"EASE ", "POUR ", "REST ", "FLOW ", "SPILL", "GIVE ", "STILL", "HOLD "};

// Phase 6: Last quarter — turning, shedding, letting go
static const char *words_b_m6[] = {"TURN ", "FALL ", "DRIFT", "BREAK", "SHED ", "PASS ", "LET  ", "LEAN "};

// Phase 7: Waning crescent — thinning, fading, hush
static const char *words_b_m7[] = {"THIN ", "FADE ", "SINK ", "WANE ", "HUSH ", "BARE ", "STILL", "END  "};

static const char **words_b_phases[] = {
    words_b_m0, words_b_m1, words_b_m2, words_b_m3,
    words_b_m4, words_b_m5, words_b_m6, words_b_m7
};
static const uint8_t words_b_count = 8;

// ─────────────────────────────────────────────────────────────────
// Moon Phase Calculation
// Simple J2000-based calculation, accurate to ±1 day
// Returns 0-7 (0=new, 2=first quarter, 4=full, 6=last quarter)
// ─────────────────────────────────────────────────────────────────
static uint8_t _calculate_moon_phase(int year, int month, int day) {
    // Calculate approximate Julian Day Number
    if (month < 3) { year--; month += 12; }
    long jdn = 365L * year + year / 4 - year / 100 + year / 400
               + (306 * (month + 1)) / 10 + day - 428;

    // Known new moon: 2000-01-06 (JDN ~2451549)
    long days_since_new = jdn - 2451549L;

    // Synodic period: 29.53 days — use integer approximation (x1000)
    // days_since mod 29530 / 1000 ≈ days into current cycle
    long cycle_pos = days_since_new % 30;
    if (cycle_pos < 0) cycle_pos += 30;

    // Map 0-29 days to 8 phases
    return (uint8_t)((cycle_pos * 8) / 30);
}

// ─────────────────────────────────────────────────────────────────
// Day Length Calculation (in minutes)
// Uses sunriset library already in codebase
// ─────────────────────────────────────────────────────────────────
static uint16_t _calculate_day_length_minutes(int year, int month, int day, float lat, float lon) {
    double hours = day_length(year, month, day, lon, lat);

    // Clamp to valid range (polar regions can return 0 or 24)
    if (hours < 0.0) hours = 0.0;
    if (hours > 24.0) hours = 24.0;

    return (uint16_t)(hours * 60.0);
}

// ─────────────────────────────────────────────────────────────────
// Day length tier (0-4): short winter day to long summer day
// ─────────────────────────────────────────────────────────────────
static uint8_t _day_length_tier(uint16_t day_length_min) {
    if (day_length_min < 300)  return 0;  // < 5h: polar/deep winter
    if (day_length_min < 480)  return 1;  // 5-8h: short winter day
    if (day_length_min < 720)  return 2;  // 8-12h: moderate day
    if (day_length_min < 900)  return 3;  // 12-15h: long day
    return 4;                             // > 15h: summer, polar
}

// ─────────────────────────────────────────────────────────────────
// Compute oracle phrase
// Seeds word selection from: moon + day_of_year + circadian + day_length
// ─────────────────────────────────────────────────────────────────
static void _compute_oracle(oracle_face_state_t *state) {
    uint8_t score = state->circadian_score;
    uint8_t doy = state->day_of_year;

    // Circadian score tier (0-4) — governs Word A quality
    uint8_t score_tier = score / 21;
    if (score_tier > 4) score_tier = 4;

    // Day length tier (0-4) — seasonal feel modifier
    uint8_t day_tier = _day_length_tier(state->day_length_min);

    // Personal seed offset from birthday (month × day XOR'd in)
    // Makes phrase unique per person even with identical inputs
    uint8_t birth_offset = 0;
    if (state->birthday.bit.is_set) {
        birth_offset = (uint8_t)((state->birthday.bit.month * state->birthday.bit.day) & 0x07);
    }

    // Word A: circadian score tier + day-of-year + birthday drift
    uint8_t a_count = words_a_counts[score_tier];
    state->word_a_idx = (doy + (day_tier * 2) + birth_offset) % a_count;

    // Word B: moon phase pool + day length + birthday shifts index
    uint8_t b_base = (doy / 3 + birth_offset) % words_b_count;
    int8_t b_shift = (int8_t)day_tier - 2;  // -2 (dark/winter) to +2 (bright/summer)
    int8_t b_idx = (int8_t)b_base + b_shift;
    if (b_idx < 0) b_idx = 0;
    if (b_idx >= words_b_count) b_idx = words_b_count - 1;
    state->word_b_idx = (uint8_t)b_idx;
}

// ─────────────────────────────────────────────────────────────────
// Birthday helpers
// ─────────────────────────────────────────────────────────────────
static void _load_birthday(oracle_face_state_t *state) {
    uint32_t bkup3 = watch_get_backup_data(3);
    state->birthday.reg = (uint16_t)(bkup3 & 0xFFFF);
}

static void _save_birthday(oracle_face_state_t *state) {
    uint32_t bkup3 = watch_get_backup_data(3);
    bkup3 = (bkup3 & 0xFFFF0000) | (uint32_t)state->birthday.reg;
    watch_store_backup_data(bkup3, 3);
}

// ─────────────────────────────────────────────────────────────────
// Load all inputs and compute phrase
// ─────────────────────────────────────────────────────────────────
static void _refresh_oracle(oracle_face_state_t *state) {
    watch_date_time_t now = movement_get_local_date_time();
    int year = now.unit.year + WATCH_RTC_REFERENCE_YEAR;
    int month = now.unit.month;
    int day = now.unit.day;

    // Moon phase today
    state->moon_phase = _calculate_moon_phase(year, month, day);

    // Day of year (1-365, truncated to uint8 for drift math)
    state->day_of_year = (uint8_t)watch_utility_days_since_new_year(year, month, day);

    // Location from BKUP[1]
    movement_location_t loc;
    loc.reg = watch_get_backup_data(1);
    float lat = (float)loc.bit.latitude  / 100.0f;
    float lon = (float)loc.bit.longitude / 100.0f;

    state->day_length_min = _calculate_day_length_minutes(year, month, day, lat, lon);

    // Circadian score from flash
    circadian_data_t circ;
    circadian_data_load_from_flash(&circ);
    state->circadian_score = circadian_score_calculate(&circ);

    // Birthday: load from BKUP[3] and check if today is the birthday
    _load_birthday(state);
    state->is_birthday_today = false;
    if (state->birthday.bit.is_set &&
        state->birthday.bit.month == (uint16_t)month &&
        state->birthday.bit.day   == (uint16_t)day) {
        state->is_birthday_today = true;
        // Calculate the moon phase on the user's next/current birthday this year
        state->birth_moon_phase = _calculate_moon_phase(year,
                                                        state->birthday.bit.month,
                                                        state->birthday.bit.day);
    }

    // Compute phrase from all inputs
    _compute_oracle(state);
    state->needs_update = false;
}

// ─────────────────────────────────────────────────────────────────
// Moon phase name (for info view) — 4 chars max
// ─────────────────────────────────────────────────────────────────
static const char *_moon_phase_name(uint8_t phase) {
    switch (phase) {
        case 0: return "NEW ";
        case 1: return "WXC "; // Waxing Crescent
        case 2: return "FQ  "; // First Quarter
        case 3: return "WXG "; // Waxing Gibbous
        case 4: return "FULL";
        case 5: return "WNG "; // Waning Gibbous
        case 6: return "LQ  "; // Last Quarter
        case 7: return "WNC "; // Waning Crescent
        default: return "????";
    }
}

// ─────────────────────────────────────────────────────────────────
// Display update
// ─────────────────────────────────────────────────────────────────
static void _update_display(oracle_face_state_t *state) {
    char buf[16];

    // Settings mode overrides normal view
    if (state->settings != ORACLE_SETTINGS_IDLE) {
        watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "BD", "BDay");
        if (state->settings == ORACLE_SETTINGS_MONTH) {
            snprintf(buf, sizeof(buf), " MO %2d ", state->birthday.bit.month);
        } else {
            snprintf(buf, sizeof(buf), " DY %2d ", state->birthday.bit.day);
        }
        watch_display_text(WATCH_POSITION_BOTTOM, buf);
        return;
    }

    // Birthday: on user's birthday, show special phrase using birth moon phase
    if (state->is_birthday_today) {
        watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "OR", "Oracle");
        switch (state->view) {
            case ORACLE_VIEW_WORD_A: {
                // Show normal word A but with top indicator flashing
                uint8_t tier = state->circadian_score / 21;
                if (tier > 4) tier = 4;
                watch_display_text(WATCH_POSITION_BOTTOM, words_a_tiers[tier][state->word_a_idx]);
                watch_set_indicator(WATCH_INDICATOR_BELL);  // Celebration marker
                break;
            }
            case ORACLE_VIEW_WORD_B:
                // Show word B from BIRTH moon phase (not today's) — the surprise
                watch_display_text(WATCH_POSITION_BOTTOM, words_b_phases[state->birth_moon_phase][state->word_b_idx]);
                break;
            case ORACLE_VIEW_INFO:
                // Show birth moon phase as special info
                snprintf(buf, sizeof(buf), "%s BDY", _moon_phase_name(state->birth_moon_phase));
                watch_display_text(WATCH_POSITION_BOTTOM, buf);
                break;
            default:
                break;
        }
        return;
    }

    // Normal mode
    watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "OR", "Oracle");
    watch_clear_indicator(WATCH_INDICATOR_BELL);

    switch (state->view) {
        case ORACLE_VIEW_WORD_A: {
            uint8_t tier = state->circadian_score / 21;
            if (tier > 4) tier = 4;
            watch_display_text(WATCH_POSITION_BOTTOM, words_a_tiers[tier][state->word_a_idx]);
            break;
        }
        case ORACLE_VIEW_WORD_B:
            watch_display_text(WATCH_POSITION_BOTTOM, words_b_phases[state->moon_phase][state->word_b_idx]);
            break;
        case ORACLE_VIEW_INFO:
            snprintf(buf, sizeof(buf), "%s%3d", _moon_phase_name(state->moon_phase), state->circadian_score);
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

    // Recompute once per day (or on first activate)
    // day_of_year is 0 on fresh init (needs_update=true), triggers refresh
    watch_date_time_t now = movement_get_local_date_time();
    int year = now.unit.year + WATCH_RTC_REFERENCE_YEAR;
    uint16_t today_doy = watch_utility_days_since_new_year(year, now.unit.month, now.unit.day);
    uint8_t today = (uint8_t)today_doy;

    if (state->needs_update || today != state->day_of_year) {
        _refresh_oracle(state);
    }

    state->view = ORACLE_VIEW_WORD_A;
    _update_display(state);
}

bool oracle_face_loop(movement_event_t event, void *context) {
    oracle_face_state_t *state = (oracle_face_state_t *)context;

    switch (event.event_type) {
        case EVENT_ACTIVATE:
        case EVENT_TICK:
            _update_display(state);
            break;

        case EVENT_ALARM_BUTTON_UP:
            if (state->settings == ORACLE_SETTINGS_MONTH) {
                // Increment birth month (1-12, wraps)
                state->birthday.bit.month = (state->birthday.bit.month % 12) + 1;
                _update_display(state);
            } else if (state->settings == ORACLE_SETTINGS_DAY) {
                // Increment birth day (1-31, wraps)
                state->birthday.bit.day = (state->birthday.bit.day % 31) + 1;
                _update_display(state);
            } else {
                // Normal: cycle Word A → Word B → Info → Word A
                state->view = (oracle_view_t)((state->view + 1) % ORACLE_VIEW_COUNT);
                _update_display(state);
            }
            break;

        case EVENT_ALARM_LONG_PRESS:
            if (state->settings != ORACLE_SETTINGS_IDLE) {
                // Exit settings: save birthday, return to normal
                state->birthday.bit.is_set = 1;
                _save_birthday(state);
                state->settings = ORACLE_SETTINGS_IDLE;
                _refresh_oracle(state);  // Recompute with new birthday offset
            } else {
                // Force refresh (update after sleep score finalizes)
                _refresh_oracle(state);
                state->view = ORACLE_VIEW_WORD_A;
            }
            _update_display(state);
            break;

        case EVENT_LIGHT_BUTTON_DOWN:
            if (state->settings == ORACLE_SETTINGS_IDLE) {
                movement_illuminate_led();
            }
            break;

        case EVENT_LIGHT_BUTTON_UP:
            if (state->settings == ORACLE_SETTINGS_MONTH) {
                // Advance to day setting
                state->settings = ORACLE_SETTINGS_DAY;
                _update_display(state);
            }
            break;

        case EVENT_LIGHT_LONG_PRESS:
            if (state->settings == ORACLE_SETTINGS_IDLE) {
                // Enter birthday settings
                _load_birthday(state);
                if (!state->birthday.bit.is_set) {
                    state->birthday.bit.month = 1;
                    state->birthday.bit.day = 1;
                }
                state->settings = ORACLE_SETTINGS_MONTH;
                _update_display(state);
            }
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
