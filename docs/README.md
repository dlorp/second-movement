# Sensor Watch Pro Documentation

This directory contains research, architecture, and feature documentation for the Sensor Watch Pro Active Hours + Sleep Tracking project.

## Directory Structure

### `/research`
Academic and industry research backing our design decisions:
- `sleep_wearables_research_summary.md` - Commercial sleep tracker research (Oura, Fitbit, Apple, WHOOP)
- `sleep_algorithm_validation.md` - (Coming soon) Validation of chosen sleep tracking algorithm
- `circadian_score_validation.md` - (Coming soon) Validation of Circadian Score design

### `/architecture`
System architecture and protocol specifications:
- `sensor-watch-comms-architecture.md` - Unified comms system (optical RX + acoustic TX)
- `sensor-watch-active-hours-spec.md` - Active Hours system specification
- `circadian-score-algorithm-v2.md` - Circadian Score (0-100) algorithm - 75% evidence-based
- `sleep-algorithm-decision.md` - Sleep tracking algorithm choice (Cole-Kripke + Light Enhancement)

### `/features`
Feature-specific documentation:
- `TAP_TO_WAKE.md` - Tap-to-wake implementation
- `TAP_TO_WAKE_SLEEP_MODE_ANALYSIS.md` - Technical analysis
- `STREAM1_*.md` - Active Hours Stream 1 (core sleep logic)
- `STREAM2_*.md` - Active Hours Stream 2 (settings UI)
- `STREAM3_*.md` - Active Hours Stream 3 (smart alarm)
- `STREAM4_*.md` - Active Hours Stream 4 (sleep tracking)
- `SLEEP_TRACKER_INTEGRATION.md` - Cole-Kripke + Light implementation guide

## Project Overview

**Goal:** Transform Sensor Watch Pro into a circadian health tracker with:
- Active Hours sleep detection (0400-2300 default)
- Smart alarm (window-based, light sleep wake)
- Circadian Score (0-100, timing + light + duration + efficiency)
- Sleep Score (0-100, single-night quality)
- Unified comms (optical RX for config, acoustic TX for data export)
- HealthKit integration (sleep analysis, light exposure, activity proxy)

**Hardware:**
- LIS2DW12 accelerometer (6D orientation detection)
- Light sensor (ADC, 0-255 log-scaled lux)
- NO HRV, heart rate, SpO2, skin temp

**Constraints:**
- Embedded C implementation
- <20µA power budget during sleep
- No cloud dependencies
- Local-first data ownership
