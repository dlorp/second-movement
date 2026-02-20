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

#include "metric_comfort.h"
#include "phase_engine.h"

// Safe absolute value wrapper to handle INT16_MIN edge case
static inline int16_t safe_abs16(int16_t x) {
    return (x == INT16_MIN) ? INT16_MAX : ((x < 0) ? -x : x);
}

// Helper: min function
static inline uint16_t min16(uint16_t a, uint16_t b) {
    return (a < b) ? a : b;
}

uint8_t metric_comfort_compute(int16_t temp_c10,
                                uint16_t light_lux,
                                uint8_t hour,
                                const homebase_entry_t *baseline) {
    if (!baseline) {
        // No baseline data - return neutral score
        return 50;
    }
    
    // Temp comfort (60%): deviation from seasonal baseline
    // Use int32_t to prevent overflow in subtraction
    int32_t temp_diff = (int32_t)temp_c10 - (int32_t)baseline->avg_temp_c10;
    int32_t temp_dev = (temp_diff < 0) ? -temp_diff : temp_diff;
    
    // Penalty: divide by 3 to convert temp deviation (in 0.1°C) to comfort penalty
    // Cap at 30°C deviation (300 units)
    uint8_t temp_comfort;
    if (temp_dev > 300) {
        temp_comfort = 0;
    } else {
        temp_comfort = 100 - (uint8_t)(temp_dev / 3);
    }
    
    // Light comfort (40%): expected vs actual for hour
    uint8_t light_comfort;
    if (hour >= 6 && hour <= 18) {
        // Daytime (6 AM - 6 PM): expect bright light (>= 200 lux)
        if (light_lux >= 200) {
            light_comfort = 100;
        } else {
            // Penalty for dim daytime: scale 0-200 lux to 0-100 comfort
            light_comfort = (uint8_t)((light_lux * 100) / 200);
        }
    } else {
        // Nighttime: expect dark (<= 50 lux)
        if (light_lux <= 50) {
            light_comfort = 100;
        } else {
            // Penalty for bright nighttime: scale above 50 lux
            // Example: 150 lux at night → 100 lux over threshold → 50 point penalty → 50 comfort
            // Cast to uint32_t to prevent overflow when light_lux is near UINT16_MAX
            uint16_t penalty = min16(100, ((uint32_t)(light_lux - 50)) / 2);
            light_comfort = 100 - (uint8_t)penalty;
        }
    }
    
    // Blend: 60% temp + 40% light
    uint16_t comfort = (temp_comfort * 60 + light_comfort * 40) / 100;
    
    return (uint8_t)comfort;
}

#endif // PHASE_ENGINE_ENABLED
