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

#include "metric_sd.h"
#include "circadian_score.h"

// Sleep target: 8 hours = 480 minutes
#define SD_TARGET_MINUTES 480

uint8_t metric_sd_compute(const circadian_data_t *sleep_data, uint8_t deficits[3]) {
    if (!sleep_data) {
        // No sleep data - return neutral score
        deficits[0] = deficits[1] = deficits[2] = 0;
        return 50;
    }
    
    // Calculate deficit for last 3 nights (most recent first)
    int16_t deficit[3];
    for (int i = 0; i < 3; i++) {
        // Access nights in reverse order (most recent = nights[0] after wraparound)
        // The circadian_data_t uses a circular buffer, so we need to account for that
        int night_idx = (sleep_data->write_index - 1 - i + 7) % 7;
        
        if (!sleep_data->nights[night_idx].valid) {
            // Invalid night - treat as no deficit
            deficit[i] = 0;
        } else {
            int16_t duration = sleep_data->nights[night_idx].duration_min;
            deficit[i] = (SD_TARGET_MINUTES - duration);
            if (deficit[i] < 0) deficit[i] = 0;  // No "credit" for oversleeping
        }
        
        // Store packed deficit (divide by 4 to fit in 0-100 range, max deficit = 480/4 = 120)
        // Clamp to 100 for storage
        deficits[i] = (deficit[i] > 100) ? 100 : (uint8_t)deficit[i];
    }
    
    // Weighted sum: night 0 (50%), night 1 (30%), night 2 (20%)
    int32_t weighted = (deficit[0] * 50 + deficit[1] * 30 + deficit[2] * 20) / 100;
    
    // Clamp to 0-100
    if (weighted > 100) weighted = 100;
    if (weighted < 0) weighted = 0;
    
    return (uint8_t)weighted;
}

#endif // PHASE_ENGINE_ENABLED
