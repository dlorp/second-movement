# Phase Engine

Context-aware circadian rhythm tracking for Sensor Watch Pro.

## Overview

The Phase Engine computes a real-time "phase score" (0-100) representing how well your current activity aligns with your circadian rhythm. Unlike simple sleep tracking, it combines:

- **Seasonal data** (daylight, temperature) via location-specific homebase table
- **Time of day** (circadian curve)
- **Activity level** (movement, exertion)
- **Environmental inputs** (temperature, light exposure)

All computations use **integer math only** (no floating point) for embedded efficiency.

## Architecture

```
lib/phase/
├── phase_engine.h       # Core API and data structures
├── phase_engine.c       # Phase computation logic
├── homebase.h           # Homebase table interface
├── homebase_table.h     # Generated LUT (365 days of seasonal data)
└── README.md            # This file
```

### Flash Budget

- **Homebase table:** ~8 KB (365 entries × ~22 bytes)
- **Phase engine code:** ~4 KB (computation logic)
- **Total:** ~12 KB (well within 15-25 KB budget)

### RAM Budget

- **Phase state:** 64 bytes (includes 24-hour history buffer)

## Homebase Table

The homebase table is a location-specific lookup table (LUT) with 365 entries (one per day of year). Each entry contains:

| Field | Type | Description | Example |
|-------|------|-------------|---------|
| `expected_daylight_min` | `uint16_t` | Sunrise to sunset duration (minutes) | `570` = 9h 30m |
| `avg_temp_c10` | `int16_t` | Average temperature (celsius × 10) | `125` = 12.5°C |
| `seasonal_baseline` | `uint8_t` | Seasonal energy baseline (0-100) | `65` = moderate |

**Why?** This table provides seasonal context without needing GPS or network access. The watch "knows" what's normal for your location and time of year.

### Generating the Homebase Table

The homebase table is generated at build time using `utils/generate_homebase.py`:

```bash
# San Francisco (default)
python3 utils/generate_homebase.py --lat 37.7749 --lon -122.4194 --tz PST --year 2026

# New York
python3 utils/generate_homebase.py --lat 40.7128 --lon -74.0060 --tz EST --year 2026

# London
python3 utils/generate_homebase.py --lat 51.5074 --lon -0.1278 --tz UTC --year 2026

# Tokyo
python3 utils/generate_homebase.py --lat 35.6762 --lon 139.6503 --tz UTC+9 --year 2026
```

**Output:** Replaces `lib/phase/homebase_table.h` with location-specific data.

