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

#pragma once

/*
 * Smart Alarm Face
 *
 * Intelligent alarm that wakes user during light sleep phase within a
 * configured time window, improving wake quality and alertness.
 *
 * Features:
 *   - Set alarm window (e.g., 06:45-07:15) instead of exact time
 *   - Watch monitors sleep phases using accelerometer
 *   - Triggers alarm during light sleep for gentler wake
 *   - Falls back to window end if no light sleep detected
 *   - Works with Stream 1 sleep detection (is_confirmed_asleep)
 *
 * UI:
 *   - Display shows window range: "06:45-07:15"
 *   - Times adjust in 15-minute increments
 *   - LIGHT button: cycle through window start/end setting
 *   - ALARM button short: increment time by 15 minutes
 *   - ALARM button long: toggle smart alarm on/off
 *   - Signal indicator shows when smart alarm is enabled
 *
 * Storage:
 *   - Configuration stored in BKUP[3] register
 *   - Bits 0-6: Window start (0-95, 15-min increments)
 *   - Bits 7-13: Window end (0-95, 15-min increments)
 *   - Bit 14: Smart alarm enabled/disabled
 *   - Bit 15: Reserved (standard alarm fallback)
 *
 * Pre-wake behavior:
 *   - Deep sleep until 15 min before window start
 *   - At T-15min: ramp up accelerometer sampling
 *   - Monitor for light sleep indicators (motion/orientation changes)
 *   - Trigger alarm on light sleep detection within window
 *   - Fallback to window end if no light sleep detected
 *
 * Compatibility:
 *   - Pro board: Full smart alarm with accelerometer tracking
 *   - Green board: Graceful fallback to standard alarm at window end
 */

#include "movement.h"

// Setting modes for the smart alarm UI
typedef enum {
    SMART_ALARM_SETTING_NONE = 0,
    SMART_ALARM_SETTING_WINDOW_START,
    SMART_ALARM_SETTING_WINDOW_END
} smart_alarm_setting_mode_t;

// Smart alarm state structure
// Storage in BKUP[3] allows persistence across power cycles
typedef struct {
    uint32_t window_start : 7;      // 0-95 (15-min increments, 0=00:00, 95=23:45)
    uint32_t window_end : 7;        // 0-95 (15-min increments)
    uint32_t alarm_enabled : 1;     // Smart alarm on/off
    uint32_t reserved : 1;          // Reserved for future use
    smart_alarm_setting_mode_t setting_mode : 2;  // Current UI setting mode
    bool alarming;                  // True while alarm is playing
    uint16_t alarm_ticks;           // Tick counter for LED breathing sync
} smart_alarm_state_t;

void smart_alarm_face_setup(uint8_t watch_face_index, void **context_ptr);
void smart_alarm_face_activate(void *context);
bool smart_alarm_face_loop(movement_event_t event, void *context);
void smart_alarm_face_resign(void *context);
movement_watch_face_advisory_t smart_alarm_face_advise(void *context);

#define smart_alarm_face ((const watch_face_t){ \
    smart_alarm_face_setup, \
    smart_alarm_face_activate, \
    smart_alarm_face_loop, \
    smart_alarm_face_resign, \
    smart_alarm_face_advise \
})
