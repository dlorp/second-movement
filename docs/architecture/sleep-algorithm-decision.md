# Sleep Tracking Algorithm Decision - Sensor Watch Pro

**Decision Date:** 2026-02-15  
**Decision:** Cole-Kripke (1992) + Light Enhancement (Hybrid)

---

## The Choice

**Implement Cole-Kripke as the core sleep/wake detection algorithm, enhanced with light sensor state as a disambiguation signal.**

Not van Hees. Not Sadeh. Not CircaCP. Not Bayesian HMM.

---

## Rationale

### Why Cole-Kripke as the Base:

1. **Best sensitivity (0.88-0.96)** — For a sleep tracker, false negatives (missing sleep) are worse than false positives (calling wake "sleep"). Users will forgive overcounting a few wake minutes; they won't forgive a tracker that says they slept 4 hours when they slept 7.

2. **Trivially implementable in C** — The algorithm is a weighted linear sum over an 11-minute sliding window of activity counts. ~11 integer multiplications and one comparison per epoch. Fits in <200 bytes of RAM, runs in microseconds.

3. **Most validated algorithm in existence** — 30+ years of actigraphy research. Known failure modes. Known tuning parameters. Massive literature to reference when debugging.

4. **Activity counts map directly to LIS2DW12 capabilities** — The 6D orientation detection + wake-on-motion interrupt gives us exactly what Cole-Kripke needs: a count of motion events per epoch.

### Why NOT the Alternatives:

- **Sadeh:** Better specificity but lower sensitivity. On embedded with no HR to disambiguate, we need high sensitivity. The specificity gap is where light sensor steps in.
- **van Hees:** Kappa of 0.39 is mediocre. Angle-based approach sounds appealing for 6D orientation, but research shows it doesn't outperform Cole-Kripke.
- **CircaCP:** Unsupervised and handles irregular patterns — appealing for v2, but requires change-point detection algorithms that are computationally heavier and harder to debug on embedded. Bad first choice.
- **Bayesian HMM:** Requires floating-point matrix operations, forward-backward algorithm, parameter estimation. Way too heavy for <20µA power budget on a watch MCU.

### Why Light Enhancement is the Killer Feature:

Cole-Kripke's weakness is **specificity** — it can't tell "lying still awake in bed" from "asleep." This is where the light sensor changes the game:

- **Dark + still = probably asleep** (high confidence)
- **Light + still = probably awake** (reading, watching TV, lying on couch)
- **Dark + moving = probably awake** (restless, getting up at night)
- **Light + moving = definitely awake**

No other wrist-only device in this class has a light sensor integrated into sleep scoring. This is the differentiator. A simple 2-bit light state (dark/dim/moderate/bright) as a modifier to the Cole-Kripke threshold can boost specificity by 10-15%.

---

## Implementation Plan

### Phase 1 (v1) — Basic Sleep/Wake Detection
**Complexity:** Easy | **Timeline:** 1-2 weeks

```c
Core algorithm:
- Sample LIS2DW12 wake-on-motion interrupts per 30-second epoch
- Accumulate activity counts into 1-minute bins
- Apply Cole-Kripke weighted sum over 11-minute window:
  score = 0.001*(404*A₋₅ + 598*A₋₄ + 326*A₋₃ + 441*A₋₂ 
          + 1408*A₋₁ + 598*A₀ + 326*A₊₁ + 441*A₊₂ 
          + 404*A₊₃ + 598*A₊₄)
  if score < 1: classify SLEEP, else: WAKE
- Store classification in circular buffer (8 hours = 480 epochs = 60 bytes)

Power strategy:
- LIS2DW12 in low-power mode with wake-on-motion interrupt
- MCU sleeps between interrupts
- Activity count = interrupt count per epoch
- Current draw: ~3µA (accelerometer) + ~1µA (MCU wake duty cycle) = ~4µA
- Light sensor: single ADC read per epoch (negligible power)

Outputs:
- Total sleep duration
- Sleep onset / offset time
- Sleep efficiency
- Number of awakenings (>5 min wake bouts)
```

### Phase 2 (v1.5) — Light-Enhanced Scoring
**Complexity:** Easy-Medium | **Timeline:** 1 week additional

```c
Enhancement:
- Read light sensor ADC once per 30-second epoch
- Classify: DARK (0-10), DIM (11-50), MODERATE (51-150), BRIGHT (151-255)
- Modify Cole-Kripke threshold:
  - If DARK: lower wake threshold (bias toward sleep classification)
  - If BRIGHT: raise wake threshold (bias toward wake classification)
  - Effectively: threshold = base_threshold + light_modifier[light_class]

Tuning parameters (4 integers):
  light_modifier = {-200, -50, +100, +400}  // DARK, DIM, MOD, BRIGHT

Additional outputs:
- Light exposure timeline (for Circadian Score integration)
- Sleep environment quality (% time in darkness during sleep)
```

