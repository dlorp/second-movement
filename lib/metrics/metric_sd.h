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

#ifndef METRIC_SD_H_
#define METRIC_SD_H_

#include <stdint.h>

#ifdef PHASE_ENGINE_ENABLED

#include "circadian_score.h"

/**
 * SD (Sleep Debt) Metric
 * 
 * Algorithm: Rolling 3-night weighted deficit
 * - Target: 480 minutes (8 hours)
 * - Deficit per night = max(0, 480 - actual_duration)
 * - Weighted: night 0 (50%), night 1 (30%), night 2 (20%)
 * - Output: 0 (fully rested) to 100 (exhausted)
 * 
 * Storage: 3 bytes in BKUP (deficit/4 for each night)
 */

/**
 * Compute Sleep Debt score from sleep history.
 * 
 * @param sleep_data Circadian sleep history (7-night buffer)
 * @param deficits Output: 3-night deficit values (0-100 each), written to caller's buffer
 * @return Sleep Debt score (0-100)
 */
uint8_t metric_sd_compute(const circadian_data_t *sleep_data, uint8_t deficits[3]);

#endif // PHASE_ENGINE_ENABLED

#endif // METRIC_SD_H_
