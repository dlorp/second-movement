/*
 * MIT License
 *
 * Copyright (c) 2026 Diego Perez
 *
 * Circadian Score v2.0: 75% Evidence-Based Scoring
 * 
 * Component weights:
 * - Sleep Regularity Index (SRI): 35% (Phillips et al. 2017)
 * - Sleep Duration: 30% (Cappuccio et al.)
 * - Sleep Efficiency: 20% (Marino et al.)
 * - Active Hours Compliance: 10%
 * - Light Exposure: 5%
 */

#include "circadian_score.h"
#include "watch.h"
#include "watch_utility.h"
#include <string.h>
#include <stdlib.h>

#define FLASH_ROW_CIRCADIAN 30
#define MINUTES_PER_DAY 1440

// Component weights (scaled to 100)
#define WEIGHT_SRI 35
#define WEIGHT_DURATION 30
#define WEIGHT_EFFICIENCY 20
#define WEIGHT_COMPLIANCE 10
#define WEIGHT_LIGHT 5

// Duration targets and penalties (Cappuccio U-curve)
#define DURATION_OPTIMAL_MIN 420  // 7 hours
#define DURATION_OPTIMAL_MAX 480  // 8 hours
#define DURATION_SHORT_PENALTY 360  // <6h = full penalty
#define DURATION_LONG_PENALTY 540   // >9h = full penalty

uint8_t circadian_score_calculate(const circadian_data_t *data) {
    circadian_score_components_t components;
    circadian_score_calculate_components(data, &components);
    return components.overall_score;
}

void circadian_score_calculate_components(const circadian_data_t *data, 
                                           circadian_score_components_t *components) {
    // Calculate each component
    components->timing_score = circadian_score_calculate_sri(data);
    
    // Duration: average last 7 nights
    uint32_t total_duration = 0;
    uint8_t valid_nights = 0;
    for (uint8_t i = 0; i < 7; i++) {
        if (data->nights[i].valid) {
            total_duration += data->nights[i].duration_min;
            valid_nights++;
        }
    }
    uint16_t avg_duration = valid_nights > 0 ? (total_duration / valid_nights) : 0;
    components->duration_score = circadian_score_calculate_duration(avg_duration);
    
    // Efficiency: average last 7 nights
    uint32_t total_efficiency = 0;
    for (uint8_t i = 0; i < 7; i++) {
        if (data->nights[i].valid) {
            total_efficiency += data->nights[i].efficiency;
        }
    }
    components->efficiency_score = valid_nights > 0 ? (total_efficiency / valid_nights) : 0;
    
    // Compliance: check sleep onset/offset vs Active Hours window
    uint8_t compliant_nights = 0;
    for (uint8_t i = 0; i < 7; i++) {
        if (!data->nights[i].valid) continue;
        
        // Extract hour from timestamps
        watch_date_time_t onset_dt = watch_utility_date_time_from_unix_time(data->nights[i].onset_timestamp, 0);
        watch_date_time_t offset_dt = watch_utility_date_time_from_unix_time(data->nights[i].offset_timestamp, 0);
        
        uint16_t onset_min = onset_dt.unit.hour * 60 + onset_dt.unit.minute;
        uint16_t offset_min = offset_dt.unit.hour * 60 + offset_dt.unit.minute;
        
        // Check if onset is within 1h of Active Hours end
        // and offset is within 1h of Active Hours start
        int16_t onset_delta = (int16_t)onset_min - (int16_t)data->active_hours_end_min;
        if (onset_delta > 720) onset_delta -= 1440;
        else if (onset_delta < -720) onset_delta += 1440;

        int16_t offset_delta = (int16_t)offset_min - (int16_t)data->active_hours_start_min;
        if (offset_delta > 720) offset_delta -= 1440;
        else if (offset_delta < -720) offset_delta += 1440;

        if (abs(onset_delta) <= 60 && abs(offset_delta) <= 60) {
            compliant_nights++;
        }
    }
    components->compliance_score = valid_nights > 0 ? 
        (compliant_nights * 100 / valid_nights) : 0;
    
    // Light: average light quality (% time in darkness)
    uint32_t total_light = 0;
    for (uint8_t i = 0; i < 7; i++) {
        if (data->nights[i].valid) {
            total_light += data->nights[i].light_quality;
        }
    }
    components->light_score = valid_nights > 0 ? (total_light / valid_nights) : 0;
    
    // Overall: weighted combination
    uint32_t weighted_sum = 
        (components->timing_score * WEIGHT_SRI) +
        (components->duration_score * WEIGHT_DURATION) +
        (components->efficiency_score * WEIGHT_EFFICIENCY) +
        (components->compliance_score * WEIGHT_COMPLIANCE) +
        (components->light_score * WEIGHT_LIGHT);
    
    components->overall_score = weighted_sum / 100;
}