### Phase 3 (v2) — Smart Alarm + Sleep Quality Score
**Complexity:** Medium | **Timeline:** 2-3 weeks

```c
Smart alarm:
- Within 30-min window before alarm time
- Trigger on: wake epoch OR movement spike OR orientation change
- Uses existing Cole-Kripke classification + 6D orientation interrupt

Sleep Quality Score (0-100):
- Duration: 0-30 pts (hours vs 7-9h target)
- Efficiency: 0-40 pts (% time asleep in bed)  
- Restlessness: 0-15 pts (inverse of movement events)
- Environment: 0-15 pts (% darkness during sleep, light sensor)

Circadian Score integration:
- Feed light exposure data to Circadian Score module (5% weight)
- Track sleep regularity (σ of onset times over 7 days)
```

### Phase 4 (v2+, if needed) — CircaCP Fallback
**Complexity:** Hard | **Only if Cole-Kripke fails for irregular sleepers**

```c
If validation shows Cole-Kripke fails badly for:
- Shift workers
- Polyphasic sleepers  
- Irregular schedules

Then implement simplified CircaCP:
- Change-point detection on 24h activity profile
- No training data needed (unsupervised)
- Heavier computation but can run once per day (not per epoch)
```

---

## Validation Strategy (No PSG Required)

### Tier 1: Self-validation (Week 1-2)
- 5-10 beta testers wear Sensor Watch + keep sleep diary
- Compare detected sleep/wake times to diary entries
- Target: <30 min mean bias on total sleep time
- Calculate sensitivity, specificity, accuracy

### Tier 2: Cross-device validation (Week 3-4)
- Wear Sensor Watch alongside a commercial tracker (Fitbit, Apple Watch, Oura)
- Compare epoch-by-epoch agreement
- Bland-Altman plots for sleep duration
- Target: Cohen's Kappa >0.6 for sleep/wake agreement

### Tier 3: Edge case testing (Ongoing)
- Test during known wake-in-bed scenarios (reading, phone use)
- Test nap detection
- Test with different sleep positions
- Log light sensor readings to verify dark/light classification accuracy

---

## Risk Mitigation

| Risk | Likelihood | Mitigation |
|------|-----------|------------|
| Cole-Kripke overestimates sleep | High | Light sensor enhancement directly addresses this |
| Activity count calibration differs from research actigraphs | Medium | Tune threshold with sleep diary validation; expose calibration parameter |
| Light sensor blocked by sleeve/blanket | Medium | Default to standard Cole-Kripke when light = 0 for extended periods |
| Shift workers get bad results | Low-Medium | Phase 4 CircaCP fallback; Active Hours window helps bound expectations |
| Users expect sleep staging (REM/Deep) | High | Clear documentation: "sleep/wake detection only." Don't claim what we can't deliver |

---

## Summary

| Aspect | Decision |
|--------|----------|
| **Primary algorithm** | Cole-Kripke (1992) |
| **Enhancement** | Light sensor threshold modifier |
| **Fallback** | CircaCP (if irregular schedules fail) |
| **Implementation complexity** | v1: Easy, v1.5: Easy-Medium, v2: Medium |
| **RAM requirement** | ~80 bytes (activity buffer + state) |
| **Power budget** | ~4-5µA (well under 20µA ceiling) |
| **Expected accuracy** | 85-90% sleep/wake (higher with light) |
| **Validation** | Sleep diary + commercial tracker comparison |

---

## Key Insight

**The light sensor is the unfair advantage.** No classic actigraphy study had ambient light as a feature because research actigraphs didn't have light sensors integrated into the scoring algorithm. Sensor Watch Pro does. Cole-Kripke + light gives us a better algorithm than any single published method, while remaining dead simple to implement and debug on embedded hardware.

---

## References

- Cole, R.J., Kripke, D.F., Gruen, W., Mullaney, D.J., & Gillin, J.C. (1992). Automatic sleep/wake identification from wrist activity. *Sleep*, 15(5), 461-469.
- Panesar et al. (2025). Comparison and Validation of Actigraphy Algorithms. *JMIR Formative Research*.
- Gao et al. (2022). Actigraphy-Based Sleep Detection: Validation with Polysomnography. *Nature and Science of Sleep*.
- pyActigraphy documentation (Hammad et al., 2024)
