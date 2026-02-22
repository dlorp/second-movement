# Phase 4E: Sleep Tracking Improvements + Telemetry Expansion

**Status:** Planning  
**Date:** 2026-02-22  
**Dependencies:** Phase Engine, Cole-Kripke sleep algorithm (Phase 1-2 complete), comms_face  

---

## Overview

Phase 4E adds three sleep quality improvements and an expanded telemetry export. The sleep improvements refine the existing Cole-Kripke sleep/wake binary into richer sleep quality data. The telemetry expansion captures per-hour operational metrics for offline analysis and tuning.

**Primary goal:** Better sleep quality insights  
**Secondary goal:** Operational telemetry for Phase Engine tuning  

---

## 1. Movement Frequency Scoring

**Purpose:** Classify sleep depth beyond binary sleep/wake.

### 4-Level Sleep States

| State | Activity Count/Epoch | Description |
|-------|---------------------|-------------|
| DEEP_SLEEP (0) | 0-1 interrupts | Near-zero movement |
| LIGHT_SLEEP (1) | 2-5 interrupts | Occasional twitches |
| RESTLESS (2) | 6-15 interrupts | Frequent repositioning |
| WAKE (3) | 16+ interrupts | Active movement |

### Data Structure

```c
// Per 30-second epoch, 2 bits for sleep state
// 8 hours = 960 epochs = 240 bytes (packed 4 per byte)
typedef struct {
    uint8_t state_buffer[240];   // 4 epochs per byte, 2 bits each
    uint8_t deep_pct;            // % time in deep sleep (0-100)
    uint8_t light_pct;           // % time in light sleep (0-100)
    uint8_t restless_pct;        // % time restless (0-100)
} sleep_state_data_t;
```

### Thresholds

Derived from Cole-Kripke activity count distribution. Tunable via `sleep_state_thresholds[3]` array stored in settings.

---

## 2. Wake Event Detection

**Purpose:** Count and characterize awakenings during sleep.

### Detection Logic

```c
// Wake event = WAKE state sustained for >= WAKE_MIN_EPOCHS consecutive epochs
#define WAKE_MIN_EPOCHS 10  // 5 minutes (10 × 30s)

typedef struct {
    uint8_t wake_count;          // Number of wake events (0-255)
    uint16_t total_wake_min;     // Total minutes awake during sleep window
    uint8_t longest_wake_min;    // Longest single wake bout (minutes, capped 255)
    uint8_t wake_times[16];      // Offset from sleep onset in 15-min blocks (up to 16 events)
} wake_event_data_t;             // 20 bytes
```

### Rules
- Brief arousals (<5 min) don't count as wake events but do affect restlessness
- Wake events reset the "sustained sleep" counter for sleep efficiency
- First 20 minutes after sleep onset excluded (normal sleep latency)

---

## 3. Restlessness Index

**Purpose:** Single 0-100 sleep quality score combining all signals.

### Formula (integer-only)

```c
// All intermediate values scaled ×100 to avoid floats
uint8_t calc_restlessness(sleep_state_data_t *states, wake_event_data_t *wakes) {
    // Movement component (0-50): % of epochs in RESTLESS or WAKE state
    uint8_t move_score = (states->restless_pct + (100 - states->deep_pct - states->light_pct)) / 2;
    if (move_score > 50) move_score = 50;

    // Wake component (0-30): scaled by wake count
    uint8_t wake_score = wakes->wake_count * 5;
    if (wake_score > 30) wake_score = 30;

    // Duration component (0-20): longest wake bout contribution
    uint8_t dur_score = wakes->longest_wake_min;
    if (dur_score > 20) dur_score = 20;

    return move_score + wake_score + dur_score;  // 0 = perfect, 100 = terrible
}
```

### Export

Restlessness index stored as single byte in daily summary. Trend stored in 7-day circular buffer (7 bytes) for weekly comparison.

---

## 4. Telemetry Expansion

