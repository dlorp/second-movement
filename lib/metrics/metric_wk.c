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

#ifdef PHASE_ENGINE_ENABLED

#include "metric_wk.h"

// Activity threshold for full bonus (arbitrary units)
#define WK_ACTIVITY_THRESHOLD 1000

// Ramp durations (minutes)
#define WK_RAMP_NORMAL 120   // 2 hours
#define WK_RAMP_FALLBACK 180 // 3 hours

// Activity bonus (percentage points)
#define WK_ACTIVITY_BONUS 30

uint8_t metric_wk_compute(uint16_t minutes_awake, uint16_t cumulative_activity, bool has_accelerometer) {
    uint8_t wk;
    
    if (has_accelerometer) {
        // Normal mode: 2-hour ramp + activity bonus
        // Base score: linear ramp from 0 to 100 over 120 minutes
        uint16_t base = (minutes_awake * 100) / WK_RAMP_NORMAL;
        if (base > 100) base = 100;
        
        // Activity bonus: +30% if cumulative activity exceeds threshold
        uint8_t bonus = (cumulative_activity >= WK_ACTIVITY_THRESHOLD) ? WK_ACTIVITY_BONUS : 0;
        
        // Combine base + bonus, capped at 100
        wk = (uint8_t)base + bonus;
        if (wk > 100) wk = 100;
        
    } else {
        // Fallback mode (no accelerometer): 3-hour ramp, no bonus
        // Linear ramp from 0 to 100 over 180 minutes
        uint16_t base = (minutes_awake * 100) / WK_RAMP_FALLBACK;
        if (base > 100) base = 100;
        wk = (uint8_t)base;
    }
    
    return wk;
}

#endif // PHASE_ENGINE_ENABLED