uint8_t circadian_score_calculate_sri(const circadian_data_t *data) {
    // Sleep Regularity Index (Phillips et al. 2017)
    // Compares each minute across pairs of days:
    // If same state (both sleep or both wake): +1
    // If different state: -1
    // Final: (matches - mismatches) / total * 50 + 50 = 0-100 scale
    
    // Simplified version: Compare onset/offset times across consecutive nights
    // Perfect match = 100, each hour difference reduces score
    
    uint32_t total_onset_variance = 0;
    uint32_t total_offset_variance = 0;
    uint8_t valid_pairs = 0;
    
    for (uint8_t i = 0; i < 6; i++) {
        if (!data->nights[i].valid || !data->nights[i+1].valid) continue;
        
        // Calculate time-of-day for onset/offset
        watch_date_time_t onset1 = watch_utility_date_time_from_unix_time(data->nights[i].onset_timestamp, 0);
        watch_date_time_t onset2 = watch_utility_date_time_from_unix_time(data->nights[i+1].onset_timestamp, 0);
        watch_date_time_t offset1 = watch_utility_date_time_from_unix_time(data->nights[i].offset_timestamp, 0);
        watch_date_time_t offset2 = watch_utility_date_time_from_unix_time(data->nights[i+1].offset_timestamp, 0);
        
        uint16_t onset1_min = onset1.unit.hour * 60 + onset1.unit.minute;
        uint16_t onset2_min = onset2.unit.hour * 60 + onset2.unit.minute;
        uint16_t offset1_min = offset1.unit.hour * 60 + offset1.unit.minute;
        uint16_t offset2_min = offset2.unit.hour * 60 + offset2.unit.minute;
        
        // Calculate variance (absolute difference) with midnight-crossing wrap
        int16_t onset_diff = (int16_t)onset1_min - (int16_t)onset2_min;
        if (onset_diff > 720) onset_diff -= 1440;
        else if (onset_diff < -720) onset_diff += 1440;
        total_onset_variance += abs(onset_diff);

        int16_t offset_diff = (int16_t)offset1_min - (int16_t)offset2_min;
        if (offset_diff > 720) offset_diff -= 1440;
        else if (offset_diff < -720) offset_diff += 1440;
        total_offset_variance += abs(offset_diff);
        valid_pairs++;
    }
    
    if (valid_pairs == 0) return 50;  // Neutral score if no data
    
    // Average variance in minutes
    uint16_t avg_onset_variance = total_onset_variance / valid_pairs;
    uint16_t avg_offset_variance = total_offset_variance / valid_pairs;
    uint16_t avg_total_variance = (avg_onset_variance + avg_offset_variance) / 2;
    
    // Convert to 0-100 score: 0 variance = 100, 180 min variance = 0
    if (avg_total_variance >= 180) return 0;
    return 100 - (avg_total_variance * 100 / 180);
}

uint8_t circadian_score_calculate_duration(uint16_t duration_min) {
    // Cappuccio U-curve: 7-8h optimal
    // Asymmetric penalties: short sleep worse than long sleep
    
    if (duration_min >= DURATION_OPTIMAL_MIN && duration_min <= DURATION_OPTIMAL_MAX) {
        return 100;  // Optimal range
    }
    
    if (duration_min < DURATION_OPTIMAL_MIN) {
        // Short sleep penalty (steeper)
        if (duration_min <= DURATION_SHORT_PENALTY) return 0;  // <6h = 0
        // Linear penalty from 6h to 7h
        uint16_t deficit = DURATION_OPTIMAL_MIN - duration_min;
        return 100 - (deficit * 100 / 60);  // 60 min range
    } else {
        // Long sleep penalty (gentler)
        if (duration_min >= DURATION_LONG_PENALTY) return 50;  // >9h = 50 (not 0)
        // Linear penalty from 8h to 9h
        uint16_t excess = duration_min - DURATION_OPTIMAL_MAX;
        return 100 - (excess * 50 / 60);  // Half penalty rate
    }
}

uint8_t circadian_score_calculate_sleep_score(const circadian_sleep_night_t *night) {
    if (!night->valid) return 0;
    
    // Single-night quality metric (simpler than full Circadian Score)
    // 50% duration, 30% efficiency, 20% light
    
    uint8_t duration_score = circadian_score_calculate_duration(night->duration_min);
    uint8_t efficiency_score = night->efficiency;
    uint8_t light_score = night->light_quality;
    
    return (duration_score * 50 + efficiency_score * 30 + light_score * 20) / 100;
}

void circadian_data_add_night(circadian_data_t *data, 
                              const circadian_sleep_night_t *night) {
    // Add to circular buffer
    memcpy(&data->nights[data->write_index], night, sizeof(circadian_sleep_night_t));
    data->write_index = (data->write_index + 1) % 7;
    
    // Persist to flash
    circadian_data_save_to_flash(data);
}

