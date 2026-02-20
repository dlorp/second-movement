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

#ifndef METRIC_COMFORT_H_
#define METRIC_COMFORT_H_

#include <stdint.h>

#ifdef PHASE_ENGINE_ENABLED

#include "phase_engine.h"

/**
 * Comfort Metric
 * 
 * Algorithm: Environmental alignment vs seasonal baseline
 * - Temp comfort (60%): Deviation from homebase avg_temp_c10
 * - Light comfort (40%): Expected vs actual for hour
 * - Output: 0 (extreme deviation) to 100 (aligned)
 * 
 * No storage needed (derived from sensors).
 */

/**
 * Compute Comfort score from current sensors and homebase.
 * 
 * @param temp_c10 Current temperature (celsius * 10)
 * @param light_lux Current light level (lux)
 * @param hour Current hour (0-23)
 * @param baseline Homebase entry for current day (NULL for neutral)
 * @return Comfort score (0-100)
 */
uint8_t metric_comfort_compute(int16_t temp_c10,
                                uint16_t light_lux,
                                uint8_t hour,
                                const homebase_entry_t *baseline);

#endif // PHASE_ENGINE_ENABLED

#endif // METRIC_COMFORT_H_
