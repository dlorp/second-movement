# Phase 3 Tuning Parameters
**Version:** 1.0 (Initial dogfooding baseline)  
**Last updated:** 2026-02-20  
**Status:** Pre-hardware testing (recommendations based on research)

---

## Overview

This document provides **specific numeric values** for Phase 3 behavioral tuning, derived from:
- Sensor Watch hardware capabilities (SAM L22, LIS2DW12)
- Competitive analysis (Oura, Whoop, Garmin, Fitbit)
- Academic research (circadian wearables, glanceable UX)
- Community feedback (r/sensorwatch, GitHub issues)

**Philosophy:**  
Phase 3 = agency. User checks watch before deciding. Watch whispers context without commanding.

---

## 1. Sensor Sampling Rates

### Motion Detection (LIS2DW12 Accelerometer)

```c
// Recommended configuration for Phase 3
#define ACCEL_MODE          LIS2DW12_MODE_LOW_POWER_1  // 12-bit resolution
#define ACCEL_ODR           LIS2DW12_ODR_1_6HZ         // 1.6 Hz sampling
#define ACCEL_POWER_MODE    LIS2DW12_STATIONARY_DETECT // No mode change on wake/sleep

// Wake-on-motion thresholds
#define MOTION_THRESHOLD_MG     62    // mg (0-2000 range)
#define MOTION_DURATION_MS      160   // Minimum motion duration to trigger wake
#define INACTIVITY_TIMEOUT_SEC  900   // 15 min (to match phase inertia)

// Power budget: 2.75 µA @ 3.3V (measured)
```

**Rationale:**
- **1.6 Hz** = lowest power ODR (2.75 µA total)
- **Stationary detect** = no ODR changes (stable power profile)
- **62 mg threshold** = proven effective in Sensor Watch sleep tracking
- **15 min inactivity** = matches desired phase stability (prevents jitter)

**Test case:**  
Walk for 2 min → motion flag HIGH. Sit for 15 min → motion flag LOW. Walk again → flag HIGH within 1 sec.

---

### Light Sensing (Onboard Lux Sensor)

```c
// Recommended configuration
#define LUX_SAMPLE_INTERVAL_SEC   60    // Sample once per phase update
#define LUX_AVERAGE_WINDOW_SEC    300   // 5-min rolling average
#define LUX_THRESHOLD_INDOOR      200   // lux (approximate)
#define LUX_THRESHOLD_OUTDOOR     2000  // lux (approximate)

// Power budget: <0.1 µA (sampled 1/min)
```

**Rationale:**
- **1/min sampling** = negligible power cost (<0.1 µA)
- **5-min average** = prevents jitter from clouds/shadows
- **Thresholds** = rough heuristics for Emergence (low light) vs Active (bright)

**Test case:**  
Move from indoors (100 lux) to outdoors (5000 lux). Lux average should transition over 5 min (not instantly).

---

## 2. Phase Engine Update Cadence

### Background Phase Computation

```c
// Phase update frequency (invisible to user)
#define PHASE_UPDATE_INTERVAL_SEC  60   // Every 1 minute

// Metric computation intervals
#define METRIC_SD_UPDATE_SEC       60   // Somatic Dissonance (motion variance)
#define METRIC_COMFORT_UPDATE_SEC  60   // Comfort (light context)
#define METRIC_EM_UPDATE_SEC       60   // Emotional Momentum (trend analysis)
#define METRIC_WK_UPDATE_SEC       60   // Work (motion intensity)
#define METRIC_ENERGY_UPDATE_SEC   60   // Energy (phase alignment)

// Power budget: 0.2 µA (1/min wake + compute)
```

**Rationale:**
- **60 sec updates** = responsive enough for behavioral decisions
- **All metrics same cadence** = simplifies implementation
- **RTC alarm wake** = no polling overhead (interrupt-driven)

**Test case:**  
Trigger high motion event. Within 60 sec, SD metric should reflect change (not instant, not >2 min).

---

### Display Update Strategy

```c
// When to update the display (user-visible)
#define FACE_UPDATE_ON_ZONE_CHANGE   true   // Update when zone shifts
#define FACE_UPDATE_ON_TICK          false  // Don't update every second
#define FACE_UPDATE_ON_ALARM         true   // User-configured alarms

// Zone change debouncing (prevent jitter)
#define ZONE_CHANGE_DEBOUNCE_SEC     300    // 5 min minimum in zone before switching
#define ZONE_CHANGE_HYSTERESIS       0.1    // 10% buffer to prevent oscillation

// Power budget: ~1 µA (display updates amortized)
```

**Rationale:**
- **Zone-driven updates** = intelligent surfacing (not time-driven spam)
- **5-min debounce** = prevents "Active → Momentum → Active" jitter
- **Hysteresis** = zone boundaries have ±10% buffer (prevents edge oscillation)

