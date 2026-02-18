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
// Word A: Moon phase families — the cosmic tone of the moment
//
// Moon phase seeds the archetype cluster (8 families × 12 words = 96).
// day_of_year % 12 rolls through the family daily — a new word every day,
// thematically related but never locked to just one word per phase.
// ─────────────────────────────────────────────────────────────────
static const char *words_a[8][12] = {
    // New moon: inward, potential, the quiet pull before anything begins
    {"SEED ", "VOID ", "DRAW ", "STILL", "BROOD", "HUSH ", "WAIT ",
     "PULL ", "DEEP ", "EMPTY", "QUIET", "DARK "},
    // Waxing crescent: first stir, something kindling
    {"RISE ", "STIR ", "SPARK", "LEAN ", "SEEK ", "REACH", "TEND ",
     "PUSH ", "BEGIN", "OPEN ", "LIGHT", "MOVE "},
    // First quarter: momentum, deciding, cutting through
    {"BUILD", "FORGE", "PRESS", "SHAPE", "DRIVE", "CARVE", "CLIMB",
     "HOLD ", "MAKE ", "FORM ", "BRACE", "GRIND"},
    // Waxing gibbous: swelling, heavy with what's coming
    {"SWELL", "FILL ", "CREST", "PULL ", "GROW ", "HEAVY", "NEAR ",
     "ACHE ", "LOAD ", "RICH ", "FULL ", "TENSE"},
    // Full moon: peak, flood, nothing hidden
    {"TIDE ", "PEAK ", "FLOOD", "BLOOM", "SURGE", "BURN ", "SHINE",
     "GLOW ", "BOLD ", "FORCE", "BLAZE", "OPEN "},
    // Waning gibbous: after the peak, slowly giving back
    {"EASE ", "POUR ", "SPILL", "FLOW ", "GIVE ", "DRAIN", "REST ",
     "YIELD", "LOOSE", "SLOW ", "SHED ", "SPENT"},
    // Last quarter: the turn, releasing what's done
    {"TURN ", "FALL ", "DRIFT", "SHED ", "PASS ", "BREAK", "YIELD",
     "LEAN ", "CLEAR", "SLIDE", "SHIFT", "LOOSE"},
    // Waning crescent: thinning, the final dark, going quiet
    {"THIN ", "FADE ", "SINK ", "WANE ", "BARE ", "STILL", "SLEEP",
     "HUSH ", "QUIET", "DARK ", "CLOSE", "EMPTY"},
};
#define WORDS_A_PER_PHASE 12

// ─────────────────────────────────────────────────────────────────
// Word B: Circadian score families — your energy, mood, what you can bring
//
// Circadian tier seeds the cluster (5 tiers × 12 words = 60).
// (day_of_year / 3) % 12 drifts daily at a different rate than Word A —
// the two words shift independently, keeping combos fresh.
// Words are mood/action: not just nouns, not just verbs — the texture of the day.
// ─────────────────────────────────────────────────────────────────
static const char *words_b[5][12] = {
    // 0-20: depleted — rest is the work, not the failure
    {"REST ", "HOLD ", "STILL", "WAIT ", "PAUSE", "YIELD", "QUIET", "DWELL",
     "FLOAT", "LOW  ", "IDLE ", "SLEEP"},
    // 21-40: low — soft, tending, gentle maintenance
    {"TEND ", "SLOW ", "SOFT ", "KEEP ", "MEND ", "EASE ", "LIGHT", "WALK ",
     "TREAD", "NURSE", "STAY ", "CALM "},
    // 41-60: average — steady, carrying it
    {"MOVE ", "SEEK ", "WORK ", "STEP ", "HOLD ", "CARRY", "PUSH ", "PRESS",
     "GRIND", "PACE ", "TRACE", "KEEP "},
    // 61-80: good — intentional, capable, building something
    {"DRIVE", "SHAPE", "MAKE ", "CRAFT", "CLIMB", "REACH", "BUILD", "LEAD ",
     "PRESS", "CARVE", "LOCK ", "FORM "},
    // 81-100: sharp — peak, vivid, don't waste it
    {"SURGE", "BLAZE", "FORGE", "SEAR ", "SPARK", "HUNT ", "SHARP", "BURN ",
     "STORM", "BOLD ", "PUSH ", "FLY  "},
};
#define WORDS_B_PER_TIER 12

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
    // Word A: moon phase selects the family (archetype cluster)
    // day_of_year % 12 rolls through all 12 words in the family daily
    state->word_a_idx = state->day_of_year % WORDS_A_PER_PHASE;

    // Word B: circadian tier selects the energy cluster
    // (day_of_year / 3) % 12 drifts at 1/3 speed of Word A —
    // the two words shift at different rates so combos stay varied
    state->word_b_idx = (state->day_of_year / 3) % WORDS_B_PER_TIER;
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
