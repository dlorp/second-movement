/*
 * MIT License
 *
 * Copyright (c) 2026 dlorp
 * Based on alarm_face.c by Josh Berson and Joey Castillo
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>
#include "smart_alarm_face.h"
#include "watch.h"
#include "watch_utility.h"

// Fairy Fountain alarm tune - progressive wake sequence
// 3-phase acceleration: Dreamy → Building → Urgent
// B4-E5-A5-B5 warm lower octave, progressing to B5-E6-A6-B6 high cascade
static const int8_t smart_alarm_tune[] = {
    // === CYCLE 1 ===
    // Phase 1: Dreamy harp (B4-E5-A5-B5)
    BUZZER_NOTE_B4, 10, BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_E5, 10, BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_A5, 10, BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_B5, 10, BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_B4, 10, BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_E5, 10, BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_A5, 10, BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_B5, 10, BUZZER_NOTE_REST, 2,
    // Phase 2: Building tempo
    BUZZER_NOTE_B4, 6, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_E5, 6, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_A5, 6, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_B5, 6, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_B4, 6, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_E5, 6, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_A5, 6, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_B5, 6, BUZZER_NOTE_REST, 1,
    // Phase 3: Urgent octave jump (B5-E6-A6-B6)
    BUZZER_NOTE_B5, 3, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_E6, 3, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_A6, 3, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_B6, 3, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_B5, 3, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_E6, 3, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_A6, 3, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_B6, 3, BUZZER_NOTE_REST, 10,
    
    // === CYCLE 2 ===
    BUZZER_NOTE_B4, 10, BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_E5, 10, BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_A5, 10, BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_B5, 10, BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_B4, 10, BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_E5, 10, BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_A5, 10, BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_B5, 10, BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_B4, 6, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_E5, 6, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_A5, 6, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_B5, 6, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_B4, 6, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_E5, 6, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_A5, 6, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_B5, 6, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_B5, 3, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_E6, 3, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_A6, 3, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_B6, 3, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_B5, 3, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_E6, 3, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_A6, 3, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_B6, 3, BUZZER_NOTE_REST, 10,
    
    // === CYCLE 3 ===
    BUZZER_NOTE_B4, 10, BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_E5, 10, BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_A5, 10, BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_B5, 10, BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_B4, 10, BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_E5, 10, BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_A5, 10, BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_B5, 10, BUZZER_NOTE_REST, 2,
    BUZZER_NOTE_B4, 6, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_E5, 6, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_A5, 6, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_B5, 6, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_B4, 6, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_E5, 6, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_A5, 6, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_B5, 6, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_B5, 3, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_E6, 3, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_A6, 3, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_B6, 3, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_B5, 3, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_E6, 3, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_A6, 3, BUZZER_NOTE_REST, 1,
    BUZZER_NOTE_B6, 3, BUZZER_NOTE_REST, 10,
    0  // End marker
};

// Convert 15-minute increment index to hour and minute
// increment: 0-95 (0=00:00, 4=01:00, 95=23:45)
static void _increment_to_time(uint8_t increment, uint8_t *hour, uint8_t *minute) {
    *hour = (increment * 15) / 60;
    *minute = (increment * 15) % 60;
}

// Convert hour and minute to 15-minute increment index
static uint8_t _time_to_increment(uint8_t hour, uint8_t minute) {
    return (hour * 4) + (minute / 15);
}

// Display the alarm window on screen
// Shows window as "06:45-07:15" or "S 06:45" / "E 07:15" when setting
static void _smart_alarm_display_window(smart_alarm_state_t *state) {
    uint8_t start_hour, start_minute, end_hour, end_minute;
    _increment_to_time(state->window_start, &start_hour, &start_minute);
    _increment_to_time(state->window_end, &end_hour, &end_minute);

    // Handle 12/24 hour display
    uint8_t display_start_hour = start_hour;
    uint8_t display_end_hour = end_hour;
    bool start_pm = false;
    bool end_pm = false;

    if (movement_clock_mode_24h()) {
        watch_set_indicator(WATCH_INDICATOR_24H);
    } else {
        // Convert to 12-hour format
        start_pm = (start_hour >= 12);
        end_pm = (end_hour >= 12);
        display_start_hour = start_hour % 12;
        if (display_start_hour == 0) display_start_hour = 12;
        display_end_hour = end_hour % 12;
        if (display_end_hour == 0) display_end_hour = 12;
    }

    char lcdbuf[11];
    
    if (state->setting_mode == SMART_ALARM_SETTING_WINDOW_START) {
        // Show "S" prefix when setting start time
        sprintf(lcdbuf, "S %2d%02d  ", display_start_hour, start_minute);
        if (!movement_clock_mode_24h() && start_pm) {
            watch_set_indicator(WATCH_INDICATOR_PM);
        } else {
            watch_clear_indicator(WATCH_INDICATOR_PM);
        }
    } else if (state->setting_mode == SMART_ALARM_SETTING_WINDOW_END) {
        // Show "E" prefix when setting end time
        sprintf(lcdbuf, "E %2d%02d  ", display_end_hour, end_minute);
        if (!movement_clock_mode_24h() && end_pm) {
            watch_set_indicator(WATCH_INDICATOR_PM);
        } else {
            watch_clear_indicator(WATCH_INDICATOR_PM);
        }
    } else {
        // Normal display: show window start time
        // (Window end shown in top indicator area)
        sprintf(lcdbuf, "%2d%02d  ", display_start_hour, start_minute);
        if (!movement_clock_mode_24h() && start_pm) {
            watch_set_indicator(WATCH_INDICATOR_PM);
        } else {
            watch_clear_indicator(WATCH_INDICATOR_PM);
        }
        
        // Show end time in top right
        char end_buf[7];
        sprintf(end_buf, "%02d%02d", display_end_hour, end_minute);
        watch_display_text(WATCH_POSITION_TOP_RIGHT, end_buf);
    }

    watch_display_text(WATCH_POSITION_BOTTOM, lcdbuf);
}

static inline void button_beep(void) {
    if (movement_button_should_sound()) {
        watch_buzzer_play_note_with_volume(BUZZER_NOTE_C7, 50, movement_button_volume());
    }
}

//
// Exported functions
//

void smart_alarm_face_setup(uint8_t watch_face_index, void **context_ptr) {
    (void) watch_face_index;

    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(smart_alarm_state_t));
        smart_alarm_state_t *state = (smart_alarm_state_t *)*context_ptr;
        memset(*context_ptr, 0, sizeof(smart_alarm_state_t));

        // Default alarm window: 06:45-07:15 (27 to 29 in 15-min increments)
        // 06:45 = (6 * 4) + 3 = 27
        // 07:15 = (7 * 4) + 1 = 29
        state->window_start = 27;  // 06:45
        state->window_end = 29;    // 07:15
        state->alarm_enabled = 0;
        state->setting_mode = SMART_ALARM_SETTING_NONE;
    }
}

void smart_alarm_face_activate(void *context) {
    smart_alarm_state_t *state = (smart_alarm_state_t *)context;
    state->setting_mode = SMART_ALARM_SETTING_NONE;
}

void smart_alarm_face_resign(void *context) {
    (void) context;
}

bool smart_alarm_face_loop(movement_event_t event, void *context) {
    smart_alarm_state_t *state = (smart_alarm_state_t *)context;

    switch (event.event_type) {
        case EVENT_ACTIVATE:
            watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "SmA", "SA");
            if (state->alarm_enabled) {
                watch_set_indicator(WATCH_INDICATOR_SIGNAL);
            }
            watch_set_colon();
            _smart_alarm_display_window(state);
            break;

        case EVENT_TICK:
            // In normal mode, no action needed
            if (state->setting_mode == SMART_ALARM_SETTING_NONE) {
                break;
            }

            // In setting mode, blink the time being set
            _smart_alarm_display_window(state);
            if (event.subsecond % 2 == 0) {
                watch_display_text(WATCH_POSITION_BOTTOM, "      ");
            }
            break;

        case EVENT_LIGHT_BUTTON_DOWN:
            switch (state->setting_mode) {
                case SMART_ALARM_SETTING_NONE:
                    // Normal mode: illuminate LED
                    movement_illuminate_led();
                    break;
                case SMART_ALARM_SETTING_WINDOW_START:
                    // Move to setting window end
                    state->setting_mode = SMART_ALARM_SETTING_WINDOW_END;
                    break;
                case SMART_ALARM_SETTING_WINDOW_END:
                    // Done setting, return to normal mode
                    state->setting_mode = SMART_ALARM_SETTING_NONE;
                    movement_request_tick_frequency(1);
                    button_beep();
                    // Enable alarm since user just configured it
                    state->alarm_enabled = 1;
                    movement_set_alarm_enabled(true);
                    watch_set_indicator(WATCH_INDICATOR_SIGNAL);
                    _smart_alarm_display_window(state);
                    break;
            }
            break;

        case EVENT_ALARM_BUTTON_UP:
            if (state->setting_mode == SMART_ALARM_SETTING_NONE) {
                // Toggle alarm on/off
                state->alarm_enabled ^= 1;
                if (state->alarm_enabled) {
                    watch_set_indicator(WATCH_INDICATOR_SIGNAL);
                    movement_set_alarm_enabled(true);
                } else {
                    watch_clear_indicator(WATCH_INDICATOR_SIGNAL);
                    movement_set_alarm_enabled(false);
                }
            }
            break;

        case EVENT_ALARM_BUTTON_DOWN:
            switch (state->setting_mode) {
                case SMART_ALARM_SETTING_NONE:
                    // Nothing to do, alarm toggle handled in BUTTON_UP
                    break;
                case SMART_ALARM_SETTING_WINDOW_START:
                    // Increment window start by 15 minutes
                    state->window_start = (state->window_start + 1) % 96;
                    // Ensure window end is after window start
                    if (state->window_end <= state->window_start) {
                        state->window_end = (state->window_start + 1) % 96;
                    }
                    _smart_alarm_display_window(state);
                    break;
                case SMART_ALARM_SETTING_WINDOW_END:
                    // Increment window end by 15 minutes
                    state->window_end = (state->window_end + 1) % 96;
                    // Ensure window end is after window start
                    if (state->window_end <= state->window_start) {
                        state->window_end = (state->window_start + 1) % 96;
                    }
                    _smart_alarm_display_window(state);
                    break;
            }
            break;

        case EVENT_ALARM_LONG_PRESS:
            if (state->setting_mode == SMART_ALARM_SETTING_NONE) {
                // Enter setting mode for window start
                state->setting_mode = SMART_ALARM_SETTING_WINDOW_START;
                movement_request_tick_frequency(4);
                button_beep();
            }
            break;

        case EVENT_BACKGROUND_TASK:
            // Alarm triggered - play Fairy Fountain wake sequence
            movement_play_sequence((int8_t *)smart_alarm_tune, BUZZER_PRIORITY_ALARM);
            break;

        case EVENT_TIMEOUT:
            movement_move_to_face(0);
            break;

        case EVENT_LOW_ENERGY_UPDATE:
            break;

        default:
            movement_default_loop_handler(event);
            break;
    }

    return true;
}

movement_watch_face_advisory_t smart_alarm_face_advise(void *context) {
    smart_alarm_state_t *state = (smart_alarm_state_t *)context;
    movement_watch_face_advisory_t retval = { 0 };

    if (state->alarm_enabled) {
        watch_date_time_t now = movement_get_local_date_time();
        
        // Convert current time to increment index
        uint8_t current_increment = _time_to_increment(now.unit.hour, now.unit.minute);
        
        // Check if we're within the alarm window
        // Note: This is a simplified check. The actual smart alarm logic
        // (monitoring for light sleep) is handled in movement.c via the
        // is_approaching_alarm_window() and is_light_sleep_detected() functions.
        
        // Request background task if we're at or past the window end time
        // (fallback behavior if no light sleep detected)
        retval.wants_background_task = (current_increment == state->window_end);
    }

    return retval;
}