**Test case:**  
Rapid activity burst (2 min). Zone should NOT change immediately. Sustained activity (10 min) → zone shifts within 5 min of threshold crossing.

---

## 3. Playlist Controller Parameters

### Face Rotation Frequency

```c
// Playlist rotation strategy
#define PLAYLIST_MODE_PASSIVE       0   // Clock dominates (default)
#define PLAYLIST_MODE_ACTIVE        1   // Zone faces rotate

// Passive mode (default)
#define CLOCK_FACE_DOMINANCE_PCT    70  // 70% of time showing clock
#define METRIC_DWELL_TIME_SEC       10  // 10 sec per metric face (when rotating)
#define ROTATION_TRIGGER            ZONE_CHANGE_ONLY  // Not time-driven

// Active mode (user-initiated, future feature)
#define METRIC_ROTATION_INTERVAL_SEC 15  // 15 sec per face (manual mode)
```

**Rationale:**
- **70% clock** = respects primary function (time)
- **10 sec dwell** = long enough to read, short enough to cycle
- **Zone-triggered** = rotation happens when conditions change (not clock spam)

**Test case:**  
Wear watch for 1 hour sitting. Clock should show ~42 min (70%), zone faces ~18 min (30%, cycling). If no zone changes, rotation frequency should be minimal.

---

### Zone Face Priority

```c
// Which face to show in each zone (priority order)
typedef struct {
    zone_t zone;
    face_t primary_face;
    face_t secondary_face;
    uint8_t rotation_enabled;
} zone_face_mapping_t;

// Recommended mappings (Phase 3 default)
zone_face_mapping_t zone_faces[4] = {
    {ZONE_EMERGENCE,  FACE_SD_COMFORT,  FACE_CLOCK, 1}, // Low motion + comfort focus
    {ZONE_MOMENTUM,   FACE_EM_ENERGY,   FACE_CLOCK, 1}, // Building energy
    {ZONE_ACTIVE,     FACE_WK_ENERGY,   FACE_CLOCK, 1}, // Peak performance
    {ZONE_DESCENT,    FACE_SD_COMFORT,  FACE_CLOCK, 1}, // Recovery mode
};

// Dwell time per face before returning to clock
#define PRIMARY_FACE_DWELL_SEC   10
#define SECONDARY_FACE_DWELL_SEC 5  // Clock shows briefly before primary returns
```

**Rationale:**
- **Primary face** = most relevant metric for current zone
- **Secondary = clock** = always return to time (respect primary function)
- **Rotation within zone** = cycles between primary and clock only (not all faces)

**Test case:**  
Enter Momentum zone. Watch should show EM/Energy face for 10 sec, clock for 5 sec, repeat. If zone changes to Active, face switches to WK/Energy.

---

## 4. Metric Computation Parameters

### Somatic Dissonance (SD)

```c
// Motion variance analysis
#define SD_VARIANCE_WINDOW_SEC      300   // 5-min window for variance calc
#define SD_HIGH_THRESHOLD           1.5   // Normalized variance (0-2 scale)
#define SD_LOW_THRESHOLD            0.3

// Scoring
#define SD_SCORE_MIN                0     // Aligned (low variance)
#define SD_SCORE_MAX                100   // Dissonant (high variance)
```

**Rationale:**
- **5-min window** = captures recent motion pattern (not too reactive)
- **Thresholds** = empirical (to be tuned during dogfooding)

**Test case:**  
Walk steadily for 10 min → SD low (~20). Walk-stop-walk-stop for 10 min → SD high (~80).

---

### Comfort

```c
// Light-based comfort heuristic
#define COMFORT_LUX_OPTIMAL_MIN     200   // lux
#define COMFORT_LUX_OPTIMAL_MAX     1000  // lux
#define COMFORT_TEMP_FACTOR         0     // Placeholder (no temp sensor yet)

// Scoring
#define COMFORT_SCORE_MIN           0     // Uncomfortable
#define COMFORT_SCORE_MAX           100   // Optimal
```

**Rationale:**
- **200-1000 lux** = comfortable indoor lighting (rough heuristic)
- **Outside range** = penalize comfort score (too dim or too bright)

**Test case:**  
Indoors (500 lux) → Comfort ~90. Outdoors (10,000 lux) → Comfort ~60 (bright = less comfortable for rest). Dark room (5 lux) → Comfort ~40.

---

### Emotional Momentum (EM)

```c
// Trend analysis (motion + lux deltas)
#define EM_DELTA_WINDOW_SEC         600   // 10-min trend window
#define EM_POSITIVE_THRESHOLD       0.2   // Increasing motion/light = building
#define EM_NEGATIVE_THRESHOLD      -0.2   // Decreasing = descending

// Scoring
#define EM_SCORE_MIN                0     // Descending
#define EM_SCORE_MAX                100   // Building momentum
```