**When to regenerate:**
- Moving to a new location (>100 miles latitude change)
- Timezone change
- Annual update (optional - seasonal patterns don't change much)

## Phase Engine API

### Initialization

```c
#include "phase_engine.h"

phase_state_t state;
phase_engine_init(&state);
```

### Computing Phase Score

```c
// Gather inputs
uint8_t hour = current_datetime.unit.hour;
uint8_t day_of_year = /* compute from date */;
uint16_t activity_level = /* from accelerometer, 0-1000 */;
int16_t temp_c10 = /* from sensor, e.g., 235 = 23.5°C */;
uint16_t light_lux = /* from light sensor, e.g., 500 */;

// Compute phase
uint16_t phase_score = phase_compute(&state, hour, day_of_year, 
                                      activity_level, temp_c10, light_lux);

// phase_score: 0-100
// - 0-30: Poor alignment (rest recommended)
// - 30-60: Moderate alignment
// - 60-80: Good alignment
// - 80-100: Excellent alignment (peak performance)
```

### Getting Recommendations

```c
uint8_t action = phase_get_recommendation(phase_score, hour);

// action codes:
// 0 = Rest (low phase, conserve energy)
// 1 = Moderate activity
// 2 = Active (good phase for work/exercise)
// 3 = Peak performance (optimal phase)
```

### Trend Analysis

```c
// Get phase trend over last 6 hours
int16_t trend = phase_get_trend(&state, 6);

// trend: -100 (declining) to +100 (improving)
// Useful for predicting energy crashes or peaks
```

## Integration with Watch Faces

### Option 1: Direct Integration

```c
#include "phase_engine.h"

typedef struct {
    // ... existing face state ...
    #ifdef PHASE_ENGINE_ENABLED
    phase_state_t phase_state;
    #endif
} my_face_state_t;

void my_face_setup(movement_settings_t *settings, uint8_t watch_face_index, void **context_ptr) {
    // ... existing setup ...
    
    #ifdef PHASE_ENGINE_ENABLED
    my_face_state_t *state = (my_face_state_t *)*context_ptr;
    phase_engine_init(&state->phase_state);
    #endif
}

void my_face_loop(movement_event_t event, movement_settings_t *settings, void *context) {
    my_face_state_t *state = (my_face_state_t *)context;
    
    #ifdef PHASE_ENGINE_ENABLED
    if (event.event_type == EVENT_TICK) {
        // Update phase score every minute
        watch_date_time_t dt = watch_rtc_get_date_time();
        uint8_t hour = dt.unit.hour;
        uint8_t day_of_year = /* compute */;
        
        uint16_t phase = phase_compute(&state->phase_state, hour, day_of_year,
                                        /* activity */, /* temp */, /* light */);
        
        // Display or use phase score
        printf("Phase: %d\n", phase);
    }
    #endif
}
```

### Option 2: Shared Global State

If multiple faces need phase data, consider a global state in `movement.c`:

```c
// movement.c
#ifdef PHASE_ENGINE_ENABLED
static phase_state_t global_phase_state;

void movement_init(void) {
    phase_engine_init(&global_phase_state);
}

phase_state_t* movement_get_phase_state(void) {
    return &global_phase_state;
}
#endif
```

Then faces can access it via `movement_get_phase_state()`.

## Enabling/Disabling Phase Engine

The phase engine is **disabled by default** to maintain backward compatibility.

### To Enable

Add to `movement_config.h`:

```c
#define PHASE_ENGINE_ENABLED
```

### Flash Size Impact

- **Disabled:** 0 bytes (all code is `#ifdef`'d out)
- **Enabled (stubs only):** ~500 bytes (Phase 1)
- **Enabled (full logic):** ~12 KB (Phase 2+)

### Testing Both Modes

```bash
# Test disabled (default)
make clean && make
arm-none-eabi-size build/watch.elf

# Test enabled
echo "#define PHASE_ENGINE_ENABLED" >> movement_config.h
make clean && make
arm-none-eabi-size build/watch.elf

# Clean up
git checkout movement_config.h
```

## Implementation Status

### Phase 1: Scaffolding ✓

- [x] Directory structure
- [x] Homebase generator script
- [x] Phase engine headers
- [x] Stub implementations (return neutral values)
- [x] `#ifdef` infrastructure
- [x] Documentation

**Current state:** Compiles cleanly with/without `PHASE_ENGINE_ENABLED`, but returns neutral scores (50/100).

### Phase 2: Metrics (TODO)

- [ ] Implement homebase table lookups
- [ ] Circadian curve computation (cosine approximation LUT)
- [ ] Activity deviation scoring
- [ ] Temperature deviation scoring
- [ ] Light exposure scoring
- [ ] Weighted combination algorithm

### Phase 3: Optimization (TODO)

- [ ] Flash size profiling
- [ ] LUT compression (if needed)
- [ ] RAM optimization
- [ ] Power profiling

### Phase 4: Watch Faces (TODO)

- [ ] Dedicated phase display face
- [ ] Integration with existing faces (sleep tracker, activity, etc.)
- [ ] Phase history visualization

## Design Decisions

### Why Integer Math?

SAM L22 has no hardware FPU. Floating point operations are **50-100× slower** than integer operations. For a battery-powered watch, this matters.

**Example:** Temperature stored as `celsius × 10`:
- `23.5°C` → `235` (int16_t)
- `12.5°C` → `125` (int16_t)

### Why a Homebase Table?

Alternative approaches considered:

1. **Real-time solar calculations** - Too slow, burns power
2. **Network lookups** - No network on watch
3. **GPS-based** - No GPS, high power draw
4. **Generic seasonal curve** - Inaccurate (e.g., Seattle vs Phoenix)

**Homebase table wins:** Fast, accurate, low power, works offline.

### Why 365 Entries (Not 12)?

Daily granularity captures real seasonal transitions:
- Spring/fall equinoxes (rapid daylight changes)
- Gradual temperature shifts
- Location-specific weather patterns

Cost: ~5 KB extra flash (totally acceptable).

### Why Casio Style?

Casio watches (G-SHOCK, Pro Trek) use extensive LUTs for tide tables, sunrise/sunset, moon phases, etc. It's a proven embedded pattern:
- Fast lookups (array indexing)
- Deterministic memory usage
- No runtime computation overhead

## Memory Footprint

### Flash (Program Memory)

| Component | Size | Notes |
|-----------|------|-------|
| Homebase metadata | ~20 bytes | Location, year, etc. |
| Homebase table (365 entries) | ~1,825 bytes | 5 bytes × 365 |
| Homebase accessor functions | ~100 bytes | Bounds checking |
| Phase engine API | ~500 bytes | Stub implementations |
| **Total (Phase 1)** | **~2,445 bytes** | Well under 4 KB budget |

Future phases (with full logic):
| Phase engine computation | ~3-4 KB | Circadian curves, scoring |
| **Total (Phase 2+)** | **~6-8 KB** | Well under 12 KB budget |

### RAM (Runtime Memory)

| Component | Size | Notes |
|-----------|------|-------|
| `phase_state_t` | 64 bytes | Includes 24-hour history |
| Stack usage (worst case) | ~32 bytes | Function calls |
| **Total** | **~96 bytes** | <0.5% of 32 KB RAM |

## Testing

### Unit Tests (Future)

```bash
# TODO: Add to CI pipeline
python3 tests/test_phase_engine.py
```

### Integration Tests

```bash
# Compile with phase engine
echo "#define PHASE_ENGINE_ENABLED" >> movement_config.h
make clean && make

# Verify size
arm-none-eabi-size build/watch.elf

# Expected (Phase 1):
# text     data     bss
# +2500    +0       +0   (vs baseline)
```

## References

### Circadian Research

- **Phillips et al. (2017):** Sleep Regularity Index
- **Cappuccio et al.:** Sleep duration U-curve
- **Czeisler & Gooley (2007):** Light exposure timing

### Implementation Patterns

- `lib/circadian_score.c` - Integer math precedent
- `lib/sunriset/` - Solar calculations (inspiration for homebase)

## FAQ

**Q: Do I need to regenerate the homebase table every year?**  
A: No. Seasonal patterns are stable. Regenerating every 5-10 years is fine.

**Q: What if I travel to a different timezone?**  
A: Phase 4 will add multi-timezone support. For now, regenerate the table for your new location.

**Q: Can I use this without environmental sensors?**  
A: Yes. Pass `0` for temperature and light inputs - the engine will fall back to time-of-day scoring.

**Q: How does this differ from circadian_score?**  
A: `circadian_score` analyzes past sleep data (retrospective). Phase engine tracks real-time alignment (prospective).

**Q: Why not use the existing sunriset library?**  
A: `sunriset` computes sunrise/sunset in real-time (slow). Homebase table precomputes it (fast).

## Contributing

When adding features to the phase engine:

1. **Maintain integer math** - No floats!
2. **Respect budgets** - Flash ≤12 KB, RAM ≤64 bytes
3. **Add `#ifdef` guards** - Must compile when disabled
4. **Update this README** - Document new APIs
5. **Test both modes** - Enabled and disabled builds

## License

MIT License - See `LICENSE.md` in repository root.

---

**Status:** Phase 1 complete (scaffolding).  
**Next:** Phase 2 (metrics computation).
