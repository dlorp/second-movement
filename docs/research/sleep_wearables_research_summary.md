# Sleep/Fitness Wearables: Movement Tracking & Sleep Scoring Research

**Research Date:** 2026-02-15  
**Context:** Sensor Watch hardware constraints - accelerometer + light sensor only  
**Focus:** What can we replicate without HRV, SpO2, skin temp, or heart rate?

---

## Executive Summary

Commercial sleep wearables rely heavily on **heart rate variability (HRV)** and **physiological signals** for accurate sleep stage detection. **Accelerometer-only methods** achieve 57-79% accuracy for 4-stage sleep classification (Wake/Light/Deep/REM), with best performance on binary sleep/wake detection (~85%).

**Key Finding:** Without HRV, we can reliably detect:
- Sleep/wake transitions (fair to moderate agreement)
- Movement-based sleep quality metrics
- Sleep timing and duration

We **cannot** reliably distinguish REM from deep sleep with movement alone.

---

## Platform-Specific Findings

### 1. **Apple Watch** (Best Documented)
**Official White Paper:** "Estimating Sleep Stages from Apple Watch" (Oct 2025)

#### Algorithm Approach:
- **Primary sensor:** Accelerometer (motion + orientation)
- **Method:** Deep neural network trained on PSG data
- **Architecture:** Self-supervised learning on large unlabeled datasets

#### Performance (Accelerometer-Only):
- **4-stage accuracy:** 57% (Wake/Light/Deep/REM)
- **2-stage accuracy:** ~90% (Sleep/Wake)
- Struggles most with: REM detection, wake periods without movement

#### Key Insight:
> "The accelerometer signals contain information about sleep stages through movement patterns, respiratory-induced motion, and postural changes during different sleep stages."

#### What They Actually Use:
- Accelerometer signals (900 samples per 30-second window)
- Derived features: arm angle, movement intensity, respiratory motion
- Heart rate & respiratory rate (from PPG sensor) improve accuracy to ~79%

**Takeaway for Sensor Watch:** Apple's accelerometer-only model is our ceiling. They use sophisticated deep learning but still only achieve 57% accuracy without physiological signals.

---

### 2. **Oura Ring** (Most Academic Validation)
**Key Paper:** "The Promise of Sleep: Multi-Sensor Approach" (MDPI 2021)

#### Sensor Suite:
- 3D accelerometer
- Infrared PPG (for HRV)
- NTC temperature sensor
- **All sensors required** for their algorithm

#### Performance:
- **Accelerometer-only:** 57% accuracy (4-stage)
- **With HRV + temp:** 79% accuracy (4-stage)
- **Improvement from adding physiological data:** +22% accuracy

#### Algorithm Details:
> "Features derived from accelerometer data alone are insufficient for reliable REM/NREM differentiation. The addition of temperature and HRV to a model relying on accelerometer data alone improved sensitivity and specificity significantly."

#### Sleep Score Components:
1. **Total sleep time** (duration)
2. **Sleep efficiency** (time asleep / time in bed)
3. **REM ratio** (requires HRV)
4. **Deep sleep ratio** (requires HRV)
5. **Sleep latency** (movement-based)
6. **Timing** (circadian alignment - uses light exposure if available)

**Takeaway for Sensor Watch:** Oura explicitly states accelerometer alone is insufficient. However, we can track duration, efficiency, and latency.

---

### 3. **Fitbit** (Proprietary but Documented Components)

#### Known Algorithm Components:
**Sleep Score Formula** (0-100):
- **Duration** (0-30 points): Hours slept vs age-appropriate target
- **Quality** (0-40 points): Deep + REM time, restlessness
- **Restoration** (0-30 points): Resting heart rate, HRV

#### Movement Detection:
- Uses 3-axis accelerometer at 100Hz
- Aggregates into 30-second "activity counts"
- Applies **Cole-Kripke algorithm** (legacy regression model from 1992)

