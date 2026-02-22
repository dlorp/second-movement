# Phase 4F: Sleep Mode & Calibration Plan

> **Status:** Planning  
> **Depends on:** Phase 4E (Sleep Tracking + Telemetry)  
> **Goal:** System-level integration and calibration before dogfooding  
> **Priority:** Must-complete before hardware dogfooding begins

---

## Overview

Phase 4F addresses five system-level improvements that make the phase engine behave correctly in real-world conditions — particularly sleep/wake boundaries, Alaska-specific light conditions, and personalized activity baselines.

| # | Feature | Priority | Flash Est. | RAM Est. |
|---|---------|----------|-----------|----------|
| 1 | Active Hours × Sleep Mode | ⭐⭐⭐⭐⭐ | ~400 B | 4 B |
| 2 | EM Light Calibration | ⭐⭐⭐⭐ | ~300 B | 8 B |
| 3 | WK Activity Baseline | ⭐⭐⭐⭐ | ~350 B | 16 B |
| 4 | Hysteresis Tuning | ⭐⭐⭐ | ~50 B | 1 B |
| 5 | Builder UI Improvements | ⭐⭐ | 0 (JS only) | 0 |
| **Total** | | | **~1,100 B** | **29 B** |

Well within the <2 KB flash constraint.

---

## 1. Active Hours × Sleep Mode Integration

### Problem

`playlist.c` line 46: `determine_zone()` maps phase score → zone unconditionally. A 3am bathroom trip generating movement can push phase score up, triggering MOMENTUM or ACTIVE zone — completely wrong at night.

### Architecture

```
playlist_update() entry point:
│
├─ is_sleep_window(hour, config) ?
│   ├─ YES → check sustained_activity_minutes
│   │   ├─ >= 30 min → ALL-NIGHTER override, use normal zones
│   │   └─ < 30 min → LOCK to ZONE_DESCENT or sleep_face
│   │
│   └─ NO → normal zone logic (phase score 0-100 → zones)
```

### Implementation

**New in `playlist.h`:**
```c
// Sleep window check
bool is_sleep_window(uint8_t hour, uint8_t sleep_start, uint8_t sleep_end);
```

**New in `playlist.c`:**
```c
/**
 * Check if current hour falls within sleep window.
 * Handles wrap-around (e.g., 22:00 - 06:00).
 */
static bool is_sleep_window(uint8_t hour, uint8_t sleep_start, uint8_t sleep_end) {
    if (sleep_start > sleep_end) {
        // Wraps midnight: e.g., 22-6 means 22,23,0,1,2,3,4,5
        return (hour >= sleep_start || hour < sleep_end);
    }
    return (hour >= sleep_start && hour < sleep_end);
}
```

**Modify `playlist_update()` (around line 130):**
```c
#ifdef PHASE_ENGINE_ENABLED
void playlist_update(playlist_state_t *state,
                     const metrics_snapshot_t *metrics,
                     const homebase_config_t *config) {
    
    uint8_t hour = /* current hour from RTC */;
    
    // Active hours enforcement
    if (config->active_hours_enabled &&
        is_sleep_window(hour, config->active_hours_start, config->active_hours_end)) {
        
        // Check for all-nighter override
        if (state->sustained_active_minutes >= 30) {
            // Fall through to normal zone logic
        } else {
            // Lock to descent/sleep
            if (state->zone != ZONE_DESCENT) {
                state->zone = ZONE_DESCENT;
                rebuild_rotation(state, metrics);
            }
            return;
        }
    }
    
    // ... existing zone logic ...
}
#endif
```

**Sustained activity tracking** (in `playlist_state_t`):
```c
uint8_t sustained_active_minutes;  // +1 B RAM
// Incremented each minute if movement > threshold
// Reset to 0 when movement drops below threshold for 5+ min
```

**Builder UI** — already stores `active_hours_enabled`, `active_hours_start`, `active_hours_end` (builder/index.html:3664-3666). No UI changes needed; just enforce in firmware.

### Flash: ~400 bytes | RAM: +4 bytes

---

## 2. EM Light Calibration (Configurable Thresholds)

### Problem

`phase_engine.c` line 114 hardcodes:
```c
uint16_t expected_light = (hour >= 6 && hour < 18) ? 500 : 50;
```

Alaska winter: overcast midday is 200-300 lux, sunset at 4pm, snow reflection spikes to 800+ lux. The fixed 500 lux "outdoor" assumption is wrong 6 months of the year.

### Approach: Option A (Configurable Thresholds)

