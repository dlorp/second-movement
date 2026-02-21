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

#ifndef METRIC_ENERGY_H_
#define METRIC_ENERGY_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef PHASE_ENGINE_ENABLED

/**
 * Energy Metric (Derived)
 * 
 * Algorithm: Computed from phase score, sleep debt, and activity
 * 
 * Base formula:
 *   energy = phase_score - (sd_score / 3)
 * 
 * Normal mode (accelerometer available):
 *   + Activity bonus: min(20, recent_activity / 50)
 * 
 * Fallback mode (no accelerometer):
 *   + Circadian bonus: (cosine_lut[hour] + 1000) / 100  (range 0-20)
 * 
 * Output: 0 (depleted) to 100 (peak capacity)
 * 
 * No persistent storage (derived from other metrics)
 */

/**
 * Compute Energy capacity score.
 * 
 * @param phase_score Current phase score from phase_engine (0-100)
 * @param sd_score Current Sleep Debt score (0-100)
 * @param recent_activity Recent activity level (0-1000+)
 * @param hour Current hour (0-23, used for fallback mode)
 * @param has_accelerometer True if LIS2DW accelerometer is available
 * @return Energy score (0-100)
 */
uint8_t metric_energy_compute(uint16_t phase_score, uint8_t sd_score, uint16_t recent_activity,
                              uint8_t hour, bool has_accelerometer);

#endif // PHASE_ENGINE_ENABLED

#endif // METRIC_ENERGY_H_
