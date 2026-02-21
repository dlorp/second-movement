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

#ifndef METRIC_WK_H_
#define METRIC_WK_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef PHASE_ENGINE_ENABLED

/**
 * WK (Wake Momentum) Metric
 * 
 * Algorithm: Time-based ramp from sleep onset to full alertness
 * 
 * Normal mode (accelerometer available):
 * - Base: 2-hour linear ramp (0-100 over 120 minutes)
 * - Bonus: +30% for high cumulative activity (>1000 units)
 * - Max score: 100 (capped after bonus)
 * 
 * Fallback mode (no accelerometer, e.g., Green board):
 * - Base: 3-hour linear ramp (0-100 over 180 minutes)
 * - No activity bonus
 * - Max score: 100
 * 
 * Storage: 2 bytes in BKUP (wake_onset_hour, wake_onset_minute)
 */

/**
 * Compute Wake Momentum score.
 * 
 * @param minutes_awake Minutes since wake onset (0-1440)
 * @param cumulative_activity Cumulative activity since wake (0-65535)
 * @param has_accelerometer True if LIS2DW accelerometer is available
 * @return WK score (0-100)
 */
uint8_t metric_wk_compute(uint16_t minutes_awake, uint16_t cumulative_activity, bool has_accelerometer);

#endif // PHASE_ENGINE_ENABLED

#endif // METRIC_WK_H_