**New homebase config fields:**
```c
typedef struct {
    // ... existing fields ...
    uint16_t outdoor_lux_min;   // Default 500, Alaska winter: 200
    uint16_t indoor_lux_min;    // Default 50, configurable
    uint8_t  daylight_start;    // Default 6, Alaska winter: 10
    uint8_t  daylight_end;      // Default 18, Alaska winter: 16
} homebase_config_t;
```

**Modified EM calculation in `phase_engine.c`:**
```c
// Replace hardcoded line 114 with:
uint16_t expected_light;
if (hour >= config->daylight_start && hour < config->daylight_end) {
    expected_light = config->outdoor_lux_min;
} else {
    expected_light = config->indoor_lux_min;
}
```

**Builder UI additions:**
```html
<!-- Light Calibration section -->
<label>Outdoor light threshold (lux)
  <input type="number" id="outdoor_lux_min" value="500" min="50" max="2000">
</label>
<label>Indoor light threshold (lux)
  <input type="number" id="indoor_lux_min" value="50" min="5" max="500">
</label>
<label>Daylight hours
  <input type="number" id="daylight_start" value="6" min="0" max="23"> to
  <input type="number" id="daylight_end" value="18" min="0" max="23">
</label>
```

**Phase 5 Option B (Auto-Calibration)** deferred — would track 7-day lux distribution and compute percentile thresholds automatically. Requires ~200 B additional RAM for histogram buckets.

### Flash: ~300 bytes | RAM: +8 bytes (config storage)

---

## 3. WK Activity Baseline (7-Day Rolling Average)

### Problem

Fixed "active" threshold treats everyone the same. A sedentary user getting 30 movements/hour is very active for them; a runner hitting 30 is resting.

### Architecture

```
wk_baseline_t (stored in sleep_data.h alongside sleep tracking):
├─ daily_totals[7]     // 7 × uint16_t = 14 B (movements per day)
├─ day_index            // uint8_t, circular buffer pointer
├─ baseline_per_hour    // uint8_t, computed: avg(daily_totals) / active_hours
```

**Baseline computation (integer math):**
```c
// Called once per day at midnight
static void update_wk_baseline(wk_baseline_t *wk) {
    uint32_t sum = 0;
    uint8_t valid_days = 0;
    for (uint8_t i = 0; i < 7; i++) {
        if (wk->daily_totals[i] > 0) {
            sum += wk->daily_totals[i];
            valid_days++;
        }
    }
    if (valid_days > 0) {
        // Baseline = average hourly movement
        // Assume 16 active hours per day
        wk->baseline_per_hour = (uint8_t)(sum / (valid_days * 16));
    } else {
        wk->baseline_per_hour = 30;  // sensible default
    }
}
```

**Active threshold derivation:**
```c
// "Active" = 1.5× baseline (integer: baseline + baseline/2)
uint8_t active_threshold = wk->baseline_per_hour + (wk->baseline_per_hour >> 1);

// "Very active" = 2.5× baseline
uint8_t very_active_threshold = wk->baseline_per_hour * 2 + (wk->baseline_per_hour >> 1);
```

**Integration with WK metric scoring:**
- Replace fixed thresholds in WK computation
- WK score becomes relative to YOUR baseline
- Stored alongside Phase 4E sleep data (same persistent storage mechanism)

### Flash: ~350 bytes | RAM: +16 bytes

---

## 4. Zone Transition Hysteresis Tuning

### Current State

`playlist.c` line 32: `#define ZONE_HYSTERESIS_COUNT 3`

With 15-minute update intervals, this means 45 minutes before zone change. May be appropriate, but should be configurable for dogfooding tuning.

### Change

```c
// Replace compile-time constant with config value
// playlist.c line 141:
if (state->consecutive_count >= config->hysteresis_count) {
```

**homebase_config_t addition:**
```c
uint8_t hysteresis_count;  // 3-10, default 3
```

**Builder UI:** Simple number input, range 3-10, with tooltip explaining "Number of consecutive readings (× 15 min) before zone changes."

**Add to TUNING_PARAMETERS.md:**
| Parameter | Default | Range | Effect |
|-----------|---------|-------|--------|
| hysteresis_count | 3 | 3-10 | Higher = more stable zones, slower response |

### Flash: ~50 bytes | RAM: +1 byte

---

## 5. Builder UI Enhancements

### Zone Timeline Visualization

Replace text-only preview with a styled visual timeline:

```
[Emergence ██████|Momentum ██████|Active ████████|Descent ██████]
 0          25       50          75              100
                        ▲ You are here (score: 52)
```

**Implementation (builder/index.html):**
```html
<div class="zone-timeline">
  <div class="zone emergence" style="width:25%">Emergence</div>
  <div class="zone momentum" style="width:25%">Momentum</div>
  <div class="zone active"   style="width:25%">Active</div>
  <div class="zone descent"  style="width:25%">Descent</div>
  <div class="marker" id="phase-marker"></div>
</div>
```

