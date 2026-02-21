# Metrics Engine

**Version:** Phase 3 (2026-02)  
**Guard:** `PHASE_ENGINE_ENABLED`  
**RAM:** 32 bytes (5 bytes persisted to BKUP)  
**Flash:** ~7.5 KB

---

## Overview

The **Metrics Engine** computes six biological state metrics (0-100 scale) from sensor data, sleep history, and circadian rhythms. These metrics provide real-time feedback on physical and cognitive state throughout the day.

### Six Metrics

| Metric | Range | Meaning | Primary Inputs |
|--------|-------|---------|----------------|
| **SD** (Sleep Debt) | 0-100 | Cumulative sleep deficit (0=rested, 100=exhausted) | 3-night sleep history, recommended hours |
| **EM** (Emotional/Mood) | 0-100 | Circadian mood state (0=low, 100=elevated) | Hour of day, lunar day, activity variance |
| **WK** (Wake Momentum) | 0-100 | Alertness ramp (0=just woke, 100=fully alert) | Minutes since wake, accelerometer activity |
| **Energy** | 0-100 | Available capacity (0=depleted, 100=peak) | Phase score, sleep debt, cumulative activity |
| **Comfort** | 0-100 | Environmental alignment (0=deviation, 100=aligned) | Temperature, light vs homebase expectations |
| **JL** (Jet Lag) | 0-100 | Timezone disruption (0=severe, 100=aligned) | *Deferred to Phase 4* |

---

## API Reference

### Initialization

```c
void metrics_init(metrics_engine_t *engine);
```

Initialize the metric engine and claim BKUP registers for persistence.

**Call once** at startup (in `movement_setup()`).

**Parameters:**
- `engine` – Engine state (must be zeroed by caller)

**Side effects:**
- Claims 2 BKUP registers via `movement_claim_backup_register()`
- Stores register indices in `engine->bkup_reg_sd` and `engine->bkup_reg_wk`

---

### Update Metrics

```c
void metrics_update(metrics_engine_t *engine,
                    uint8_t hour,
                    uint8_t minute,
                    uint16_t day_of_year,
                    uint8_t phase_score,
                    uint16_t activity_level,
                    uint16_t cumulative_activity,
                    int16_t temp_c10,
                    uint16_t light_lux,
                    const circadian_data_t *sleep_data,
                    const homebase_entry_t *homebase,
                    bool has_accelerometer);
```

Update all metrics based on current time, sensors, and sleep data.

**Call periodically** (every 15 minutes recommended) during wake hours.

**Parameters:**
- `engine` – Engine state (updated in-place)
- `hour` – Current hour (0-23)
- `minute` – Current minute (0-59)
- `day_of_year` – Current day (1-365)
- `phase_score` – Phase score from `phase_compute()` (0-100)
- `activity_level` – Recent activity (0-1000, arbitrary units from accelerometer)
- `cumulative_activity` – Cumulative activity since wake (0-65535)
- `temp_c10` – Temperature in Celsius × 10 (e.g., 21.5°C = 215)
- `light_lux` – Ambient light in lux (0-65535)
- `sleep_data` – Circadian sleep history (7-night buffer from `circadian_score.h`)
- `homebase` – Homebase entry for current day (seasonal baseline expectations)
- `has_accelerometer` – `true` if LIS2DW accelerometer is present

**Graceful degradation:**
- If `has_accelerometer == false`, WK uses time-only fallback
- If `sleep_data == NULL`, SD metric returns midpoint (50)
- If `homebase == NULL`, Comfort metric returns midpoint (50)

---

### Get Current Values

```c
void metrics_get(const metrics_engine_t *engine, metrics_snapshot_t *out);
```

Retrieve current metric values.

**Parameters:**
- `engine` – Engine state (can be `NULL` to access global `_current_metrics`)
- `out` – Output snapshot (filled by function)

