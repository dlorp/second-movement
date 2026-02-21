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

#ifndef METRIC_EM_H_
#define METRIC_EM_H_

#include <stdint.h>

#ifdef PHASE_ENGINE_ENABLED

/**
 * EM (Emotional/Circadian Mood) Metric
 * 
 * Algorithm: Three-component blend
 * - Circadian (40%): Daily cycle using cosine curve, peak at hour 14 (2 PM)
 * - Lunar (20%): 29-day cycle approximation, peaks at day 14.5
 * - Variance (40%): Activity variance vs zone expectation (placeholder in Phase 3)
 * 
 * Output: 0 (low mood) to 100 (elevated mood)
 * 
 * No persistent storage (computed fresh each update)
 */

/**
 * Compute Emotional/Mood score.
 * 
 * @param hour Current hour (0-23)
 * @param day_of_year Current day (1-365)
 * @param activity_variance Activity variance over 15 min (0-1000, placeholder)
 * @return EM score (0-100)
 */
uint8_t metric_em_compute(uint8_t hour, uint16_t day_of_year, uint16_t activity_variance);

#endif // PHASE_ENGINE_ENABLED

#endif // METRIC_EM_H_
