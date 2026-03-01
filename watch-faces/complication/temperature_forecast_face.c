/*
 * MIT License
 *
 * Copyright (c) 2026 Diego Perez
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
#include "temperature_forecast_face.h"
#include "watch.h"
#include "watch_utility.h"

#ifdef PHASE_ENGINE_ENABLED
#include "../../lib/phase/forecast_table.h"
#include "../../lib/phase/homebase.h"
#endif

static const char DAY_ABBR[7][3] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};

static void display_forecast(temperature_forecast_state_t *state, movement_settings_t *settings);

// Get weekday index (0=Sunday, 6=Saturday) using Zeller's congruence
static uint8_t get_weekday_idx(watch_date_time_t date_time) {
    date_time.unit.year += 20;
    if (date_time.unit.month <= 2) {
        date_time.unit.month += 12;
        date_time.unit.year--;
    }
    return (date_time.unit.day + 13 * (date_time.unit.month + 1) / 5 + date_time.unit.year + date_time.unit.year / 4 + 525 - 2) % 7;
}

void temperature_forecast_face_setup(movement_settings_t *settings, uint8_t watch_face_index, void ** context_ptr) {
    (void) settings;
    (void) watch_face_index;
    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(temperature_forecast_state_t));
        memset(*context_ptr, 0, sizeof(temperature_forecast_state_t));
    }
}

void temperature_forecast_face_activate(movement_settings_t *settings, void *context) {
    (void) settings;
    temperature_forecast_state_t *state = (temperature_forecast_state_t *)context;
    display_forecast(state, settings);
}

bool temperature_forecast_face_loop(movement_event_t event, movement_settings_t *settings, void *context) {
    temperature_forecast_state_t *state = (temperature_forecast_state_t *)context;

    switch (event.event_type) {
        case EVENT_MODE_BUTTON_UP:
            // Cycle through days: 0 → 1 → 2 → ... → 6 → 0
            state->current_day = (state->current_day + 1) % 7;
            display_forecast(state, settings);
            break;
        case EVENT_ALARM_BUTTON_UP:
            // Toggle between high and low temperature
            state->show_low_temp = !state->show_low_temp;
            display_forecast(state, settings);
            break;
        case EVENT_ACTIVATE:
            display_forecast(state, settings);
            break;
        case EVENT_TIMEOUT:
            // Don't timeout
            break;
        default:
            return movement_default_loop_handler(event);
    }

    return true;
}

void temperature_forecast_face_resign(movement_settings_t *settings, void *context) {
    (void) settings;
    (void) context;
}

static void display_forecast(temperature_forecast_state_t *state, movement_settings_t *settings) {
    (void) settings;
    char buf[11];
    int16_t temp_c10 = 0;
    bool has_data = false;

#ifdef PHASE_ENGINE_ENABLED
    // Try to get forecast data
    const homebase_entry_t *entry = forecast_get_entry(state->current_day);
    
    if (entry != NULL && forecast_table_valid) {
        // Use forecast data
        temp_c10 = entry->avg_temp_c10;
        has_data = true;
        watch_set_indicator(WATCH_INDICATOR_SIGNAL); // Show "FC" indicator
    } else {
        // Fallback to homebase seasonal average
        watch_date_time_t now = watch_rtc_get_date_time();
        uint16_t day_of_year = watch_utility_days_since_new_year(
            now.unit.year + WATCH_RTC_REFERENCE_YEAR,
            now.unit.month, 
            now.unit.day
        );
        // Add current_day offset to get future day
        day_of_year = (day_of_year + state->current_day - 1) % 365 + 1;
        
        const homebase_entry_t *homebase = homebase_get_entry(day_of_year);
        if (homebase != NULL) {
            temp_c10 = homebase->avg_temp_c10;
            has_data = true;
        }
        watch_clear_indicator(WATCH_INDICATOR_SIGNAL);
    }
#endif

    if (!has_data) {
        // No data available
        watch_display_text_with_fallback(WATCH_POSITION_TOP, "no da", "noda");
        watch_clear_indicator(WATCH_INDICATOR_SIGNAL);
        return;
    }

    // Calculate which day of week this forecast is for
    watch_date_time_t now = watch_rtc_get_date_time();
    uint8_t current_weekday = get_weekday_idx(now);
    uint8_t forecast_day_of_week = (current_weekday + state->current_day) % 7;

    // Display day abbreviation
    sprintf(buf, "%s", DAY_ABBR[forecast_day_of_week]);
    watch_display_text_with_fallback(WATCH_POSITION_TOP, buf, buf);

    // Convert temperature from c10 to Celsius and display
    // temp_c10 is in tenths of degrees Celsius
    int16_t temp_c = temp_c10 / 10;
    
    // Display temperature with sign
    // Format: "+15" or "-05"
    if (temp_c >= 0) {
        sprintf(buf, "  %c%2d", '+', temp_c);
    } else {
        sprintf(buf, "  %c%2d", '-', -temp_c);
    }
    watch_display_text_with_fallback(WATCH_POSITION_BOTTOM, buf, buf);

    // Show indicator for high/low temp mode
    if (state->show_low_temp) {
        watch_set_indicator(WATCH_INDICATOR_LAP);
    } else {
        watch_clear_indicator(WATCH_INDICATOR_LAP);
    }
}