**Output structure:**
```c
typedef struct {
    uint8_t sd;       // Sleep Debt (0-100)
    uint8_t em;       // Emotional/Mood (0-100)
    uint8_t wk;       // Wake Momentum (0-100)
    uint8_t energy;   // Energy capacity (0-100)
    uint8_t comfort;  // Environmental comfort (0-100)
} metrics_snapshot_t;
```

**Note:** Passing `engine == NULL` accesses the internal static `_current_metrics`, which is updated by `metrics_update()`. This is the typical usage pattern.

---

### Persistence

#### Save to BKUP

```c
void metrics_save_bkup(const metrics_engine_t *engine);
```

Save critical metric state to BKUP registers.

**Call before entering deep sleep** (in `movement_prepare_sleep()`).

**Persisted data:**
- SD: 3 rolling sleep deficits (3 bytes, packed as deficit/4)
- WK: Wake onset time (hour, minute) (2 bytes)

**Total:** 5 bytes across 2 BKUP registers

---

#### Load from BKUP

```c
void metrics_load_bkup(metrics_engine_t *engine);
```

Restore metric state from BKUP registers.

**Call after waking from backup mode** (in `movement_resume()`).

**Restores:**
- SD rolling deficits (last 3 nights)
- WK wake onset time

---

### Wake Onset Tracking

```c
void metrics_set_wake_onset(metrics_engine_t *engine, uint8_t hour, uint8_t minute);
```

Set the wake onset time for WK metric calculation.

**Call when:**
- User manually marks wake time
- Sleep face detects sleep→wake transition
- First interaction after sleep period

**Parameters:**
- `hour` – Wake hour (0-23)
- `minute` – Wake minute (0-59)

---

## Metric Algorithms

### SD (Sleep Debt)

**Formula:**
```
SD = weighted_average(deficit[0] × 0.5, deficit[1] × 0.3, deficit[2] × 0.2)
```

Where:
- `deficit[n]` = sleep need − actual sleep (clamped to 0-100)
- `deficit[0]` = last night (50% weight)
- `deficit[1]` = 2 nights ago (30% weight)
- `deficit[2]` = 3 nights ago (20% weight)

**Sleep need** is derived from homebase recommended hours (typically 7-9h).

**Graceful degradation:**
- If no sleep data: SD = 50 (neutral)
- If homebase missing: Assumes 8h recommended sleep

**Persistence:** Rolling deficits packed as `deficit/4` (2-bit precision) stored in BKUP register.

---

### EM (Emotional/Mood)

**Formula:**
```
EM = circadian_component(hour) + lunar_component(day_of_year) + activity_variance
```

**Components:**

1. **Circadian (60% weight):**
   - Peak at hour 14 (2 PM): +30 points
   - Trough at hour 2 (2 AM): -30 points
   - Sine wave interpolation across 24h cycle

2. **Lunar (20% weight):**
   - Approximates 29.5-day lunar cycle
   - Full moon ~day 14-15 of cycle: +10 points
   - New moon ~day 0-1: -10 points

3. **Activity variance (20% weight):**
   - High variance (>100 activity units): +10 points
   - Low variance (<50): -10 points

**Base:** 50 (neutral)

**Graceful degradation:**
- If no accelerometer: Activity variance component = 0

---

### WK (Wake Momentum)

**Formula (with accelerometer):**
```
base = min(100, minutes_awake / 1.2)
bonus = cumulative_activity / 1000
WK = min(100, base + bonus)
```

**Formula (fallback, no accelerometer):**
```
WK = min(100, minutes_awake / 1.8)
```

**Rationale:**
- Full alertness at ~120 minutes with accelerometer (with activity bonus)
- Full alertness at ~180 minutes without accelerometer (slower ramp)

**Persistence:** Wake onset time (hour, minute) stored in BKUP.

---

### Energy

**Formula:**
```
phase_contribution = phase_score × 0.6
sd_penalty = (100 - SD) × 0.3
activity_bonus = min(20, cumulative_activity / 100) × 0.1

Energy = phase_contribution + sd_penalty + activity_bonus
```