#### Sleep Stage Detection:
- **Light sleep:** Moderate movement, elevated heart rate
- **Deep sleep:** Minimal movement, lowest heart rate
- **REM sleep:** Minimal movement, elevated heart rate (similar to wake)
- **Wake:** High movement OR elevated heart rate with motion

**Proprietary Challenge:** Exact weighting of features is undisclosed.

**Takeaway for Sensor Watch:** Fitbit's basic movement detection uses simple regression (Cole-Kripke). We can implement this. Stage detection requires heart rate.

---

### 4. **Garmin** (Body Battery + Sleep Score)

#### Body Battery Algorithm:
- **Inputs:** HRV (primary), stress level, sleep quality, activity level
- **Not available** without heart rate sensor
- Proprietary Firstbeat Analytics algorithm

#### Sleep Tracking:
- **Movement-based** detection on older devices without HR
- **Method:** Similar to Cole-Kripke (activity count thresholds)
- **Limitations:** Binary sleep/wake only, no stage detection

#### Sleep Score Components (newer devices):
1. Total sleep time
2. Deep + light + REM ratios (requires HR)
3. Movement during sleep
4. Respiratory rate (requires HR)

**Takeaway for Sensor Watch:** Garmin's older, movement-only devices provide a precedent for basic sleep tracking without physiological sensors.

---

### 5. **WHOOP** (Most Opaque)

#### Known Information:
- **Primary metric:** Recovery Score (heavily HRV-dependent)
- **Sleep detection:** Time in bed detection via accelerometer
- **Sleep stages:** Requires HRV analysis
- **Strain:** Activity tracking (movement + HR)

#### Algorithm:
- Proprietary, no public documentation
- Strong emphasis on HRV for all metrics
- Accelerometer used primarily for activity tracking, not sleep staging

**Takeaway for Sensor Watch:** WHOOP is the least replicable without HRV. Their entire value proposition depends on physiological signals.

---

## Academic Validation: Actigraphy Algorithms

### Classic Algorithms (Activity Count-Based)
Source: "40 years of actigraphy in sleep medicine" (Nature Digital Medicine 2023)

#### 1. **Cole-Kripke Algorithm** (1992)
- **Method:** Linear regression on activity counts
- **Input:** Sum of activity in current epoch + surrounding epochs
- **Formula:** `P(sleep) = 1 / (1 + e^(0.00001 * (550 - scale)))`
  - scale = weighted sum of activity counts in 11-minute window
- **Performance:** 
  - Sensitivity: 95.3% (detects sleep well)
  - Specificity: 46.1% (poor at detecting wake)
  - Best for: Total sleep duration estimates
- **Limitation:** Overestimates sleep (classifies still-awake periods as sleep)

#### 2. **Sadeh Algorithm** (1994)
- **Features:** Activity count variability, mean activity, standard deviation
- **Performance:** Similar to Cole-Kripke (~85% overall accuracy)
- **Advantage:** Slightly better wake detection
- **Limitation:** Still struggles with quiet wakefulness

#### 3. **Oakley Algorithm** (1997)
- **Method:** Logistic regression with rescoring rules
- **Rescoring:** Heuristic adjustments based on bout length
- **Performance:** 
  - With rescoring: Best specificity (62.8%) among classic algorithms
  - RMSE for WASO: 12.7 min (best among legacy algorithms)
- **Advantage:** Better at detecting wakefulness in bed

#### 4. **van Hees Algorithm** (2015)
- **Method:** Angle-based (uses arm orientation, not just movement)
- **Input:** Z-axis angle range over 5-second windows
- **Performance:**
  - Sensitivity: 83.6%
  - Specificity: ~50%
  - F1 score: 79.1 (best among heuristic methods)
- **Advantage:** Works on raw accelerometer data (no proprietary "counts")

### Modern Deep Learning Approaches

#### Self-Supervised Neural Networks (2024)
Source: "Self-supervised learning of accelerometer data" (Nature Digital Medicine 2024)

