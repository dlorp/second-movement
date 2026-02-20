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

#include "metrics.h"
#include "metric_sd.h"
#include "metric_comfort.h"
#include "circadian_score.h"
#include "phase_engine.h"
#include "watch.h"

// Forward declaration from movement (defined in movement.c)
extern uint8_t movement_claim_backup_register(void);

// Current metric values (computed on each update)
static metrics_snapshot_t _current_metrics;

void metrics_init(metrics_engine_t *engine) {
    if (!engine) return;
    
    // Claim BKUP registers for persistent storage
    // Need 2 registers total: one for SD (3 bytes), one for WK (2 bytes)
    engine->bkup_reg_sd = movement_claim_backup_register();
    engine->bkup_reg_wk = movement_claim_backup_register();
    
    // Initialize state
    engine->sd_deficits[0] = 0;
    engine->sd_deficits[1] = 0;
    engine->sd_deficits[2] = 0;
    engine->wake_onset_hour = 0;
    engine->wake_onset_minute = 0;
    engine->last_update_hour = 0;
    engine->initialized = true;
    
    // Initialize current metrics to neutral
    _current_metrics.sd = 50;
    _current_metrics.em = 50;
    _current_metrics.wk = 50;
    _current_metrics.energy = 50;
    _current_metrics.comfort = 50;
    
    // Try to load from BKUP if registers were claimed successfully
    if (engine->bkup_reg_sd != 0 && engine->bkup_reg_wk != 0) {
        metrics_load_bkup(engine);
    }
}

void metrics_update(metrics_engine_t *engine,
                    uint8_t hour,
                    uint8_t minute,
                    uint16_t day_of_year,
                    uint8_t phase_score,
                    uint16_t activity_level,
                    int16_t temp_c10,
                    uint16_t light_lux,
                    const circadian_data_t *sleep_data,
                    const homebase_entry_t *homebase) {
    
    if (!engine || !engine->initialized) return;
    
    // Update cadence tracking
    engine->last_update_hour = hour;
    
    // --- Sleep Debt (SD) ---
    // Compute from sleep history and store deficits
    _current_metrics.sd = metric_sd_compute(sleep_data, engine->sd_deficits);
    
    // --- Comfort ---
    // Compute from current sensors + homebase
    _current_metrics.comfort = metric_comfort_compute(temp_c10, light_lux, hour, homebase);
    
    // --- Emotional (EM) ---
    // Phase 3A PR 1: Stub (implement in PR 2)
    _current_metrics.em = 50;  // Neutral for now
    
    // --- Wake Momentum (WK) ---
    // Phase 3A PR 1: Stub (implement in PR 2)
    _current_metrics.wk = 50;  // Neutral for now
    
    // --- Energy ---
    // Phase 3A PR 1: Stub (implement in PR 2)
    _current_metrics.energy = 50;  // Neutral for now
    
    // Auto-save to BKUP on hourly boundary
    // (Only save SD and WK state, other metrics are derived)
    if (engine->bkup_reg_sd != 0 && engine->bkup_reg_wk != 0) {
        metrics_save_bkup(engine);
    }
}

void metrics_get(const metrics_engine_t *engine, metrics_snapshot_t *out) {
    if (!out) return;
    
    // Copy current metrics to output
    *out = _current_metrics;
}

void metrics_save_bkup(const metrics_engine_t *engine) {
    if (!engine || engine->bkup_reg_sd == 0 || engine->bkup_reg_wk == 0) return;
    
    // Pack SD state into BKUP[bkup_reg_sd] (3 bytes)
    // Format: [deficit[0], deficit[1], deficit[2], unused]
    uint32_t sd_data = 0;
    sd_data |= (uint32_t)engine->sd_deficits[0];
    sd_data |= ((uint32_t)engine->sd_deficits[1]) << 8;
    sd_data |= ((uint32_t)engine->sd_deficits[2]) << 16;
    watch_store_backup_data(sd_data, engine->bkup_reg_sd);
    
    // Pack WK state into BKUP[bkup_reg_wk] (2 bytes)
    // Format: [wake_onset_hour, wake_onset_minute, unused, unused]
    uint32_t wk_data = 0;
    wk_data |= (uint32_t)engine->wake_onset_hour;
    wk_data |= ((uint32_t)engine->wake_onset_minute) << 8;
    watch_store_backup_data(wk_data, engine->bkup_reg_wk);
}

void metrics_load_bkup(metrics_engine_t *engine) {
    if (!engine || engine->bkup_reg_sd == 0 || engine->bkup_reg_wk == 0) return;
    
    // Load SD state from BKUP
    uint32_t sd_data = watch_get_backup_data(engine->bkup_reg_sd);
    engine->sd_deficits[0] = (uint8_t)(sd_data & 0xFF);
    engine->sd_deficits[1] = (uint8_t)((sd_data >> 8) & 0xFF);
    engine->sd_deficits[2] = (uint8_t)((sd_data >> 16) & 0xFF);
    
    // Load WK state from BKUP
    uint32_t wk_data = watch_get_backup_data(engine->bkup_reg_wk);
    engine->wake_onset_hour = (uint8_t)(wk_data & 0xFF);
    engine->wake_onset_minute = (uint8_t)((wk_data >> 8) & 0xFF);
}

#endif // PHASE_ENGINE_ENABLED
