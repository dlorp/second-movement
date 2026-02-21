# Phase 3 Calibration Checklist
**Purpose:** Systematic validation of Phase 3 behavior during dogfooding  
**Target duration:** 7-14 days continuous wear  
**Status:** Pre-hardware (ready for green board testing)

---

## Overview

This checklist guides **qualitative + quantitative validation** of Phase 3 tuning parameters. Use alongside:
- **PHASE3_BASELINE_RESEARCH.md** (research foundations)
- **TUNING_PARAMETERS.md** (specific numeric values)
- **COMPETITIVE_ANALYSIS.md** (UX benchmarks)

**Calibration philosophy:**  
Measure what matters. Trust somatic state over algorithm output. Iterate knobs based on felt experience, not theory.

---

## Pre-Flight Checklist (Before Dogfooding)

### Hardware Validation

- [ ] **Green board detected** (`movement_state.has_lis2dw == true`)
- [ ] **Accelerometer responding** (motion detection triggers within 1 sec)
- [ ] **Lux sensor functional** (readings change indoors→outdoors)
- [ ] **BKUP registers persistent** (phase state survives power loss)
- [ ] **Battery installed** (CR2016, fresh, >3.0V)
- [ ] **Display updates** (zone faces render without artifacts)

### Software Validation

- [ ] **Phase engine compiles** (no warnings, <10 KB flash budget)
- [ ] **Metrics compute** (SD, Comfort, EM, WK, Energy all update)
- [ ] **Playlist controller functional** (faces rotate on zone changes)
- [ ] **Zone detection working** (manual motion test triggers Momentum→Active)
- [ ] **Debouncing enabled** (rapid motion bursts don't cause zone jitter)

### Baseline Measurements

- [ ] **Power consumption measured** (target: <3 µA average)
  - Method: Multimeter in series with battery, sample over 1 min
  - Baseline (clock only): ______ µA
  - With Phase Engine: ______ µA
  - Delta: ______ µA (should be ~2 µA)

- [ ] **Phase update latency tested**
  - Trigger motion event → time until SD metric changes: ______ sec (target: <60 sec)
  - Sustained motion → time until zone changes: ______ sec (target: 5-10 min)

---

## Daily Observation Log (7-Day Template)

### Day [___] - Date: ______

#### Morning (6-10 AM)

**Somatic state upon waking:**
- [ ] Groggy / [ ] Neutral / [ ] Alert
- Sleep quality (1-5): _____ (1=terrible, 5=excellent)

**Watch displayed zone:**
- [ ] Emergence / [ ] Momentum / [ ] Active / [ ] Descent

**Alignment check:**  
Does zone match how you feel? [ ] Yes / [ ] No  
If no, describe mismatch: ________________________________

**Morning routine activity log:**

| Time | Activity | Displayed Zone | Feels Right? | Notes |
|------|----------|----------------|--------------|-------|
| 6:30 | Waking up | Emergence | ☐ Yes ☐ No | |
| 7:00 | Shower | | ☐ Yes ☐ No | |
| 7:30 | Breakfast | | ☐ Yes ☐ No | |
| 8:00 | Commute | | ☐ Yes ☐ No | |

**Zone change observations:**
- Number of zone changes (6-10 AM): _____
- Did any feel premature/jittery? [ ] Yes / [ ] No
- If yes, when: ________________________________

#### Midday (10 AM-2 PM)

**Peak productivity window:**
- [ ] Emergence / [ ] Momentum / [ ] Active / [ ] Descent (around 11 AM-12 PM)

**Activity log:**

| Time | Activity | Displayed Zone | Feels Right? | Notes |
|------|----------|----------------|--------------|-------|
| 10:00 | Work/study | | ☐ Yes ☐ No | |
| 11:00 | | | ☐ Yes ☐ No | |
| 12:00 | Lunch | | ☐ Yes ☐ No | |
| 1:00 | | | ☐ Yes ☐ No | |

**Post-lunch dip:**
- Did zone shift to Descent around 2-3 PM? [ ] Yes / [ ] No
- If yes, did this match your energy level? [ ] Yes / [ ] No

#### Evening (6-10 PM)

**Activity log:**

| Time | Activity | Displayed Zone | Feels Right? | Notes |
|------|----------|----------------|--------------|-------|
| 6:00 | Exercise/dinner | | ☐ Yes ☐ No | |
| 7:00 | | | ☐ Yes ☐ No | |
| 8:00 | Relaxing | | ☐ Yes ☐ No | |
| 9:00 | Wind down | | ☐ Yes ☐ No | |

**Wind-down observation:**
- Did zone shift to Descent/Emergence in evening? [ ] Yes / [ ] No
- Time of shift: _____ (expected: 8-10 PM)
- Did this prompt you to rest? [ ] Yes / [ ] No / [ ] Already resting

#### Overall Day Summary

**Total zone changes today:** _____  
**Appropriate:** _____ (felt right)  
**Premature:** _____ (changed too early)  
**Delayed:** _____ (should have changed sooner)  
**Jittery:** _____ (oscillated <5 min)

**Playlist rotation behavior:**
- Estimated % time showing clock: _____%  (target: 70%)
- Estimated % time showing metrics: _____%  (target: 30%)
- Was rotation distracting? [ ] Yes / [ ] No
- Did you notice rotation happening? [ ] Often / [ ] Sometimes / [ ] Rarely

**Triggering behavior observations:**
- Did you check watch before deciding what to do? [ ] Yes / [ ] No
- If yes, how many times? _____
- Example decision influenced by zone: ________________________________

**Battery status:**
- Voltage at end of day: _____ V (use multimeter or estimate from low-battery indicator)
- Days since install: _____

---

## Red Flags to Watch For

### Zone Detection Issues

**Symptom:** Zone changes feel random/unpredictable

**Potential causes:**
- [ ] Debounce too short (increase ZONE_CHANGE_DEBOUNCE_SEC from 300→600)
- [ ] Thresholds too sensitive (decrease SENSITIVITY_ZONE_CHANGE from 1.0→0.8)
- [ ] Motion variance too noisy (increase SD_VARIANCE_WINDOW_SEC from 300→600)

**Action:** Log specific instances of "random" changes → look for pattern

---

**Symptom:** Zone stuck in one state all day

**Potential causes:**
- [ ] Debounce too long (decrease ZONE_CHANGE_DEBOUNCE_SEC from 300→180)
- [ ] Thresholds too aggressive (increase SENSITIVITY_ZONE_CHANGE from 1.0→1.2)
- [ ] Accelerometer not detecting motion (hardware issue?)

**Action:** Test with manual motion (walk for 10 min) → does zone change?

---

**Symptom:** Zone oscillates rapidly (Active→Momentum→Active within 5 min)

**Potential causes:**
- [ ] Hysteresis insufficient (increase ZONE_CHANGE_HYSTERESIS from 0.1→0.2)
- [ ] Activity pattern truly erratic (user behavior, not algorithm issue)
- [ ] Metric computation window too short (increase to 10 min)

**Action:** Increase debounce to 10 min, re-test

---

### Display Update Issues

**Symptom:** Face rotation too frequent/distracting

**Potential causes:**
- [ ] Dwell time too short (increase DWELL_METRIC_FACE_SEC from 10→15)
- [ ] Clock dominance too low (increase CLOCK_DOMINANCE_PCT from 70→80)
- [ ] Zone changes too frequent (see above)

**Action:** Reduce rotation frequency (increase dwell, increase clock %)

---

**Symptom:** Never see metric faces (always clock)

**Potential causes:**
- [ ] Clock dominance too high (decrease CLOCK_DOMINANCE_PCT from 70→60)
- [ ] Playlist rotation disabled (check config)
- [ ] No zone changes happening (zone detection broken)

**Action:** Force zone change manually, observe rotation

---

### Power Consumption Issues

**Symptom:** Battery draining faster than expected (<7 days)

**Potential causes:**
- [ ] Accelerometer in wrong mode (check ODR, power mode)
- [ ] Display updating too frequently (>1/min)
- [ ] Phase computation taking too long (>10 ms/update)
- [ ] Sensor stuck in high-power state (not entering sleep)

**Action:** Measure current draw at each component (isolate culprit)

---

**Symptom:** Power consumption higher than 3 µA average

**Breakdown test:**
1. Disable Phase Engine → measure baseline (should be ~0.5 µA)
2. Enable accelerometer only → measure (should add ~0.6 µA)
3. Enable phase computation → measure (should add ~0.2 µA)
4. Enable display updates → measure (should add ~1 µA)

**If any step exceeds budget:** Investigate that component

---

### Behavioral Alignment Issues

**Symptom:** Zone suggests "Active" but you feel exhausted

**Potential causes:**
- [ ] Metrics weighting wrong (motion high, but comfort/energy low)
- [ ] Phase algorithm missing somatic cues (sleep quality, stress)
- [ ] User chronotype mismatch (algorithm assumes morning lark, user is night owl)

**Action:** Note misalignment instances → look for patterns (time of day? activity type?)

---

**Symptom:** Zone suggests "Descent" but you feel energized

**Potential causes:**
- [ ] Lux sensor misreading (dark environment ≠ low energy)
- [ ] Motion threshold too conservative (fidgeting while sitting = high SD)
- [ ] Phase algorithm overfitting to past pattern (you slept poorly, but feel fine)

**Action:** Check metric values individually (is SD/Comfort/Energy driving zone?)

---

## Quantitative Measurements (End of Week)

### Power Budget Validation

**Test 1: 24-hour current profile**
- [ ] Completed (attach CSV data from power profiler)
- Average current draw: ______ µA (target: <3 µA)
- Peak current: ______ µA (should occur during phase computation)
- Baseline current: ______ µA (should be ~0.5 µA during sleep)

**Test 2: Battery life projection**
- Days worn: _____
- Starting voltage: _____ V
- Ending voltage: _____ V
- Voltage drop: _____ V
- Projected battery life: _____ days (formula: (Starting V - 2.0V) / (drop per day) = total days)

**Pass criteria:**  
☑ Average current <3 µA  
☑ Projected battery life >30 days (ideal: >365 days)

---

### Zone Change Frequency

**From daily logs, calculate:**
- Total zone changes over 7 days: _____
- Average per day: _____ (target: 4-8 changes/day)
- Max in 1 hour: _____ (red flag if >6)
- Min time in zone: _____ min (red flag if <5 min)

**Pass criteria:**  
☑ Average 4-8 zone changes/day  
☑ No hour with >6 changes  
☑ All zones dwelled >5 min  

---

### Display Rotation Metrics

**From daily logs, estimate:**
- % time showing clock: _____%  (target: 70%)
- % time showing metric faces: _____%  (target: 30%)
- Average metric dwell time: _____ sec (target: 10 sec)

**Pass criteria:**  
☑ Clock >60% of time  
☑ Metric dwell 5-15 sec  
☑ Rotation not distracting (subjective)

---

### Metric Accuracy Spot Checks

**Test 1: Somatic Dissonance (SD)**
- Walk steadily for 10 min → SD value: _____ (expect: <30)
- Walk-stop-walk-stop for 10 min → SD value: _____ (expect: >70)

**Test 2: Work (WK)**
- Sit still for 10 min → WK value: _____ (expect: <20)
- Brisk walk for 10 min → WK value: _____ (expect: 60-80)
- Jog for 5 min → WK value: _____ (expect: >80)

**Test 3: Comfort**
- Indoors (well-lit) → Comfort value: _____ (expect: 70-90)
- Outdoors (bright sun) → Comfort value: _____ (expect: 50-70)
- Dark room → Comfort value: _____ (expect: <50)

**Pass criteria:**  
☑ Metrics directionally correct (high motion = high SD/WK)  
☑ Values within expected ranges

---

## Subjective Validation (End of Week)

### Phase 1: Did it become a clock?  
**Goal:** Tell time effortlessly, always-on display, <30 sec to read

Rate (1-5): _____  
1 = Failed (can't read time easily)  
5 = Succeeded (as good as passive Casio)

---

### Phase 2: Did it create awareness?  
**Goal:** User notices patterns (e.g., "I'm more active in mornings")

Rate (1-5): _____  
1 = No awareness (ignored metrics)  
5 = High awareness (learned about rhythms)

**Specific insights gained:**
1. ________________________________
2. ________________________________
3. ________________________________

---

### Phase 3: Did it enable agency?  
**Goal:** User checks watch before decisions, acts on phase context

Rate (1-5): _____  
1 = No agency (never influenced behavior)  
5 = High agency (frequently guided decisions)

**Examples of decisions influenced by zone:**
1. ________________________________
2. ________________________________
3. ________________________________

**Triggering behavior developed?**
- [ ] Yes, I check before activities
- [ ] Sometimes, but not habitual yet
- [ ] No, I ignore the zone faces

---

### Trust & Reliability

**Do you trust the zone suggestions?**
- [ ] Yes, almost always
- [ ] Mostly, with occasional misses
- [ ] 50/50 (as often wrong as right)
- [ ] No, feels random

**When zone mismatches somatic state, why?**
- [ ] Algorithm missing context (stress, sleep quality)
- [ ] Metrics weighting wrong (motion overriding comfort)
- [ ] Display lag (zone change delayed >10 min)
- [ ] User chronotype mismatch (algorithm wrong about my rhythms)

---

### Usability & UX

**Is playlist rotation helpful or distracting?**
- [ ] Helpful (surfaces insights I wouldn't check manually)
- [ ] Neutral (don't notice it much)
- [ ] Distracting (updates too frequent)

**Is 70% clock dominance right?**
- [ ] Too much clock (want more metrics)
- [ ] Just right (balance)
- [ ] Too many metrics (want more clock)

**Face dwell time (10 sec) feels:**
- [ ] Too short (can't read metric)
- [ ] Just right
- [ ] Too long (want faster rotation)

**Overall, Phase 3 feels:**
- [ ] Invisible background (good)
- [ ] Subtle companion (good)
- [ ] Noisy distraction (bad)
- [ ] Ignored feature (bad)

---

## Comparison to Research Baselines

### vs Oura Ring

| Metric | Oura | Phase 3 | Pass? |
|--------|------|---------|-------|
| **Update cadence** | 1x/morning | 1/min (background) | ☐ |
| **Surfacing strategy** | App-based | Passive rotation | ☐ |
| **Battery life** | 5-7 days | _____ days | ☐ (>7 days) |
| **User checks/day** | 1-3x | _____ x | ☐ (similar) |
| **Triggering behavior** | Morning ritual | Pre-activity check | ☐ |

---

### vs Garmin Body Battery

| Metric | Garmin | Phase 3 | Pass? |
|--------|--------|---------|-------|
| **Update frequency** | 5 min | 1 min | ☐ |
| **Display updates** | Continuous | Zone-driven | ☐ |
| **Notifications** | 0 (unless <25) | 0 (passive) | ☐ |
| **Battery life** | 5-7 days | _____ days | ☐ (>7 days) |
| **Feels responsive?** | Yes (5 min OK) | _____ | ☐ |

---

### vs Sensor Watch Baseline

| Metric | Passive Clock | Phase 3 | Overhead |
|--------|---------------|---------|----------|
| **Battery life** | 1-2 years | _____ days | _____ % drain |
| **Power consumption** | 0.5 µA | _____ µA | _____ µA |
| **Display updates** | 1/sec (clock) | Zone-driven | Acceptable? ☐ |
| **User complaints** | None (simple) | _____ | Tolerable? ☐ |

**Acceptable overhead:**  
☑ Battery life >30 days (10x worse than passive = acceptable)  
☑ Power <5x baseline (2.5 µA / 0.5 µA = 5x = OK)

---

## Iteration Recommendations (Based on Observations)

### If Zone Changes Too Frequent (>8/day):
1. Increase `ZONE_CHANGE_DEBOUNCE_SEC` from 300→600 (10 min)
2. Increase `ZONE_CHANGE_HYSTERESIS` from 0.1→0.2 (20% buffer)
3. Reduce `SENSITIVITY_ZONE_CHANGE` from 1.0→0.8 (less sensitive)

### If Zone Changes Too Rare (<3/day):
1. Decrease `ZONE_CHANGE_DEBOUNCE_SEC` from 300→180 (3 min)
2. Increase `SENSITIVITY_ZONE_CHANGE` from 1.0→1.2 (more sensitive)
3. Check accelerometer thresholds (motion detected?)

### If Playlist Rotation Distracting:
1. Increase `CLOCK_DOMINANCE_PCT` from 70→80
2. Increase `DWELL_METRIC_FACE_SEC` from 10→15 (slower rotation)
3. Disable auto-rotation (manual mode only)

### If Metrics Ignored (No Agency):
1. Decrease `CLOCK_DOMINANCE_PCT` from 70→60 (more metric exposure)
2. Add haptic feedback (chirp on zone changes)
3. Re-evaluate metric relevance (are SD/Comfort/EM meaningful?)

### If Power Budget Exceeded (>3 µA):
1. Reduce `PHASE_UPDATE_INTERVAL_SEC` from 60→120 (slower updates)
2. Check accelerometer ODR (should be 1.6 Hz, not higher)
3. Reduce display refresh rate (update on zone change only, not ticks)

---

## Success Criteria (Pass/Fail)

### Minimum Viable Phase 3:

✅ **Battery life >7 days** (measure: _____ days)  
✅ **Zone changes align with somatic state >70% of time** (subjective)  
✅ **User checks watch before decisions** (triggering behavior established)  
✅ **Playlist rotation not distracting** (helpful or neutral, not annoying)  
✅ **Power consumption <3 µA** (measure: _____ µA)

**If all criteria pass → Phase 3 ready for daily wear**  
**If any fail → iterate tuning parameters, re-test**

---

## Post-Calibration Report Template

**Date:** ______  
**Duration:** _____ days  
**Hardware:** Green board + LIS2DW12  
**Firmware:** Commit hash ______

### Summary

**Overall rating (1-5):** _____  
1 = Complete failure, back to drawing board  
5 = Production-ready, ship it

**Key successes:**
1. ________________________________
2. ________________________________
3. ________________________________

**Key failures:**
1. ________________________________
2. ________________________________
3. ________________________________

### Quantitative Results

| Metric | Target | Actual | Pass? |
|--------|--------|--------|-------|
| Battery life | >7 days | _____ days | ☐ |
| Power consumption | <3 µA | _____ µA | ☐ |
| Zone changes/day | 4-8 | _____ | ☐ |
| Clock dominance | 70% | _____% | ☐ |
| Metric alignment | >70% | _____% | ☐ |

### Qualitative Results

**Phase 1 (Clock):** ☐ Pass ☐ Fail  
**Phase 2 (Awareness):** ☐ Pass ☐ Fail  
**Phase 3 (Agency):** ☐ Pass ☐ Fail

**Triggering behavior:**  
☐ Established (checks before decisions)  
☐ Developing (occasional checks)  
☐ Absent (ignores zones)

### Next Steps

**Tuning adjustments for next iteration:**
1. ________________________________
2. ________________________________
3. ________________________________

**Code changes required:**
1. ________________________________
2. ________________________________
3. ________________________________

**Research questions raised:**
1. ________________________________
2. ________________________________
3. ________________________________

---

**End of calibration checklist. Complete this over 7-14 days, then synthesize findings into tuning iteration.**
