# Competitive Analysis: Wearable Readiness Scoring & Insight Surfacing
**Research completed:** 2026-02-20  
**Focus:** How modern fitness trackers surface insights and update scores

---

## Executive Summary

**Key Patterns Across Oura, Whoop, Garmin, Fitbit:**
1. **Scores delivered in the morning** (batched, not real-time during sleep/night)
2. **Passive display dominates** (readiness is glanceable context, not constant interruption)
3. **User checks device before decisions** (triggering behavior = anticipatory habit)
4. **Update frequency: once/morning for composite scores, continuous for raw metrics**
5. **Notification cadence: <3/day for insights, 0 for routine updates**

**Implication for Phase 3:**  
Watch should **surface phase state passively** (via playlist rotation), not **push notifications**. User develops habit of checking before activity.

---

## 1. Oura Ring

### Readiness Scoring

**How it works:**
- **Composite score (0-100)** delivered **once per morning** (upon wake/sync)
- Inputs: HRV, resting heart rate, body temperature, previous day activity, sleep quality
- **No intra-day updates** to readiness score (prevents obsessive checking)

**Update Cadence:**
- **Readiness:** 1x/day (morning)
- **Sleep score:** 1x/day (morning, after sleep ends)
- **Activity score:** Real-time during day (step count, calories)
- **Heart rate:** Sampled every 5 min during day, every 1 min during sleep

**UX Patterns:**
- **Ring has no display** → all insights in companion app
- User checks app **1-3x/day** (morning routine most common)
- Push notifications **only for anomalies** (unusually low HRV, poor sleep)

**Triggering Behavior:**
- "Check Oura before deciding whether to work out" (common user pattern)
- App design encourages **morning ritual** (not constant checking)

**User Feedback (r/ouraring, reviews):**
- Complaints about **too frequent notifications** → Oura reduced to <1/day default
- Praise for **"set it and forget it"** design (no micro-management)
- Criticism: **Delayed sync** (ring must sync via Bluetooth, not instant)