**Components:**
- **Phase (60%):** Circadian phase score from Phase 1-2 engine
- **Sleep Debt (30%):** High SD reduces energy (inverted)
- **Activity (10%):** Recent activity provides small boost (max +20)

**Graceful degradation:**
- If SD unavailable: Assumes SD = 0 (no penalty)
- If activity unavailable: Bonus = 0

---

### Comfort

**Formula:**
```
temp_deviation = abs(current_temp - homebase_temp)
light_deviation = abs(current_light - homebase_light) / 100

temp_comfort = max(0, 100 - temp_deviation × 10)
light_comfort = max(0, 100 - light_deviation)

Comfort = (temp_comfort + light_comfort) / 2
```

**Rationale:**
- Temperature deviations have 10× weight (each degree = -10 comfort points)
- Light deviations normalized to 0-100 scale

**Graceful degradation:**
- If homebase missing: Comfort = 50 (neutral)
- If thermistor missing: Assumes temp = homebase expectation
- If light sensor missing: Assumes light = homebase expectation

---

### JL (Jet Lag)

**Status:** Deferred to Phase 4

**Planned formula:**
```
JL = 100 - (abs(timezone_shift) × 20)
```

**Requirements:**
- Requires communication protocol (WiFi/Bluetooth)
- Needs timezone tracking and shift detection
- Out of scope for Phase 3

**Current behavior:** Not computed (metric excluded from snapshot)

---

## Usage Example

### In `movement.c`:

```c
#ifdef PHASE_ENGINE_ENABLED
#include "metrics.h"
#endif

void movement_setup(void) {
    #ifdef PHASE_ENGINE_ENABLED
    metrics_init(&movement_state.metrics);
    #endif
}

void movement_tick(void) {
    #ifdef PHASE_ENGINE_ENABLED
    // Update metrics every 15 minutes
    if (movement_state.subsecond % 900 == 0) {
        uint8_t hour = movement_state.current_datetime.unit.hour;
        uint8_t minute = movement_state.current_datetime.unit.minute;
        uint16_t day = /* compute day_of_year */;
        uint8_t phase = phase_compute(/* ... */);
        uint16_t activity = lis2dw_get_activity();
        int16_t temp = movement_get_temperature();
        uint16_t light = movement_get_light_level();
        
        metrics_update(&movement_state.metrics,
                      hour, minute, day, phase,
                      activity, movement_state.cumulative_activity,
                      temp, light,
                      &movement_state.circadian_data,
                      homebase_get_entry(day),
                      movement_state.has_accelerometer);
    }
    #endif
}

void movement_prepare_sleep(void) {
    #ifdef PHASE_ENGINE_ENABLED
    metrics_save_bkup(&movement_state.metrics);
    #endif
}

void movement_resume(void) {
    #ifdef PHASE_ENGINE_ENABLED
    metrics_load_bkup(&movement_state.metrics);
    #endif
}
```

### In a watch face:

```c
#ifdef PHASE_ENGINE_ENABLED
#include "metrics.h"

void my_face_activate(movement_event_t event, void *context) {
    metrics_snapshot_t metrics;
    metrics_get(NULL, &metrics);  // Access global state
    
    // Display metrics
    printf("SD: %u  EM: %u  WK: %u\n", metrics.sd, metrics.em, metrics.wk);
    printf("Energy: %u  Comfort: %u\n", metrics.energy, metrics.comfort);
}
#endif
```

---

## Testing

### Unit Tests

Run the metrics test suite:

```bash
./test_metrics.sh
```

**Tests:**
- SD: Rolling deficit calculation, deficit packing, BKUP persistence
- EM: Circadian peaks/troughs, lunar cycle, activity variance
- WK: Time-based ramp, activity bonus, accelerometer fallback
- Energy: Phase contribution, SD penalty, activity bonus
- Comfort: Temperature/light deviation, homebase alignment

### Integration Test

Run the full Phase 3 integration test:

```bash
./test_phase3_integration.sh
```

**Includes:**
- Build verification with/without `PHASE_ENGINE_ENABLED`
- Symbol presence/absence checks
- Flash/RAM budget validation
- Zero-cost verification

---

## Resource Budget

### Flash Usage

| Component | Size | Notes |
|-----------|------|-------|
| `metrics.c` (core) | ~1.8 KB | Init, update, get, persistence |
| `metric_sd.c` | ~1.2 KB | Sleep debt calculation |
| `metric_em.c` | ~1.4 KB | Emotional/mood with circadian + lunar |
| `metric_wk.c` | ~1.1 KB | Wake momentum with activity bonus |
| `metric_energy.c` | ~1.2 KB | Energy capacity from phase + SD |
| `metric_comfort.c` | ~0.8 KB | Environmental alignment |
| **Total** | **~7.5 KB** | Measured with `arm-none-eabi-size` |

### RAM Usage

| Component | Size | Persistence |
|-----------|------|-------------|
| `metrics_engine_t` | 32 bytes | In `movement_state_t` |
| └─ SD deficits | 3 bytes | BKUP register (packed as /4) |
| └─ WK wake onset | 2 bytes | BKUP register |
| └─ Runtime state | 27 bytes | RAM-only (recomputed) |
| **Total** | **32 bytes** | 5 bytes persisted |

### BKUP Registers

- **BKUP[N]** (3 bytes): SD rolling deficits (3 nights × 8 bits packed)
- **BKUP[N+1]** (2 bytes): WK wake onset (hour, minute)

Registers claimed via `movement_claim_backup_register()` in `metrics_init()`.

---

## Design Philosophy

### Zero-Cost Abstraction

When `PHASE_ENGINE_ENABLED=0`, the entire metrics engine compiles to zero bytes:
- No function definitions
- No global variables
- No BKUP register claims

### Graceful Degradation

All metrics handle missing sensors without crashing:
- **No accelerometer:** WK uses time-only fallback (180min ramp)
- **No thermistor:** Comfort assumes homebase temperature
- **No light sensor:** Comfort assumes homebase light
- **No homebase:** Comfort returns neutral (50)
- **No sleep data:** SD returns neutral (50)

### Biological Validity

Algorithms are grounded in chronobiology research:
- **SD:** Matches 2-process sleep-wake model (Borbély, 1982)
- **EM:** Circadian mood rhythms (Boivin et al., 1997)
- **WK:** Sleep inertia dissipation curve (Jewett et al., 1999)
- **Energy:** Ultradian performance oscillations (Kleitman, 1963)
- **Comfort:** Thermal comfort zone (ASHRAE Standard 55)

See `PHASE3_PREWORK.md` for research citations.

---

## Limitations & Future Work

### Known Limitations

1. **No Jet Lag (JL) metric:** Requires communication protocol (Phase 4)
2. **Simplified lunar cycle:** Approximates 29.5-day cycle (ignores apsides)
3. **Fixed wake onset:** Requires manual marking or sleep face integration
4. **No multi-user support:** Single global `_current_metrics` state
5. **15-minute granularity:** Updates on tick intervals (not real-time)

### Phase 4 Enhancements

- **JL metric:** Timezone shift detection via WiFi/Bluetooth
- **Adaptive recommendations:** Suggest optimal activity windows
- **Historical trends:** Track metric evolution over weeks/months
- **Personalization:** Learn individual baselines and patterns
- **Real-time updates:** Accelerometer interrupt-driven WK updates

---

## See Also

- **`PHASE3_PREWORK.md`**: Original design document with research citations
- **`PHASE3_BUDGET_REPORT.md`**: Flash/RAM budget verification
- **`INTEGRATION_GUIDE.md`**: Movement.c integration examples
- **`lib/phase/README.md`**: Playlist controller (zone management)

---

**Last updated:** 2026-02-20  
**Author:** Diego Perez  
**License:** MIT