**Architecture:**
- ResNet-17 feature extractor (pre-trained on 700,000 person-days)
- Bi-directional LSTM for temporal dependencies
- Multi-task learning objective

**Performance:**
- **3-class (Wake/REM/NREM):** F1 = 0.57 (internal), 0.49 (external)
- **2-class (Sleep/Wake):** F1 = 0.75 (internal), 0.66 (external)
- **Cohen's Kappa:** 0.39 (fair agreement) for 3-class

**Validation Stats (Bland-Altman):**
- Total sleep duration: Mean bias +9.9 min, 95% LoA: -100 to +120 min
- REM duration: Mean bias -24.4 min, 95% LoA: -137 to +88 min
- NREM duration: Mean bias +34.4 min, 95% LoA: -106 to +175 min

**Key Finding:**
> "Deep learning models did not significantly outperform simple heuristic methods (van Hees, Cole-Kripke) for sleep outcome estimation. This may be due to: (1) overfitting to PSG annotation styles, (2) intrinsic limitations of using motion data alone, (3) differences in activity count computation across datasets."

---

## What Movement Patterns Reveal About Sleep Stages

### Physiological Basis
**Source:** Multiple PSG validation studies

#### Wake:
- **Movement:** Frequent, large-amplitude movements
- **Pattern:** Irregular, voluntary motion
- **Accelerometer signature:** High variance, sustained activity

#### Light Sleep (NREM Stage 1 & 2):
- **Movement:** Occasional body position changes
- **Pattern:** Periodic (~1-2 times per hour)
- **Accelerometer signature:** Low baseline, intermittent spikes
- **Confound:** Quiet wake looks identical

#### Deep Sleep (NREM Stage 3):
- **Movement:** Minimal, mostly respiratory motion
- **Pattern:** Very stable position
- **Accelerometer signature:** Lowest variance, near-zero activity
- **Confound:** REM atonia also produces minimal movement

#### REM Sleep:
- **Movement:** Minimal (muscle atonia), occasional twitches
- **Pattern:** Paradoxical - looks like deep sleep motorically
- **Accelerometer signature:** Low activity, occasional micro-movements
- **Critical limitation:** **Indistinguishable from deep sleep without physiological signals**

### Why Accelerometer-Only Struggles:
> "REM sleep and deep sleep both exhibit minimal movement. The key differentiator is elevated heart rate during REM (approaching wake levels) versus lowest heart rate during deep sleep. Without HR monitoring, these stages cannot be reliably separated."

---

## Circadian Rhythm Tracking

### What Wearables Track:

#### 1. **Sleep Timing Regularity**
- **Metric:** Variation in sleep onset/offset times
- **Method:** Standard deviation of sleep start times across days
- **Achievable with:** Accelerometer + light sensor
- **Clinical relevance:** Irregular sleep timing associated with metabolic syndrome

#### 2. **Light Exposure Patterns**
- **Metric:** Timing and intensity of light exposure
- **Method:** Ambient light sensor readings during wake hours
- **Achievable with:** Light sensor (lux measurements)
- **Use:** Estimate circadian phase shifts, jet lag recovery

#### 3. **Activity-Rest Rhythm**
- **Metric:** 24-hour activity patterns (cosinor analysis)
- **Method:** Fit sinusoidal curve to daily activity profile
- **Achievable with:** Accelerometer
- **Parameters:**
  - **Mesor:** Midline of rhythm (average activity)
  - **Amplitude:** Strength of rhythm
  - **Acrophase:** Timing of peak activity

#### 4. **Social Jet Lag**
- **Metric:** Difference between weekday and weekend sleep timing
- **Method:** Compare average sleep midpoint (weekend vs weekday)
- **Achievable with:** Accelerometer
- **Interpretation:** >1 hour difference indicates circadian misalignment

### What Requires Physiological Signals:
- **Core body temperature rhythm** (requires skin temp sensor)
- **Melatonin dim light onset** (requires blood/saliva sampling)
- **Cortisol awakening response** (requires cortisol sensor - not in wearables)