**Rationale:**
- **10-min trend** = captures direction of change (not instant delta)
- **Thresholds** = tune during dogfooding (sign of acceleration)

**Test case:**  
Gradually increase activity over 15 min → EM rises (~70+). Plateau → EM stabilizes (~50). Decrease activity → EM falls (~30).

---

### Work (WK)

```c
// Motion intensity (accelerometer magnitude)
#define WK_INTENSITY_WINDOW_SEC     300   // 5-min average
#define WK_HIGH_THRESHOLD_MG        200   // High motion (walking)
#define WK_LOW_THRESHOLD_MG         50    // Low motion (sitting)

// Scoring
#define WK_SCORE_MIN                0     // Resting
#define WK_SCORE_MAX                100   // Peak exertion
```

**Rationale:**
- **5-min average** = smooths brief bursts (e.g., standing up)
- **Thresholds** = empirical (to be calibrated vs actual activity)

**Test case:**  
Sitting → WK ~10. Walking → WK ~60. Running → WK ~90.

---

### Energy

```c
// Phase alignment score (how well are you timing activity?)
#define ENERGY_OPTIMAL_ZONE         ZONE_ACTIVE     // Best time for exertion
#define ENERGY_SUBOPTIMAL_ZONE      ZONE_MOMENTUM   // Building toward active
#define ENERGY_POOR_ZONE            ZONE_DESCENT    // Recovery, avoid high WK

// Scoring
#define ENERGY_SCORE_MIN            0     // Misaligned (high WK during Descent)
#define ENERGY_SCORE_MAX            100   // Aligned (high WK during Active)
```

**Rationale:**
- **Zone-dependent** = Energy penalizes activity during Descent, rewards during Active
- **Meta-metric** = reflects how well user is listening to phase

**Test case:**  
High motion during Active zone → Energy ~90. High motion during Descent → Energy ~30 (misalignment penalty).

---

## 5. Power Budget Allocation

### Total Power Target: <3 µA

| Component | Budget (µA) | Configuration | Notes |
|-----------|-------------|---------------|-------|
| **RTC baseline** | 0.5 | Always-on | Passive clock |
| **Accelerometer** | 0.6 | 1.6 Hz stationary detect | LIS2DW12 LPMode1 |
| **Phase computation** | 0.2 | 1/min wake | 10 ms compute time |
| **Display updates** | 1.0 | Zone-driven | Amortized over 5-min dwell |
| **Lux sensor** | 0.1 | 1/min poll | Single ADC read |
| **Misc overhead** | 0.1 | BKUP writes, etc | Negligible |
| **Total** | **2.5 µA** | | **Target: >1 year battery** |

**Battery life calculation:**
- CR2016: 90 mAh = 90,000 µAh
- 90,000 µAh / 2.5 µA = **36,000 hours = 1,500 days = 4.1 years**

**Realistic target (accounting for inefficiencies):**  
**>1 year battery life** with Phase Engine active.

---

## 6. Test Cases for Validation

### Behavioral Responsiveness

**Test 1: Morning routine**
- Wake up (7 AM) → Emergence zone (low motion overnight)
- Get out of bed → motion spike → SD increases
- Within 5 min → zone should remain Emergence (debounce working)
- 15 min later (showering, breakfast) → transition to Momentum

**Expected:** Zone changes ~15-20 min after sustained activity change (not instant).

**Test 2: Desk work burst**
- Sitting at desk (2 PM) → Momentum zone
- Stand up for 5 min meeting → brief motion spike
- Sit back down → motion returns to baseline
- Zone should NOT change to Active (brief burst filtered out)

**Expected:** Zone remains Momentum (5-min debounce prevents jitter).

**Test 3: Evening wind-down**
- Active zone (6 PM post-workout)
- Sit on couch (7 PM) → motion drops
- Watch TV for 30 min → sustained low motion
- Within 5-10 min of sitting → transition to Descent

**Expected:** Zone shifts to Descent after debounce period.

---

### Power Consumption Validation

**Test 4: 24-hour power profile**
- Wear watch for 24 hours
- Measure current draw every 1 sec (via multimeter or power profiler)
- Expected average: **2.5 µA ± 0.5 µA**

**Red flags:**
- Spikes >10 µA (indicates sensor misconfiguration)
- Baseline >3 µA (power budget exceeded)
- Accelerometer draw >3 µA (should be 0.6 µA)

---

### Display Update Frequency

**Test 5: Hour-long passive observation**
- Sit still for 1 hour (no zone changes)
- Count face updates
- Expected: **~6 updates/hour** (clock ↔ metric rotation at 10-sec dwell)
- Red flag: >12 updates/hour (too frequent, jittery)

