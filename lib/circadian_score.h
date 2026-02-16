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

#pragma once

#include <stdint.h>
#include <stdbool.h>

// Circadian Score v2.0: 75% evidence-based scoring
// Algorithm calculates a 0-100 score from 5 components:
// - Sleep Regularity Index (SRI): 35% (Phillips et al. 2017)
// - Sleep Duration: 30% (Cappuccio et al. U-curve)
// - Sleep Efficiency: 20% (Marino et al. specificity note)
// - Active Hours Compliance: 10%
// - Light Exposure: 5% (bonus signal)

// Sleep data for a single night
typedef struct {
    uint32_t onset_timestamp;      // Sleep onset (Unix timestamp)
    uint32_t offset_timestamp;     // Sleep offset (Unix timestamp)
    uint16_t duration_min;          // Total sleep duration (minutes)
    uint8_t efficiency;             // Sleep efficiency (0-100%)
    uint16_t waso_min;              // Wake After Sleep Onset (minutes)
    uint8_t awakenings;             // Number of awakenings (>5 min)
    uint8_t light_quality;          // % time in darkness (0-100)
    bool valid;                     // Data is valid for this night
} circadian_sleep_night_t;

// 7-day rolling window for score calculation
typedef struct {
    circadian_sleep_night_t nights[7];  // 7 nights of sleep data
    uint8_t write_index;                 // Circular buffer write position
    uint16_t active_hours_start_min;     // Active hours start (minutes since midnight)
    uint16_t active_hours_end_min;       // Active hours end (minutes since midnight)
} circadian_data_t;

// Component scores for drill-down display
typedef struct {
    uint8_t timing_score;        // SRI (Sleep Regularity Index)
    uint8_t duration_score;      // Sleep duration penalty
    uint8_t efficiency_score;    // Sleep efficiency
    uint8_t compliance_score;    // Active Hours compliance
    uint8_t light_score;         // Light exposure quality
    uint8_t overall_score;       // Combined 0-100 score
} circadian_score_components_t;

// Calculate overall Circadian Score (0-100)
uint8_t circadian_score_calculate(const circadian_data_t *data);

// Calculate individual components (for drill-down)
void circadian_score_calculate_components(const circadian_data_t *data, 
                                           circadian_score_components_t *components);

// Add new night of sleep data to rolling window
void circadian_data_add_night(circadian_data_t *data, 
                              const circadian_sleep_night_t *night);

// Calculate Sleep Regularity Index (SRI) from 7 nights
// Returns 0-100 (higher = more regular)
uint8_t circadian_score_calculate_sri(const circadian_data_t *data);

// Calculate sleep duration score (Cappuccio U-curve)
// Target: 7-8h optimal, penalties for <6h or >9h
// Returns 0-100
uint8_t circadian_score_calculate_duration(uint16_t duration_min);

// Calculate single-night Sleep Score (0-100)
// Combines duration + efficiency + light exposure
// Used by sleep_score_face for quick feedback
uint8_t circadian_score_calculate_sleep_score(const circadian_sleep_night_t *night);

// Load/save from flash (row 30)
bool circadian_data_load_from_flash(circadian_data_t *data);
bool circadian_data_save_to_flash(const circadian_data_t *data);