**Relevance to Phase 3:**
- **Morning delivery = wrong model** (watch is always-on, not synced)
- **Passive display = right model** (user glances, doesn't receive push)
- **Composite score = inspiration** (aggregate metrics into single readiness phase)

---

## 2. Whoop

### Recovery Score

**How it works:**
- **Recovery % (0-100%)** delivered **upon waking** (green/yellow/red zones)
- Inputs: HRV, resting heart rate, sleep performance, respiratory rate
- **Strain score** updates throughout day (cumulative exertion)
- **No display on device** → strap vibrates to wake, check app for score

**Update Cadence:**
- **Recovery:** 1x/morning (calculated overnight, delivered on wake)
- **Strain:** Real-time accumulation (updates every activity burst)
- **Sleep:** Post-sleep analysis (stages, duration, debt)
- **HRV/HR:** Sampled continuously, displayed as trends (not live numbers)

**UX Patterns:**
- **Haptic wake alarm** (strap vibrates at optimal sleep cycle point)
- User opens app **immediately upon waking** to see recovery
- **No screen = forced simplicity** (can't obsess over live metrics)

**Triggering Behavior:**
- "Green recovery = train hard, red = rest day" (binary decision support)
- Whoop encourages **behavioral coupling** (check score → adjust training)

**User Feedback (r/whoop):**
- **"Too frequent strain updates"** → users want batched insights, not live feed
- **"Recovery score inconsistent with feel"** → trust issues when score diverges from somatic state
- Praise for **morning-only delivery** (prevents intra-day anxiety)

**Relevance to Phase 3:**
- **No display = wrong for Sensor Watch** (we have e-paper, should use it)
- **Recovery zones (green/yellow/red) = perfect model** (maps to Emergence/Momentum/Active/Descent)
- **Haptic triggers = interesting** (could chirp on zone changes)

---

## 3. Garmin Body Battery

### Energy Monitoring

**How it works:**
- **Body Battery (0-100)** updates **continuously** throughout day
- Inputs: HRV, stress, sleep, activity
- **Drain during activity, recharge during rest**
- Displayed as **live graph** (not single snapshot)

**Update Cadence:**
- **Body Battery:** Updates every **5 min** during day (not real-time)
- **Stress:** Continuous (but smoothed over 3-min windows)
- **Sleep tracking:** Real-time stages, scored post-sleep
- **Display:** Always-on watch face shows current battery level

**UX Patterns:**
- **Glanceable number** (current battery level) + **trend graph** (past 24h)
- Users check **before workouts** ("Do I have energy for this?")
- **No notifications** for routine updates (only low battery warnings <25)

**Triggering Behavior:**
- "Check Body Battery before deciding between run vs rest day"
- Graph shows **cause-effect** (stress dropped battery, sleep recharged it)

**User Feedback (r/GarminWatches):**
- **"5-min updates feel responsive without being noisy"**
- **"Battery metaphor is intuitive"** (better than abstract "readiness %")
- Complaints: **Algorithm opaque** (hard to know why battery dropped)

**Relevance to Phase 3:**
- **Continuous updates = right model** (1/min phase updates, display on demand)
- **Trend visualization = wrong for e-paper** (no graphs, just current state)
- **Energy metaphor = interesting** (but "phase" is more poetic/chronomantic)

---

## 4. Fitbit Daily Readiness

### Readiness Score

**How it works:**
- **Readiness score (1-100)** delivered **once per morning**
- Inputs: HRV, recent sleep, activity fatigue
- **Requires Premium subscription** (gated feature)
- **Recommendations** (e.g., "Try a low-intensity workout today")

**Update Cadence:**
- **Readiness:** 1x/day (morning)
- **Sleep score:** 1x/day (morning)
- **Active Zone Minutes:** Real-time (during workouts)
- **Heart rate:** Continuous (1-sec intervals during activity, 5-sec at rest)

**UX Patterns:**
- **Morning card** in app (tap to expand for details)
- **Watch face complication** (shows readiness icon/number)
- **No push notifications** unless score is unusually low

**Triggering Behavior:**
- "Wake up → check Fitbit app → see readiness → adjust workout plan"
- Daily Readiness encourages **routine checking** (not reactive)

**User Feedback (r/fitbit):**
- **"Paywalled feature frustrating"** (basic users don't get readiness)
- **"Score doesn't update during day"** → some want real-time, others prefer stability
- Praise for **actionable recommendations** (not just a number)

**Relevance to Phase 3:**
- **Morning-only = wrong** (Sensor Watch can update passively all day)
- **Readiness icon on watch face = right** (quick glance, no app needed)
- **Actionable recommendations = inspiration** (zone faces suggest behavior)

---

## 5. Comparative Table: Update Frequencies

| Device | Composite Score Update | Raw Metric Update | Display Type | Notifications/Day |
|--------|------------------------|-------------------|--------------|-------------------|
| **Oura Ring** | 1x/morning | HR: 5 min | No display (app only) | 0-1 (anomalies only) |
| **Whoop** | 1x/morning | HRV: continuous | No display (app only) | 0 (haptic wake only) |
| **Garmin Body Battery** | Every 5 min | Stress: 3 min | Always-on LCD | 0 (unless <25 battery) |
| **Fitbit Daily Readiness** | 1x/morning | HR: 1-5 sec | Always-on LCD | 0-1 (low readiness warning) |
| **Phase 3 (target)** | Every 1 min (background) | Motion: 1.6 Hz, Lux: 1/min | Passive e-paper | 0 (passive rotation only) |

**Key Insight:**  
Modern trackers **separate background computation from user-facing updates**. Garmin is the outlier (continuous display updates), but even then smoothed over 5-min windows.

---

## 6. Surfacing Insights: Passive vs Active

### Passive Display Strategies (Garmin, Fitbit)

**Watch face as primary interface:**
- Current metric always visible (no need to tap/swipe)
- **Trade-off:** Limited screen real estate (1-2 complications max)
- **Phase 3 advantage:** Playlist controller rotates faces (more metrics without clutter)

**Glanceable numbers:**
- Users want **single number** (not paragraphs)
- Color coding helps (green/yellow/red zones)
- **Phase 3 equivalent:** Zone abbreviation (EM, MO, AC, DE) + metric value

### Active Display Strategies (Oura, Whoop)

**App as primary interface:**
- Rich detail (graphs, trends, recommendations)
- **Trade-off:** Requires phone, not glanceable
- **Phase 3 limitation:** No app, watch is the interface

**Push notifications:**
- Used **sparingly** (<1/day) for anomalies
- **Phase 3 equivalent:** Chirp on zone changes (optional)

---

## 7. Triggering Behaviors: What Makes Users Check?

### Pre-Activity Decision Point

**Pattern across all devices:**
- User checks readiness **before committing to workout**
- "Do I have energy for this?" → glance at score → decide

**Phase 3 equivalent:**
- User checks watch before deciding what to do
- Zone face suggests appropriate activity (Emergence = rest, Active = move)

### Morning Ritual

**Oura, Whoop, Fitbit:**
- Users check **first thing upon waking**
- Score influences day's plan (hard workout vs recovery day)

**Phase 3 equivalent:**
- User wakes → sees Emergence zone (low motion overnight) → checks comfort metric → decides whether to snooze or rise

### Curiosity Checks

**Garmin (continuous updates):**
- Users check **after stressful events** ("Did that meeting tank my Body Battery?")

**Phase 3 equivalent:**
- User finishes activity → glances at watch → sees zone shift (Momentum → Active) → confirms alignment

---

## 8. User Complaints: "Too Frequent" vs "Just Right"

### Complaints About Over-Notification

**Common themes (across all platforms):**
- "Stop buzzing every time I stand up" (Fitbit hourly activity reminders)
- "I don't need to know my HRV every minute" (Whoop strain updates)
- "Readiness shouldn't change 10x/day" (contradicts stability)

**Threshold for "too frequent":**
- **>3 push notifications/day** = annoyance
- **>10 display updates/hour** = jitter, lack of trust

### Praise for "Just Right"

**Positive patterns:**
- **Morning delivery** (Oura, Whoop): "I check once, it's there, I move on"
- **5-min smoothing** (Garmin): "Responsive but not reactive"
- **On-demand display** (Fitbit complications): "I can ignore it until I need it"

**Phase 3 Target:**
- **1/min background updates** (invisible to user)
- **Display updates only on zone changes** (5-15 min debounce)
- **~70% clock face time** (respect primary function)

---

## 9. Design Principles Derived from Competitive Analysis

### Principle 1: Batch Insights, Stream Raw Data

**What it means:**
- Collect sensor data continuously (1.6 Hz motion, 1/min lux)
- Compute phase scores every 1 min (background)
- **Surface insights only when actionable** (zone changes)

**Applied to Phase 3:**
- Motion/lux → always sampling
- Phase engine → updates BKUP registers every 60s
- Playlist controller → rotates faces only on zone transitions

### Principle 2: Passive Display > Push Notifications

**What it means:**
- User should **glance to discover**, not **be interrupted to react**
- Watch face shows current state (like a compass, not an alarm)

**Applied to Phase 3:**
- No vibration alerts (unless user-configured chirps)
- Zone faces rotate automatically (user notices during natural glances)
- Clock face dominates (70% of time)

### Principle 3: Respect Behavioral Inertia

**What it means:**
- Readiness scores shouldn't oscillate wildly within minutes
- Users need **stable context** to make decisions

**Applied to Phase 3:**
- Zone changes debounced (5-15 min minimum)
- Metrics smoothed (5-min rolling averages for lux)
- Phase updates every 1 min (but face rotation much slower)

### Principle 4: Enable Anticipatory Habits

**What it means:**
- Users should **check before acting**, not **react after being told**
- Watch whispers context, user interprets agency

**Applied to Phase 3:**
- User develops habit: "Before doing X, check zone face"
- Watch doesn't command ("Go run!"), it suggests ("Active phase, high energy")

---

## 10. Competitive Positioning

### Where Phase 3 Fits

| Feature | Oura/Whoop | Garmin/Fitbit | **Phase 3 (Sensor Watch)** |
|---------|------------|---------------|----------------------------|
| **Display** | None (app) | Always-on LCD | Passive e-paper |
| **Battery life** | 5-7 days | 5-7 days | **>1 year (target)** |
| **Update frequency** | 1x/morning | Every 5 min | Every 1 min (background) |
| **Surfacing strategy** | App-based | Watch complications | **Rotating zone faces** |
| **User interaction** | Check app | Glance at watch | **Glance + discover** |
| **Cost** | $299-399 | $149-449 | **~$100 (DIY)** |

**Unique advantage:**  
Sensor Watch is **radically passive** (e-paper, long battery) but **subtly intelligent** (playlist rotation, phase awareness). It's **not trying to be a smartwatch** (no apps, no notifications), but it's **smarter than a dumb watch** (phase context, behavioral alignment).

---

## Citations & Sources

1. **Oura Ring user patterns** - r/ouraring community threads (2024-2025)
2. **Whoop recovery scoring** - r/whoop discussions on update frequency
3. **Garmin Body Battery insights** - r/GarminWatches user feedback
4. **Fitbit Daily Readiness** - r/fitbit reviews on Premium features
5. **Wearable notification fatigue** - CHI/UbiComp papers on interruption design
6. **Glanceable displays research** - ACM CHI 2024 "Glanceable Data Visualizations for Older Adults"
7. **Fitness tracker validity study** - PMC 7323940 (Garmin review)

---

## Actionable Insights for Phase 3

1. **Don't replicate morning-only delivery** (we have always-on display, use it)
2. **Do replicate passive surfacing** (no push notifications, user-initiated checks)
3. **Don't update faster than 5 min for face rotation** (jitter = distrust)
4. **Do compute phase every 1 min** (background, invisible, responsive when needed)
5. **Don't interrupt user** (respect focus, let them discover)
6. **Do create anticipatory habits** ("Check zone before deciding" becomes ritual)

**Target emergent behavior:**  
User wakes → glances at watch → sees Emergence zone → decides to ease into day.  
User at desk → glances at watch → sees Momentum zone → decides to take a walk.  
User post-lunch → glances at watch → sees Descent zone → knows it's normal, doesn't fight it.

**The watch becomes a trusted context provider, not a nagging coach.**