**Takeaway for Sensor Watch:** We can track behavioral circadian markers (sleep timing, light exposure, activity rhythm) but not physiological markers.

---

## Replicable Features for Sensor Watch

### High Confidence (Validated, Accelerometer-Only):

1. **Sleep/Wake Detection**
   - **Algorithm:** van Hees angle-based OR Cole-Kripke activity count
   - **Expected accuracy:** 85-90%
   - **Use:** Total sleep duration, sleep efficiency

2. **Sleep Window Detection**
   - **Algorithm:** Random forest with HMM smoothing (validated 90%+ precision)
   - **Use:** Identify overnight sleep period vs naps

3. **Wake After Sleep Onset (WASO)**
   - **Algorithm:** Sum of wake epochs within sleep window
   - **Expected error:** ±12-18 min RMSE
   - **Use:** Sleep fragmentation metric

4. **Movement During Sleep**
   - **Metric:** Count of movement events during sleep
   - **Threshold:** Activity count > X (device-specific calibration)
   - **Use:** Restlessness index

5. **Sleep Latency**
   - **Metric:** Time from bed entry to sustained sleep
   - **Method:** First 20-minute window with <10% wake epochs
   - **Use:** Insomnia screening

### Medium Confidence (Needs Validation):

6. **Light Exposure Integration**
   - **Metric:** Lux-hours during day, darkness during sleep
   - **Use:** Circadian rhythm alignment proxy
   - **Limitation:** Wrist-worn sensor may not reflect retinal light exposure

7. **Activity-Based Readiness**
   - **Metric:** Combination of:
     - Yesterday's activity level
     - Sleep duration
     - Sleep efficiency
   - **Algorithm:** Weighted sum (requires tuning)
   - **Use:** Daily readiness score (without HRV)

### Low Confidence (Exploratory):

8. **Coarse Sleep Staging**
   - **Classes:** Deep sleep vs light sleep (collapse REM into light)
   - **Method:** Movement variance thresholds
   - **Expected accuracy:** ~60-70%
   - **Use:** Rough sleep quality assessment
   - **Limitation:** Will misclassify REM as deep sleep

---

## Key Academic Papers for Implementation

### Must-Read for Algorithm Development:

1. **"40 years of actigraphy in sleep medicine"** (Patterson et al., 2023)
   - Systematic comparison of 8 sleep algorithms
   - Validation on 1,113 PSG nights
   - Provides formula for Cole-Kripke, Sadeh, Oakley
   - **Recommendation:** Implement Oakley with rescoring for best WASO accuracy

2. **"Self-supervised learning of accelerometer data"** (Yuan et al., 2024)
   - State-of-the-art deep learning approach
   - Large-scale validation (UK Biobank + 1,166 PSG nights)
   - Open-source code available: github.com/OxWearables/asleep
   - **Limitation:** Requires Python/PyTorch (not feasible for embedded systems)

3. **"A novel, open access method to assess sleep duration"** (van Hees et al., 2015)
   - Raw accelerometer algorithm (no proprietary counts)
   - Simple angle-based method
   - **Best for:** Resource-constrained devices
   - **Formula:** `Z-angle range over 5s windows`

### Supplementary Reading:

4. **"Sleep classification from wrist-worn accelerometer data using random forests"** (Sundararajan et al., 2021)
   - Random forest features for sleep/wake
   - Performance: F1 = 0.76 (accelerometer-only)

5. **"Estimating Sleep Stages from Apple Watch"** (Apple Inc., 2025)
   - Industry best-practices
   - Neural network architecture details
   - Accelerometer-only baseline results

---

## Implementation Recommendations for Sensor Watch

### Phase 1: Basic Sleep Tracking (Achievable Now)

#### Algorithm: **van Hees Angle-Based**
**Why:** Simple, open-source, no proprietary "activity counts" needed