**CSS color scheme:**
- Emergence: `#4a90d9` (cool blue, morning calm)
- Momentum: `#f5a623` (warm amber, building energy)
- Active: `#d0021b` (red, peak)
- Descent: `#7b68ee` (purple, winding down)

**Interactive:** Slider input updates marker position in real-time, showing which zone the score maps to with current threshold configuration.

### Flash/RAM: 0 (JavaScript/CSS only, runs in browser)

---

## Implementation Phases

### PR 1: Active Hours Enforcement (Priority: Critical)
- `is_sleep_window()` in playlist.c
- `sustained_active_minutes` tracking
- Sleep mode lock during sleep window
- Wire existing builder active_hours config to firmware
- **Tests:** Unit test sleep window wrap-around, all-nighter override

### PR 2: EM Light Calibration (Priority: High)
- Add configurable lux thresholds to homebase config
- Replace hardcoded values in phase_engine.c line 114
- Add daylight_start/daylight_end to config
- Builder UI fields for light thresholds
- **Tests:** Verify EM score with Alaska-winter values (200 lux midday)

### PR 3: WK Baseline + Hysteresis (Priority: High)
- 7-day rolling average in sleep_data.h
- Daily total accumulation + midnight rollover
- Configurable hysteresis_count
- Builder UI for hysteresis
- **Tests:** Baseline computation with various activity levels, threshold derivation

### PR 4: Builder UI Timeline (Priority: Low)
- Zone timeline visualization
- Interactive slider preview
- Color-coded zones
- Can ship independently of firmware changes

---

## Testing Approach

### Unit Tests
```
test_sleep_window_normal      // 22-6: hour 23 → true, hour 12 → false
test_sleep_window_no_wrap     // 1-5: hour 3 → true, hour 6 → false
test_allnighter_override      // 30+ min activity in sleep window → normal zones
test_em_alaska_winter         // 200 lux midday with threshold 200 → good EM score
test_em_default_thresholds    // Backward compat: 500 lux = outdoor
test_wk_baseline_cold_start   // No history → default 30
test_wk_baseline_sedentary    // Low activity → low threshold
test_wk_baseline_active       // High activity → high threshold
test_hysteresis_configurable  // Count 5 → requires 5 consecutive readings
```

### Dogfooding Validation
1. **Sleep mode:** Wear overnight, confirm no zone escalation from bathroom trips
2. **All-nighter:** Stay up past midnight doing focused work, confirm zones unlock after 30 min
3. **EM calibration:** Set Alaska winter thresholds, verify EM score tracks "outdoor feel" not absolute lux
4. **WK baseline:** Wear for 7 days, check if "active" threshold matches subjective feel
5. **Hysteresis:** Try values 3, 5, 7 — find sweet spot for stable but responsive zones

---

## Integration with Phase 4E Telemetry

Phase 4E adds telemetry logging. Phase 4F features should emit telemetry events:

| Event | Data | Purpose |
|-------|------|---------|
| `SLEEP_LOCK` | hour, movement_level | When sleep window locks zones |
| `ALLNIGHTER` | sustained_minutes | When all-nighter override triggers |
| `EM_CAL` | lux_reading, threshold, em_score | Light calibration effectiveness |
| `WK_BASELINE` | baseline_per_hour, active_threshold | Daily baseline updates |
| `ZONE_HYST` | from_zone, to_zone, count_required | Zone transitions with hysteresis detail |

These feed back into tuning decisions during dogfooding.

---

## Constraints Checklist

- [x] Integer-only math (no FPU) — all computations use uint8/uint16/uint32
- [x] <2 KB flash total — estimated 1,100 bytes
- [x] All code `#ifdef PHASE_ENGINE_ENABLED`
- [x] Compatible with Phase 4E sleep improvements
- [x] Configurable via builder UI (no recompilation for tuning)
- [x] CR2016 power budget maintained (no new wake sources, piggyback on existing 15-min updates)

---

## File Touch Map

| File | Changes |
|------|---------|
| `lib/phase/playlist.c` | Sleep window check, sustained activity, configurable hysteresis |
| `lib/phase/playlist.h` | New fields in playlist_state_t |
| `lib/phase/phase_engine.c` | Configurable lux thresholds (line 114) |
| `lib/phase/sleep_data.h` | WK baseline struct (7-day rolling) |
| `movement/movement.c` | Route to sleep_face during sleep window |
| `builder/index.html` | Light calibration fields, hysteresis input, zone timeline |
| `docs/TUNING_PARAMETERS.md` | Document all new configurable values |