**Purpose:** Capture per-hour operational metrics for offline Phase Engine tuning and validation.

This is a **secondary enhancement** — sleep improvements (sections 1-3) are the primary deliverable. Telemetry piggybacks on the comms_face export with minimal additional cost.

### 4.1 Per-Hour Telemetry Fields

24 samples per day (one per hour), each containing:

| Field | Size | Encoding | Range |
|-------|------|----------|-------|
| Zone transition flag | 1 bit | 1 = zone changed this hour | 0-1 |
| Dominant metric ID | 2 bits | 0=SD, 1=EM, 2=WK, 3=Comfort | 0-3 |
| Anomaly flag | 1 bit | 1 = anomaly chime fired this hour | 0-1 |
| *(padding)* | 4 bits | Reserved | — |
| Light exposure duration | 1 byte | Minutes of light exposure (0-60) | 0-60 |
| Movement event count | 1 byte | Total LIS2DW12 interrupts this hour (capped 255) | 0-255 |
| Battery voltage | 1 byte | Scaled: `(V - 2.0) / 2.2 × 255` | 2.0-4.2V → 0-255 |
| Metric confidence: SD | 1 byte | Hour-to-hour stability (0-100) | 0-100 |
| Metric confidence: EM | 1 byte | Hour-to-hour stability (0-100) | 0-100 |
| Metric confidence: WK | 1 byte | Hour-to-hour stability (0-100) | 0-100 |
| Metric confidence: Comfort | 1 byte | Hour-to-hour stability (0-100) | 0-100 |

**Per-sample size:** 1 + 7 = **8 bytes/hour**  
**Daily total:** 8 × 24 = **192 bytes/day**

### 4.2 Data Structure

```c
// Bit-packed flags byte
// Bits [7:4] reserved
// Bit  [3]   anomaly_flag
// Bits [2:1] dominant_metric_id (0=SD, 1=EM, 2=WK, 3=Comfort)
// Bit  [0]   zone_transition

typedef struct {
    uint8_t flags;               // zone_transition:1, dominant_metric:2, anomaly:1, reserved:4
    uint8_t light_exposure_min;  // 0-60 minutes
    uint8_t movement_count;      // LIS2DW12 interrupt count (capped 255)
    uint8_t battery_voltage;     // scaled 2.0-4.2V → 0-255
    uint8_t confidence_sd;       // 0-100
    uint8_t confidence_em;       // 0-100
    uint8_t confidence_wk;       // 0-100
    uint8_t confidence_comfort;  // 0-100
} hourly_telemetry_t;           // 8 bytes

typedef struct {
    hourly_telemetry_t hours[24]; // 192 bytes
} daily_telemetry_t;
```

### 4.3 Accumulation Logic

```c
// Called once per hour from Phase Engine tick
void telemetry_accumulate_hour(hourly_telemetry_t *sample, uint8_t hour_idx) {
    // Zone transition: compare current zone to previous hour's zone
    sample->flags = 0;
    if (current_zone != prev_hour_zone)
        sample->flags |= 0x01;  // zone_transition bit

    // Dominant metric: which metric has highest absolute delta from midpoint
    sample->flags |= (get_dominant_metric() & 0x03) << 1;

    // Anomaly flag
    if (anomaly_fired_this_hour)
        sample->flags |= 0x08;

    // Direct readings
    sample->light_exposure_min = light_minutes_this_hour;  // accumulated in light ISR
    sample->movement_count = MIN(motion_interrupts_this_hour, 255);
    sample->battery_voltage = (uint8_t)(((read_battery_mv() - 2000) * 255UL) / 2200);

    // Confidence: abs difference from previous hour, inverted
    // High stability (small delta) = high confidence
    sample->confidence_sd = 100 - MIN(abs(sd_now - sd_prev_hour), 100);
    sample->confidence_em = 100 - MIN(abs(em_now - em_prev_hour), 100);
    sample->confidence_wk = 100 - MIN(abs(wk_now - wk_prev_hour), 100);
    sample->confidence_comfort = 100 - MIN(abs(comfort_now - comfort_prev_hour), 100);
}
```

