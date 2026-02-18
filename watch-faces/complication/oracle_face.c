/*
 * MIT License
 * Copyright (c) 2026 Diego Perez
 *
 * Oracle Face - Daily 2-word phrase from natural cycles + birth chart
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
//
// Word A: Quality/state — governed by circadian score tier
//         Biased within each tier by sun sign element
// Word B: Force/movement — governed by moon phase pool
//         Biased within each pool by birth moon phase
//
// All words: exactly 5 chars (padded with spaces), stored in flash
// ─────────────────────────────────────────────────────────────────

// Word A tiers (circadian score), 12 words each (3 per element: Fire/Earth/Air/Water)
// Order within tier: Fire(0,1,2) Earth(3,4,5) Air(6,7,8) Water(9,10,11)
// Element bias picks a preferred starting index: fire→0, earth→3, air→6, water→9

// Tier 0 (0-20): depleted, heavy
static const char *words_a_t0[] = {
    "BOLD ", "BLUNT", "DARK ", // Fire: heavy but present
    "DENSE", "HEAVY", "WORN ", // Earth: weighted down
    "DULL ", "BLUR ", "GRAY ", // Air: foggy, distant
    "STILL", "MUTE ", "DEEP ", // Water: submerged, silent
};

// Tier 1 (21-40): low energy
static const char *words_a_t1[] = {
    "SLOW ", "LEAN ", "DAMP ", // Fire: banked embers
    "THICK", "FLAT ", "PALE ", // Earth: dense ground
    "HAZE ", "THIN ", "COOL ", // Air: overcast
    "SOFT ", "QUIET", "LOW  ", // Water: still surface
};

// Tier 2 (41-60): average
static const char *words_a_t2[] = {
    "FAIR ", "EVEN ", "WARM ", // Fire: steady
    "FIRM ", "CALM ", "OPEN ", // Earth: stable ground
    "MILD ", "CLEAR", "LEAN ", // Air: fresh, unremarkable
    "STILL", "DEEP ", "EVEN ", // Water: calm depth
};

// Tier 3 (61-80): good
static const char *words_a_t3[] = {
    "BOLD ", "SHARP", "KEEN ", // Fire: lit, vivid
    "TAUT ", "CRISP", "CLEAN", // Earth: solid, ready
    "SWIFT", "LIGHT", "WIDE ", // Air: open sky
    "CLEAR", "CALM ", "DEEP ", // Water: lucid pool
};

// Tier 4 (81-100): excellent
static const char *words_a_t4[] = {
    "ALIVE", "GOLD ", "SHARP", // Fire: bright peak
    "RICH ", "WARM ", "FULL ", // Earth: harvest
    "FREE ", "SWIFT", "LIGHT", // Air: open, weightless
    "STILL", "GOLD ", "KEEN ", // Water: perfect clarity
};

static const char **words_a_tiers[] = {words_a_t0, words_a_t1, words_a_t2, words_a_t3, words_a_t4};
static const uint8_t words_a_count = 12;

// Word B: Force/movement — 8 moon phase pools, 8 words each
// Birth moon phase (0-7) adds an offset within the pool for personal flavor

// Phase 0: New moon — potential, seed, void
static const char *words_b_m0[] = {"SEED ", "VOID ", "WELL ", "ROOT ", "PULL ", "DEEP ", "STILL", "DRAW "};
// Phase 1: Waxing crescent — path, seeking
static const char *words_b_m1[] = {"PATH ", "RISE ", "REACH", "CLIMB", "LEAN ", "SEEK ", "TRAIL", "PUSH "};
// Phase 2: First quarter — building, will
static const char *words_b_m2[] = {"DRIVE", "FORGE", "BUILD", "WILL ", "FORM ", "WORK ", "PRESS", "HOLD "};
// Phase 3: Waxing gibbous — swelling, filling
static const char *words_b_m3[] = {"SWELL", "FILL ", "DRAW ", "BLOOM", "WEIGH", "LOAD ", "PULL ", "GROW "};
// Phase 4: Full moon — tide, force, peak
static const char *words_b_m4[] = {"TIDE ", "FLOOD", "PEAK ", "BLOOM", "FORCE", "SURGE", "BURST", "FLOW "};
// Phase 5: Waning gibbous — ease, pour
static const char *words_b_m5[] = {"EASE ", "POUR ", "REST ", "FLOW ", "SPILL", "GIVE ", "STILL", "HOLD "};
// Phase 6: Last quarter — turning, shedding
static const char *words_b_m6[] = {"TURN ", "FALL ", "DRIFT", "BREAK", "SHED ", "PASS ", "LET  ", "LEAN "};
// Phase 7: Waning crescent — thinning, hush
static const char *words_b_m7[] = {"THIN ", "FADE ", "SINK ", "WANE ", "HUSH ", "BARE ", "STILL", "END  "};

static const char **words_b_phases[] = {
    words_b_m0, words_b_m1, words_b_m2, words_b_m3,
    words_b_m4, words_b_m5, words_b_m6, words_b_m7
};
static const uint8_t words_b_count = 8;

// ─────────────────────────────────────────────────────────────────
// Moon Phase Calculation
// J2000-based, accurate to ±1 day
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
// Sun Sign → Element
// Zodiac sign from birth month + day → Fire/Earth/Air/Water
// ─────────────────────────────────────────────────────────────────
static zodiac_element_t _sun_element(uint8_t month, uint8_t day) {
    // Aries(3/21-4/19)=Fire, Taurus(4/20-5/20)=Earth, Gemini(5/21-6/20)=Air,
    // Cancer(6/21-7/22)=Water, Leo(7/23-8/22)=Fire, Virgo(8/23-9/22)=Earth,
    // Libra(9/23-10/22)=Air, Scorpio(10/23-11/21)=Water,
    // Sagittarius(11/22-12/21)=Fire, Capricorn(12/22-1/19)=Earth,
    // Aquarius(1/20-2/18)=Air, Pisces(2/19-3/20)=Water
    switch (month) {
        case 1:  return day <= 19 ? ELEMENT_EARTH : ELEMENT_AIR;    // Cap / Aquarius
        case 2:  return day <= 18 ? ELEMENT_AIR   : ELEMENT_WATER;  // Aquarius / Pisces
        case 3:  return day <= 20 ? ELEMENT_WATER  : ELEMENT_FIRE;  // Pisces / Aries
        case 4:  return day <= 19 ? ELEMENT_FIRE   : ELEMENT_EARTH; // Aries / Taurus
        case 5:  return day <= 20 ? ELEMENT_EARTH  : ELEMENT_AIR;   // Taurus / Gemini
        case 6:  return day <= 20 ? ELEMENT_AIR    : ELEMENT_WATER; // Gemini / Cancer
        case 7:  return day <= 22 ? ELEMENT_WATER  : ELEMENT_FIRE;  // Cancer / Leo
        case 8:  return day <= 22 ? ELEMENT_FIRE   : ELEMENT_EARTH; // Leo / Virgo
        case 9:  return day <= 22 ? ELEMENT_EARTH  : ELEMENT_AIR;   // Virgo / Libra
        case 10: return day <= 22 ? ELEMENT_AIR    : ELEMENT_WATER; // Libra / Scorpio
        case 11: return day <= 21 ? ELEMENT_WATER  : ELEMENT_FIRE;  // Scorpio / Sag
        case 12: return day <= 21 ? ELEMENT_FIRE   : ELEMENT_EARTH; // Sagittarius / Cap
        default: return ELEMENT_AIR;
    }
}

// ─────────────────────────────────────────────────────────────────
// Day Length (minutes) from lat/lon
// ─────────────────────────────────────────────────────────────────
static uint16_t _day_length_minutes(int year, int month, int day, float lat, float lon) {
    double hours = day_length(year, month, day, lon, lat);
    if (hours < 0.0) hours = 0.0;
    if (hours > 24.0) hours = 24.0;
    return (uint16_t)(hours * 60.0);
}

// Day length tier 0-4 (polar winter → polar summer)
static uint8_t _day_tier(uint16_t min) {
    if (min < 300)  return 0;  // < 5h
    if (min < 480)  return 1;  // 5-8h
    if (min < 720)  return 2;  // 8-12h
    if (min < 900)  return 3;  // 12-15h
    return 4;                  // 15h+
}

// ─────────────────────────────────────────────────────────────────
// Compute oracle phrase
// ─────────────────────────────────────────────────────────────────
static void _compute_oracle(oracle_face_state_t *state) {
    uint8_t score = state->circadian_score;
    uint8_t doy   = state->day_of_year;

    // Circadian score tier (0-4)
    uint8_t score_tier = score / 21;
    if (score_tier > 4) score_tier = 4;

    // Day length tier (seasonal feel)
    uint8_t day_tier = _day_tier(state->day_length_min);

    // Word A: circadian tier → base range, element → starting index within tier
    // Element picks cluster: Fire=0, Earth=3, Air=6, Water=9 (3 words each)
    uint8_t elem_base = (uint8_t)state->sun_element * 3;
    // Day-of-year and day-tier drift within the 3-word element cluster
    uint8_t a_cluster_idx = (doy + day_tier) % 3;
    state->word_a_idx = (elem_base + a_cluster_idx) % words_a_count;

    // Word B: today's moon phase pool + birth moon phase shifts index
    // Birth moon phase adds a personal offset (0-7) within the pool
    uint8_t b_base = (doy / 3) % words_b_count;
    // Birth moon phase: add half its value as a shift (0-3 range to avoid wrapping too much)
    uint8_t birth_shift = state->birthdate_set ? (state->birth_moon_phase / 2) : 0;
    // Day length shifts: winter→dark end, summer→bright end
    int8_t day_shift = (int8_t)day_tier - 2;
    int8_t b_idx = (int8_t)b_base + (int8_t)birth_shift + day_shift;
    if (b_idx < 0) b_idx = 0;
    if (b_idx >= words_b_count) b_idx = words_b_count - 1;
    state->word_b_idx = (uint8_t)b_idx;
}

// ─────────────────────────────────────────────────────────────────
// BKUP[3] storage helpers
// Lower 16 bits: oracle_birthdate_t
// Upper 16 bits: reserved for other uses
// ─────────────────────────────────────────────────────────────────
static void _load_birthdate(oracle_face_state_t *state) {
    uint32_t bkup3 = watch_get_backup_data(3);
    state->birthdate.reg = (uint16_t)(bkup3 & 0xFFFF);
    // Validate: a year_off of 0 with month=0 means not set
    state->birthdate_set = (state->birthdate.bit.month >= 1 &&
                            state->birthdate.bit.month <= 12 &&
                            state->birthdate.bit.day   >= 1 &&
                            state->birthdate.bit.day   <= 31);
}

static void _save_birthdate(oracle_face_state_t *state) {
    uint32_t bkup3 = watch_get_backup_data(3);
    bkup3 = (bkup3 & 0xFFFF0000) | (uint32_t)state->birthdate.reg;
    watch_store_backup_data(bkup3, 3);
}

// ─────────────────────────────────────────────────────────────────
// Full refresh — load all inputs, compute phrase
// ─────────────────────────────────────────────────────────────────
static void _refresh_oracle(oracle_face_state_t *state) {
    watch_date_time_t now = movement_get_local_date_time();
    int year  = now.unit.year + WATCH_RTC_REFERENCE_YEAR;
    int month = now.unit.month;
    int day   = now.unit.day;

    // Today's moon phase
    state->moon_phase = _moon_phase(year, month, day);

    // Day of year (truncated to uint8 — enough range for drift)
    state->day_of_year = (uint8_t)watch_utility_days_since_new_year(year, month, day);

    // Day length from lat/lon (BKUP[1])
    movement_location_t loc;
    loc.reg = watch_get_backup_data(1);
    float lat = (float)loc.bit.latitude  / 100.0f;
    float lon = (float)loc.bit.longitude / 100.0f;
    state->day_length_min = _day_length_minutes(year, month, day, lat, lon);

    // Circadian score from 7-day flash data
    circadian_data_t circ;
    circadian_data_load_from_flash(&circ);
    state->circadian_score = circadian_score_calculate(&circ);

    // Birth chart: load from BKUP[3], derive element + birth moon
    _load_birthdate(state);
    if (state->birthdate_set) {
        int birth_year  = 1960 + (int)state->birthdate.bit.year_off;
        int birth_month = (int)state->birthdate.bit.month;
        int birth_day   = (int)state->birthdate.bit.day;

        state->sun_element      = _sun_element(birth_month, birth_day);
        state->birth_moon_phase = _moon_phase(birth_year, birth_month, birth_day);

        // Check if today is the birthday
        state->is_birthday_today = (month == birth_month && day == birth_day);
    } else {
        state->sun_element       = ELEMENT_AIR;  // Neutral default
        state->birth_moon_phase  = 0;
        state->is_birthday_today = false;
    }

    _compute_oracle(state);
    state->needs_update = false;
}

// ─────────────────────────────────────────────────────────────────
// Moon phase short name (4 chars)
// ─────────────────────────────────────────────────────────────────
static const char *_moon_name(uint8_t phase) {
    switch (phase) {
        case 0: return "NEW ";
        case 1: return "WXC ";
        case 2: return "FQ  ";
        case 3: return "WXG ";
        case 4: return "FULL";
        case 5: return "WNG ";
        case 6: return "LQ  ";
        case 7: return "WNC ";
        default: return "?   ";
    }
}

// ─────────────────────────────────────────────────────────────────
// Display update
// ─────────────────────────────────────────────────────────────────
static void _update_display(oracle_face_state_t *state) {
    char buf[16];

    // Settings mode overrides everything
    if (state->settings != ORACLE_SETTINGS_IDLE) {
        watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "BD", "BDay");
        switch (state->settings) {
            case ORACLE_SETTINGS_YEAR:
                snprintf(buf, sizeof(buf), "YR  %2d", 1960 + (int)state->birthdate.bit.year_off);
                break;
            case ORACLE_SETTINGS_MONTH:
                snprintf(buf, sizeof(buf), "MO  %2d", state->birthdate.bit.month);
                break;
            case ORACLE_SETTINGS_DAY:
                snprintf(buf, sizeof(buf), "DAY %2d", state->birthdate.bit.day);
                break;
            default:
                buf[0] = '\0';
                break;
        }
        watch_display_text(WATCH_POSITION_BOTTOM, buf);
        return;
    }

    // Normal oracle face header
    watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "OR", "Oracle");

    // Birthday: show BDAY message as first view
    if (state->is_birthday_today && state->view == ORACLE_VIEW_BDAY) {
        watch_display_text(WATCH_POSITION_BOTTOM, " BDAY");
        watch_set_indicator(WATCH_INDICATOR_BELL);
        return;
    }

    // Clear bell unless it's birthday
    if (!state->is_birthday_today) {
        watch_clear_indicator(WATCH_INDICATOR_BELL);
    }

    // Determine effective view (skip BDAY if not birthday)
    oracle_view_t effective_view = state->view;
    if (!state->is_birthday_today && effective_view == ORACLE_VIEW_BDAY) {
        effective_view = ORACLE_VIEW_WORD_A;
    }

    switch (effective_view) {
        case ORACLE_VIEW_WORD_A:
            watch_display_text(WATCH_POSITION_BOTTOM, words_a_tiers[state->circadian_score / 21 > 4 ? 4 : state->circadian_score / 21][state->word_a_idx]);
            break;
        case ORACLE_VIEW_WORD_B:
            watch_display_text(WATCH_POSITION_BOTTOM, words_b_phases[state->moon_phase][state->word_b_idx]);
            break;
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

    // Start on BDAY view if it's the birthday, else Word A
    state->view = state->is_birthday_today ? ORACLE_VIEW_BDAY : ORACLE_VIEW_WORD_A;
    state->settings = ORACLE_SETTINGS_IDLE;
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
            if (state->settings == ORACLE_SETTINGS_YEAR) {
                // Increment year (1960-2087, wraps at 127)
                state->birthdate.bit.year_off = (state->birthdate.bit.year_off + 1) % 128;
            } else if (state->settings == ORACLE_SETTINGS_MONTH) {
                state->birthdate.bit.month = (state->birthdate.bit.month % 12) + 1;
            } else if (state->settings == ORACLE_SETTINGS_DAY) {
                state->birthdate.bit.day = (state->birthdate.bit.day % 31) + 1;
            } else {
                // Normal: cycle views, skip BDAY if not birthday
                oracle_view_t next = (oracle_view_t)((state->view + 1) % ORACLE_VIEW_COUNT);
                if (!state->is_birthday_today && next == ORACLE_VIEW_BDAY) {
                    next = ORACLE_VIEW_WORD_A;
                }
                state->view = next;
            }
            _update_display(state);
            break;

        case EVENT_ALARM_LONG_PRESS:
            if (state->settings != ORACLE_SETTINGS_IDLE) {
                // Advance settings page: year → month → day → save+exit
                if (state->settings == ORACLE_SETTINGS_YEAR) {
                    state->settings = ORACLE_SETTINGS_MONTH;
                } else if (state->settings == ORACLE_SETTINGS_MONTH) {
                    state->settings = ORACLE_SETTINGS_DAY;
                } else {
                    // Save and exit settings
                    _save_birthdate(state);
                    state->settings = ORACLE_SETTINGS_IDLE;
                    _refresh_oracle(state);
                    state->view = ORACLE_VIEW_WORD_A;
                }
            } else {
                // Force refresh (after sleep score updates)
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

        case EVENT_LIGHT_LONG_PRESS:
            if (state->settings == ORACLE_SETTINGS_IDLE) {
                // Enter birthdate setup
                _load_birthdate(state);
                if (!state->birthdate_set) {
                    // Default starting values
                    state->birthdate.bit.year_off = 30;  // 1990
                    state->birthdate.bit.month    = 1;
                    state->birthdate.bit.day      = 1;
                }
                state->settings = ORACLE_SETTINGS_YEAR;
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