bool circadian_data_load_from_flash(circadian_data_t *data) {
    // Read from flash row 30
    bool ok = watch_storage_read(FLASH_ROW_CIRCADIAN, 0,
                                 (uint8_t*)data, sizeof(circadian_data_t));

    // Validate: check return value and write_index range
    if (!ok || data->write_index >= 7) {
        // Corrupted or unreadable data, initialize fresh
        memset(data, 0, sizeof(circadian_data_t));
        return false;
    }

    // Get current time for timestamp validation
    uint32_t now = watch_utility_date_time_to_unix_time(watch_rtc_get_date_time(), 0);
    uint32_t one_year_future = (uint32_t)((uint64_t)now + (365UL * 24 * 60 * 60));
    
    // Validate each night's data
    for (uint8_t i = 0; i < 7; i++) {
        circadian_sleep_night_t *night = &data->nights[i];
        
        // Skip already-invalid nights
        if (!night->valid) continue;
        
        // Validate timestamps: must be non-zero and not wildly in the future
        if (night->onset_timestamp == 0 || night->offset_timestamp == 0 ||
            night->onset_timestamp > one_year_future || night->offset_timestamp > one_year_future) {
            night->valid = false;
            continue;
        }
        
        // Validate timestamp ordering: offset must be after onset
        if (night->offset_timestamp <= night->onset_timestamp) {
            night->valid = false;
            continue;
        }
        
        // Validate duration: must be in range 0-1440 minutes (24 hours)
        if (night->duration_min > MINUTES_PER_DAY) {
            night->valid = false;
            continue;
        }
        
        // Validate duration consistency: should match timestamp delta within tolerance
        uint16_t calculated_duration = (night->offset_timestamp - night->onset_timestamp) / 60;
        if (abs(calculated_duration - night->duration_min) > 5) {  // 5-min tolerance
            night->valid = false;
            continue;
        }
        
        // Clamp percentage fields to valid range
        if (night->efficiency > 100) night->efficiency = 100;
        if (night->light_quality > 100) night->light_quality = 100;
    }

    return true;
}

bool circadian_data_save_to_flash(const circadian_data_t *data) {
    return watch_storage_write(FLASH_ROW_CIRCADIAN, 0, 
                                (const uint8_t*)data, sizeof(circadian_data_t));
}

uint16_t circadian_data_export_binary(const circadian_data_t *data, uint8_t *buffer, uint16_t buffer_size) {
    // Each night: 16 bytes (4 + 4 + 2 + 1 + 2 + 1 + 1 + 1 = 16, no padding)
    // 7 nights = 112 bytes minimum
    // Compression: 287 → 112 bytes (-61%) = ~3 min → ~1 min transmission
    if (buffer_size < 7 * CIRCADIAN_EXPORT_NIGHT_BYTES) {
        return 0;  // Buffer too small
    }
    
    uint16_t offset = 0;
    
    // Export 7 nights in chronological order (starting from write_index)
    for (uint8_t i = 0; i < 7; i++) {
        uint8_t idx = (data->write_index + i) % 7;
        const circadian_sleep_night_t *night = &data->nights[idx];
        
        // Pack into binary format (little-endian, tightly packed)
        // Onset timestamp (4 bytes)
        buffer[offset++] = (night->onset_timestamp >> 0) & 0xFF;
        buffer[offset++] = (night->onset_timestamp >> 8) & 0xFF;
        buffer[offset++] = (night->onset_timestamp >> 16) & 0xFF;
        buffer[offset++] = (night->onset_timestamp >> 24) & 0xFF;
        
        // Offset timestamp (4 bytes)
        buffer[offset++] = (night->offset_timestamp >> 0) & 0xFF;
        buffer[offset++] = (night->offset_timestamp >> 8) & 0xFF;
        buffer[offset++] = (night->offset_timestamp >> 16) & 0xFF;
        buffer[offset++] = (night->offset_timestamp >> 24) & 0xFF;
        
        // Duration (2 bytes)
        buffer[offset++] = (night->duration_min >> 0) & 0xFF;
        buffer[offset++] = (night->duration_min >> 8) & 0xFF;
        
        // Efficiency (1 byte)
        buffer[offset++] = night->efficiency;
        
        // WASO (2 bytes)
        buffer[offset++] = (night->waso_min >> 0) & 0xFF;
        buffer[offset++] = (night->waso_min >> 8) & 0xFF;
        
        // Awakenings (1 byte)
        buffer[offset++] = night->awakenings;
        
        // Light quality (1 byte)
        buffer[offset++] = night->light_quality;
        
        // Valid flag (1 byte)
        buffer[offset++] = night->valid ? 1 : 0;
    }
    
    return offset;  // Should be exactly 112 bytes
}