**Requirements:**
- 3-axis accelerometer (✓ you have this)
- 30Hz sampling (or downsample to 30Hz)
- 30-second epoch classification

**Outputs:**
- Total sleep duration (±15 min error expected)
- Sleep efficiency (time asleep / time in bed)
- Sleep onset/offset times

**Configuration:**
- **Active Hours:** Use to define expected sleep window (exclude daytime motion)
- **Smart Alarm:** Trigger during detected wake/light movement window

---

### Phase 2: Enhanced with Light Sensor

#### Add: **Circadian Metrics**

**Metrics:**
1. **Light Exposure Timing**
   - Lux measurements during wake hours
   - Duration in bright light (>1000 lux)
   - Evening light exposure (<2h before sleep)

2. **Sleep Regularity Index**
   - Standard deviation of sleep onset times
   - Weekday vs weekend comparison

**Use Cases:**
- "Your sleep was 30 min later than usual"
- "You got only 15 min of bright light today"
- "Your weekend sleep is shifted by 1.5 hours (social jet lag)"

---

### Phase 3: Advanced (Requires Tuning)

#### Coarse Sleep Quality Score (0-100)

**Components:**
1. **Duration Score** (0-30): Hours slept vs 7-9h target
2. **Efficiency Score** (0-40): % time asleep when in bed
3. **Movement Score** (0-30): Restlessness during sleep (inverted)

**Formula:**
```
duration_score = min(30, (sleep_hours / 8) * 30)
efficiency_score = (time_asleep / time_in_bed) * 40
movement_score = 30 - (movement_events * restlessness_penalty)
sleep_score = duration_score + efficiency_score + movement_score
```

**Limitation:** This is a heuristic approximation. Without HRV, cannot assess true "restoration."

---

## Critical Limitations to Communicate

### What Sensor Watch CANNOT Do (Without HR/HRV):

1. **Distinguish REM from Deep Sleep**
   - Both have minimal movement
   - Requires heart rate/respiratory signals
   - Any "REM detection" claim would be misleading

2. **Measure Recovery/Readiness Accurately**
   - True readiness requires HRV (autonomic nervous system state)
   - Movement-based proxies are weak correlates

3. **Detect Sleep Disorders**
   - Sleep apnea: Requires SpO2 or breathing effort detection
   - Restless leg syndrome: May detect movements but not diagnose
   - Insomnia: Can track symptoms (long latency, low efficiency) but not diagnose

4. **Match Oura/WHOOP Accuracy**
   - Those devices use HRV as primary signal
   - Accelerometer-only ceiling is ~57-79% accuracy for staging

---

## Validation Strategy

### Before Public Release:

1. **Benchmark Against Existing Algorithms**
   - Implement Cole-Kripke or van Hees
   - Compare your output to research-grade actigraph (ActiGraph GT3X)

2. **Collect Ground Truth Data**
   - Recruit 10-20 users with concurrent sleep diary
   - Compare detected sleep windows to self-report
   - Calculate precision/recall for sleep window detection

3. **Bland-Altman Agreement Analysis**
   - Plot (Sensor Watch - Sleep Diary) vs average
   - Calculate mean bias and 95% limits of agreement
   - Target: <30 min mean bias for total sleep duration

4. **Face Validity Checks**
   - Does sleep duration vary by age as expected?
   - Do shift workers show irregular sleep patterns?
   - Do weekends show later sleep onset?

---

## Summary Table: Feature Feasibility