**Test 6: Active hour (zone changes)**
- Varied activity (sit, walk, run, rest) over 1 hour
- Count zone face changes
- Expected: **2-4 zone changes/hour** (depending on activity variance)
- Red flag: >6 zone changes/hour (insufficient debouncing)

---

## 7. Tuning Knobs (For Dogfooding Iteration)

### Easy Adjustments (change and test immediately)

```c
// In phase_config.h (future: make these runtime-configurable)

// Sensitivity knobs
#define SENSITIVITY_MOTION          1.0   // Multiplier for SD threshold (0.5-2.0)
#define SENSITIVITY_LIGHT           1.0   // Multiplier for Comfort lux range (0.5-2.0)
#define SENSITIVITY_ZONE_CHANGE     1.0   // Multiplier for zone thresholds (0.8-1.2)

// Temporal knobs
#define DEBOUNCE_ZONE_CHANGE_SEC    300   // 5 min (test range: 180-600)
#define DWELL_METRIC_FACE_SEC       10    // 10 sec (test range: 5-20)
#define CLOCK_DOMINANCE_PCT         70    // 70% (test range: 60-80)

// Power knobs (use with caution)
#define PHASE_UPDATE_INTERVAL_SEC   60    // 1 min (test: 30-120)
#define ACCEL_ODR                   1.6   // Hz (test: 1.6, 3.125, 6.25)
```

**Tuning workflow:**
1. Start with defaults (above)
2. Wear watch for 3 days → collect subjective feedback
3. Adjust one knob at a time (e.g., increase debounce to 10 min if jittery)
4. Re-test for 3 days → iterate

---

## 8. Success Criteria (Qualitative)

After dogfooding, Phase 3 succeeds if:

✅ **User checks watch before decisions** (not after being prompted)  
✅ **Zone changes feel "inevitable, not surprising"** (phase aligns with somatic state)  
✅ **Face rotation is "invisible background, visible when needed"** (not distracting)  
✅ **Battery life >7 days** (ideally >30 days, target >1 year)  
✅ **Watch feels like "intelligent companion, not nagging coach"** (whispers context)

**Red flags:**
❌ User ignores zone faces (not providing value)  
❌ Zone changes feel random (algorithmic distrust)  
❌ Face rotation is distracting (updates too frequent)  
❌ Battery drains <7 days (power budget exceeded)  
❌ User has to "interpret" what zone means (not intuitive)

---

## 9. Calibration Baseline (Pre-Dogfooding)

### Assumed Behavioral Patterns (to validate)

| Time of Day | Expected Zone | Rationale |
|-------------|---------------|-----------|
| **6-7 AM** | Emergence | Waking up, low motion |
| **8-10 AM** | Momentum | Morning energy building |
| **11 AM-1 PM** | Active | Peak cognitive/physical window |
| **2-3 PM** | Descent | Post-lunch dip |
| **4-6 PM** | Momentum | Second wind |
| **7-9 PM** | Active | Evening activity/exercise |
| **10 PM-6 AM** | Emergence | Sleep/rest |

**Caveat:** These are **population averages** (circadian research). Individual chronotypes vary (morning larks vs night owls). Phase 3 should **learn user's pattern**, not enforce normative schedule.

---

## 10. Next Steps

1. **Implement tuning parameters** in Phase 3 codebase (chronomantic_instrument.c)
2. **Flash to hardware** (green board with LIS2DW accelerometer)
3. **Dogfood for 7 days** using calibration checklist (CALIBRATION_CHECKLIST.md)
4. **Iterate knobs** based on subjective feedback + power measurements
5. **Document lessons** (update this file with "tuned" values)

**First tuning target:** Get zone debouncing right (5-10 min feels stable vs jittery).  
**Second tuning target:** Playlist rotation frequency (does 70% clock feel right?).  
**Third tuning target:** Power budget (measure actual vs target).

---

## Appendix: Rationale Summary

**Why 1/min phase updates?**  
- Garmin updates Body Battery every 5 min (too slow for behavioral responsiveness)
- Fitbit updates heart rate every 1-5 sec (too fast, power-hungry)
- **1/min = sweet spot** (responsive for decisions, efficient for power)

**Why 5-min zone debounce?**  
- Oura/Whoop deliver scores 1x/morning (too slow for intra-day agency)
- Garmin's 5-min smoothing praised as "responsive without jitter"
- **5 min = proven UX pattern** from competitive analysis

**Why 70% clock dominance?**  
- Sensor Watch is **primarily a watch** (time is the core function)
- Fitness trackers show 1-2 complications (limited screen space)
- **70% = balance between passive time and intelligent surfacing**

**Why LIS2DW12 stationary mode?**  
- Lowest power consumption (2.75 µA total @ 1.6 Hz)
- No ODR changes = stable power profile (easier to budget)
- Proven in Sensor Watch sleep tracking (real-world validation)

---

**End of tuning parameters. Begin dogfooding with these defaults, iterate based on reality.**
