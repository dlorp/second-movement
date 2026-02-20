/*
 * MIT License
 *
 * Copyright (c) 2026 Diego Perez
 *
 * Phase Engine: Context-aware circadian rhythm tracking
 * 
 * This module computes a real-time "phase score" (0-100) representing
 * circadian alignment based on:
 * - Time of day and season (via homebase table)
 * - Current activity level
 * - Environmental inputs (temperature, light)
 * 
 * All computations use integer math for embedded efficiency.
 */

#ifndef PHASE_ENGINE_H_
#define PHASE_ENGINE_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef PHASE_ENGINE_ENABLED

// Phase engine state (≤64 bytes RAM budget)
typedef struct {
    uint16_t last_phase_score;      // Most recent phase score (0-100)
    uint8_t last_hour;              // Last computed hour (0-23)
    uint16_t last_day_of_year;      // Last computed day (1-365), requires 9 bits
    uint16_t cumulative_phase;      // Rolling 24h sum for trends
    uint8_t phase_history[24];      // Hourly phase scores (circular buffer)
    uint8_t history_index;          // Current position in circular buffer
    bool initialized;               // Has engine been initialized?
} phase_state_t;

// Homebase data point (one per day-of-year)
typedef struct {
    uint16_t expected_daylight_min; // Expected daylight duration (minutes)
    int16_t avg_temp_c10;           // Average temperature (celsius * 10)
    uint8_t seasonal_baseline;      // Seasonal energy baseline (0-100)
} homebase_entry_t;

/**
 * Initialize the phase engine state.
 * Call once at startup.
 */
void phase_engine_init(phase_state_t *state);

/**
 * Compute current phase score (0-100).
 * 
 * @param state Engine state (updated in-place)
 * @param hour Current hour (0-23)
 * @param day_of_year Current day (1-365)
 * @param activity_level Recent activity (0-1000, arbitrary units)
 * @param temp_c10 Current temperature (celsius * 10)
 * @param light_lux Current light level (lux)
 * @return Phase score (0-100), higher = better alignment
 */
uint16_t phase_compute(phase_state_t *state,
                       uint8_t hour,
                       uint8_t day_of_year,
                       uint16_t activity_level,
                       int16_t temp_c10,
                       uint16_t light_lux);

/**
 * Get phase trend over last N hours.
 * 
 * @param state Engine state
 * @param hours Number of hours to analyze (1-24)
 * @return Trend: -100 (declining) to +100 (improving)
 */
int16_t phase_get_trend(const phase_state_t *state, uint8_t hours);

/**
 * Get recommended action based on current phase.
 * 
 * @param phase_score Current phase score (0-100)
 * @param hour Current hour (0-23)
 * @return Action code: 0=rest, 1=moderate, 2=active, 3=peak performance
 */
uint8_t phase_get_recommendation(uint16_t phase_score, uint8_t hour);

#endif // PHASE_ENGINE_ENABLED

#endif // PHASE_ENGINE_H_