### 4.4 Diagnostic Value

| Field | What It Reveals |
|-------|----------------|
| Zone transitions | Zone stability; detects rapid oscillation between zones |
| Dominant metric | Which metric drives zone changes; identifies conflicting metrics |
| Light exposure | Validates EM metric accuracy; detects "all day indoors" patterns |
| Movement count | Cross-checks WK metric; validates Cole-Kripke activity counts |
| Battery voltage | Tracks Phase Engine power drain over time; detects anomalous consumption |
| Anomaly log | Tunes anomaly detection thresholds; tracks false positive rate |
| Confidence scores | Identifies which metrics are unstable and need tuning first |

---

## 5. Updated comms_face Export Format

### Previous Format (Phase 4D)
- ~258 bytes: daily summary + zone history

### Phase 4E Format

| Section | Size (bytes) | Contents |
|---------|-------------|----------|
| Header | 2 | Magic + version byte |
| Daily summary | 16 | Sleep onset/offset, duration, efficiency, score |
| Sleep states summary | 4 | deep/light/restless/wake percentages |
| Wake events | 20 | Count, total wake min, longest bout, timestamps |
| Restlessness index | 8 | Today + 7-day history |
| Zone history (24h) | 24 | Hourly zone IDs |
| Telemetry (24h) | 192 | Hourly telemetry samples |
| Existing Phase Engine data | ~180 | Metric snapshots, settings |
| Checksum | 2 | CRC-16 |
| **Total** | **~448 bytes** | |

### TX Time Estimate
- Piezo encoding: ~3.5 bytes/second (conservative)
- 448 bytes ÷ 3.5 = **~128 seconds (~2 min 8 sec)**
- Within acceptable ~2 min TX window ✓

### Export Trigger
- Same as existing comms_face: long-press ALARM button
- Data covers previous 24 hours (midnight to midnight)
- Telemetry buffer overwrites daily at midnight

---

## 6. Resource Impact

### Flash (Code)

| Component | Estimate |
|-----------|----------|
| Sleep state classification | ~300 bytes |
| Wake event detection | ~250 bytes |
| Restlessness calculation | ~150 bytes |
| Telemetry accumulation | ~200 bytes |
| Updated comms_face export | ~200 bytes |
| **Total flash addition** | **~1,100 bytes** |

Well under 2 KB flash constraint. ✓

### RAM

| Component | Estimate |
|-----------|----------|
| sleep_state_data_t | 243 bytes |
| wake_event_data_t | 20 bytes |
| Restlessness + 7-day buffer | 8 bytes |
| daily_telemetry_t | 192 bytes |
| Hourly accumulators (counters) | 8 bytes |
| **Total RAM addition** | **~471 bytes** |

### Power
- Telemetry accumulation: once per hour, <1ms computation → negligible
- Battery ADC read: single sample per hour → negligible
- No additional sensor power; all data derived from existing interrupts and readings

---

## 7. Implementation Priority

| Priority | Component | Complexity |
|----------|-----------|-----------|
| P0 | Movement frequency scoring (4-level states) | Easy |
| P0 | Wake event detection | Easy |
| P1 | Restlessness index | Easy |
| P2 | Telemetry accumulation | Easy |
| P2 | Updated comms_face export | Medium |

Sleep improvements (P0/P1) ship first. Telemetry (P2) follows once sleep data is validated.

---

## Constraints Compliance

- ✅ Integer-only arithmetic (no floats)
- ✅ < 2 KB flash addition (~1.1 KB)
- ✅ Fits comms_face TX window (~2 min)
- ✅ No new sensor hardware required
- ✅ All data derived from existing Phase Engine state + LIS2DW12 + light sensor + battery ADC