| Feature | Feasibility | Expected Accuracy | Sensor Req. |
|---------|-------------|-------------------|-------------|
| Sleep/Wake Detection | ✅ High | 85-90% | Accelerometer |
| Total Sleep Duration | ✅ High | ±15-30 min | Accelerometer |
| Sleep Efficiency | ✅ High | ±5-10% | Accelerometer |
| WASO | ✅ Medium | ±15-25 min | Accelerometer |
| Sleep Latency | ✅ Medium | ±10-20 min | Accelerometer |
| Sleep Regularity | ✅ High | N/A (tracking metric) | Accelerometer |
| Light Exposure Timing | ✅ High | Depends on sensor | Light sensor |
| Circadian Phase Estimate | ⚠️ Medium | Low precision | Accel + Light |
| Light vs Deep Sleep | ❌ Low | 60-70% | Accelerometer |
| REM Detection | ❌ Not Feasible | <50% (guessing) | Requires HRV |
| Sleep Score (composite) | ⚠️ Medium | Heuristic only | Accelerometer |
| Recovery/Readiness | ❌ Low | Weak proxy | Requires HRV |

**Legend:**
- ✅ High: Validated in research, reproducible
- ⚠️ Medium: Possible but needs tuning/validation
- ❌ Low/Not Feasible: Requires sensors you don't have

---

## Recommended Metrics for Sensor Watch

### Tier 1: Core Sleep Metrics (Display prominently)
1. **Total Sleep Duration** (hours:minutes)
2. **Sleep Efficiency** (percentage)
3. **Sleep Onset Time** (HH:MM)
4. **Wake Time** (HH:MM)

### Tier 2: Sleep Quality Indicators (Secondary display)
5. **Awakenings** (count of wake episodes >5 min)
6. **Restlessness** (movement events during sleep)
7. **Sleep Latency** (time to fall asleep)

### Tier 3: Circadian Health (Weekly summary)
8. **Sleep Regularity** (consistency score 0-100)
9. **Light Exposure** (hours in bright light)
10. **Social Jet Lag** (weekend vs weekday shift)

### Do NOT Claim:
- "Deep sleep" or "REM sleep" percentages (inaccurate without HR)
- "Recovery score" or "readiness" (misleading without HRV)
- Medical-grade sleep apnea detection (requires SpO2)

---

## Open Questions for Further Research

1. **Can light sensor improve movement-only sleep detection?**
   - Hypothesis: Darkness during sleep window may help disambiguate still-awake vs asleep
   - Requires validation study

2. **Does wrist orientation add value to movement counts?**
   - Hypothesis: Arm angle may correlate with sleep depth (supine vs side sleeping)
   - van Hees algorithm suggests yes, but needs device-specific tuning

3. **Can multi-night aggregation improve accuracy?**
   - Hypothesis: Averaging across 7 nights may reduce per-night error
   - Useful for weekly "sleep quality" trends vs nightly precision

4. **What is the minimum sampling rate for reliable sleep detection?**
   - Current algorithms use 30-100Hz; can we reduce to save battery?
   - May be acceptable if power budget is constrained

---

## Citations & Data Sources

### Primary Research Papers:
1. Patterson et al. (2023). "40 years of actigraphy in sleep medicine." *npj Digital Medicine*, 6, 51.
2. Yuan et al. (2024). "Self-supervised learning of accelerometer data provides new insights for sleep." *npj Digital Medicine*, 7, 86.
3. Apple Inc. (2025). "Estimating Sleep Stages from Apple Watch." Technical White Paper.
4. Altini et al. (2021). "The Promise of Sleep: Multi-Sensor Approach for Accurate Sleep Stage Detection Using Oura Ring." *Sensors*, 21(13), 4302.
5. van Hees et al. (2015). "A novel, open access method to assess sleep duration using a wrist-worn accelerometer." *PLoS ONE*, 10(11), e0142533.

### Industry References:
- Oura Blog: "Developing Oura's Latest Sleep Staging Algorithm" (2022)
- Garmin: "Body Battery Energy Monitoring" (Technology page)
- Fitbit Help: "Sleep Score Calculation" (Support documentation)

### Open-Source Code:
- Oxford Wearables: github.com/OxWearables/asleep (Python sleep staging)
- ActiGraph: github.com/actigraph/Sleep-Wake-Classification (validation code)

---

**End of Report**  
*For questions or implementation guidance, consult the cited papers or reach out to the research community via GitHub issues on the Oxford Wearables repository.*
