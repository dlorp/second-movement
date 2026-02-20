/*
 * MIT License
 *
 * Copyright (c) 2026 Diego Perez
 *
 * Homebase Table Interface
 * 
 * Provides access to location-specific seasonal data (daylight,
 * temperature, energy baseline) for phase computation.
 * 
 * The homebase table is generated at build time by
 * utils/generate_homebase.py and compiled into the firmware.
 */

#ifndef HOMEBASE_H_
#define HOMEBASE_H_

#include <stdint.h>
#include "phase_engine.h"

#ifdef PHASE_ENGINE_ENABLED

/**
 * Get homebase entry for a given day of year.
 * 
 * @param day_of_year Day of year (1-365, Jan 1 = 1)
 * @return Pointer to homebase entry (const, in flash)
 */
const homebase_entry_t* homebase_get_entry(uint16_t day_of_year);

/**
 * Get metadata about the homebase table.
 * Used for debugging and configuration validation.
 */
typedef struct {
    int32_t latitude_e6;    // Latitude * 1000000 (e.g., 37770000 for 37.77°N)
    int32_t longitude_e6;   // Longitude * 1000000
    int16_t timezone_offset; // Timezone offset in minutes (e.g., -480 for PST)
    uint16_t year;          // Year used for generation (e.g., 2026)
    uint16_t entry_count;   // Number of entries (should be 365)
} homebase_metadata_t;

const homebase_metadata_t* homebase_get_metadata(void);

#endif // PHASE_ENGINE_ENABLED

#endif // HOMEBASE_H_
