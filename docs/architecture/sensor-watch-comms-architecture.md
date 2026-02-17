# Sensor Watch Pro — Unified Comms Architecture

## 1. System Architecture Overview

```
┌─────────────────────────────────────────────────────┐
│                  COMPANION APP                       │
│  (PWA / native wrapper via Capacitor)               │
│                                                      │
│  ┌──────────┐  ┌──────────┐  ┌───────────────────┐ │
│  │ TX Engine │  │ RX Engine│  │ HealthKit Bridge  │ │
│  │ (Screen   │  │ (Mic →   │  │ (HKHealthStore)   │ │
│  │  Flicker) │  │  FESK    │  │                   │ │
│  │  8-32 Hz  │  │  decode) │  │ Sleep Analysis    │ │
│  └─────┬─────┘  └────┬─────┘  │ Light Exposure    │ │
│        │              │        │ Activity (proxy)  │ │
│        ▼              ▼        └────────┬──────────┘ │
│  ┌─────────────────────────┐           │            │
│  │    Protocol Layer        │───────────┘            │
│  │  Framing / CRC / Chunks │                        │
│  └─────────────────────────┘                        │
└─────────────────────────────────────────────────────┘
          │ optical ▼          ▲ acoustic │
┌─────────────────────────────────────────────────────┐
│              SENSOR WATCH PRO                        │
│                                                      │
│  ┌────────────────────────────────────────────┐     │
│  │         Unified Comms Face                  │     │
│  │                                             │     │
│  │  MODE SELECT (long-press ALARM cycles):     │     │
│  │   [RX] ← optical config receive             │     │
│  │   [TX] ← acoustic data transmit             │     │
│  │   [ST] ← status / last sync info            │     │
│  │                                             │     │
│  │  Display: mode + progress bar (segments)    │     │
│  └────────────────────────────────────────────┘     │
│                                                      │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐            │
│  │ Sleep    │ │ Light    │ │ Activity │  Data       │
│  │ Stream 4 │ │ Log      │ │ Counters │  Stores     │
│  │ 70 B     │ │ 168 B    │ │ ~48 B    │             │
│  └──────────┘ └──────────┘ └──────────┘            │
└─────────────────────────────────────────────────────┘
```

### Unified Comms Face — UI Flow

| State | Display | ALARM btn | LIGHT btn |
|-------|---------|-----------|-----------|
| Idle  | `CO` + last sync age (`CO 2d`) | → Mode select | Backlight |
| Mode select | Flashing `RX` / `TX` / `ST` | Cycle mode | Confirm |
| RX active | `RX` + progress segments | Cancel → idle | — |
| TX active | `TX` + progress segments | Cancel → idle | — |
| Status | Scrolls: bytes pending, battery, last sync | → idle | — |

The face registers as a single movement module. Internally it owns a state machine:

```c
typedef enum {
    COMMS_IDLE,
    COMMS_MODE_SELECT,
    COMMS_RX_WAITING,   // light sensor polling, waiting for preamble
    COMMS_RX_ACTIVE,    // receiving flicker data
    COMMS_TX_QUEUING,   // user picks what to send
    COMMS_TX_ACTIVE,    // buzzer emitting FESK
    COMMS_STATUS
} comms_state_t;
```

---

## 2. Data Export Protocol

### 2.1 Packet Format (unified for both directions)

```
┌──────┬──────┬───────┬──────────┬───────┐
│ SYNC │ HDR  │  LEN  │ PAYLOAD  │ CRC8  │
│ 1 B  │ 1 B  │ 1 B   │ 0-60 B   │ 1 B   │
└──────┴──────┴───────┴──────────┴───────┘
```

- **SYNC**: `0xA5` — frame delimiter
- **HDR** (bitfield):
  - `[7:6]` direction: `00`=watch→app, `01`=app→watch
  - `[5:4]` stream type: `00`=sleep, `01`=light, `10`=activity, `11`=control
  - `[3:0]` sequence number (0-15, wraps)
- **LEN**: payload bytes (max 60 to fit ~1 second FESK bursts)
- **PAYLOAD**: stream-specific encoding (below)
- **CRC8**: CRC-8/MAXIM over HDR+LEN+PAYLOAD

### 2.2 Stream Encodings

#### Sleep Data (Stream Type 0x00)

70 bytes total, 7 nights × 10 bytes per night.

Per-night encoding (10 bytes):
```
Byte 0:    Date offset (days before today, 0-6)
Byte 1:    Bedtime hour (0-23) + quarter (2 bits) → 1 byte
Byte 2:    Wake hour + quarter → 1 byte
Byte 3-9:  7 bytes = 28 bins × 2 bits each
             00 = no data
             01 = face-up (sleeping)
             10 = face-down (deep sleep proxy)
             11 = tilted (restless/awake)
```

2 packets to send all 7 nights (4 nights + 3 nights, 40B + 30B).

#### Light Exposure (Stream Type 0x01)

168 bytes (24h × 7 days), each byte = hourly lux average mapped to log scale:
- `0x00` = dark (<1 lux)
- `0x40` = indoor (~100 lux)  
- `0x80` = bright indoor (~1000 lux)
- `0xFF` = direct sun (~100k lux)

3 packets (60+60+48 bytes).

#### Activity (Stream Type 0x02)

Per-day summary (7 bytes per day):
```
Byte 0:    Date offset
Byte 1-2:  Total motion events (uint16 LE)
Byte 2-3:  Tap-to-wake count (uint16 LE)  
Byte 5:    Active Hours compliance (bitmap, 8 slots)
Byte 6:    Smart alarm metadata:
             [7:4] trigger minute offset from target
             [3:2] trigger type (00=light sleep, 01=motion, 10=fallback, 11=manual)
             [1:0] dismissal (00=button, 01=motion, 10=timeout)
```

49 bytes total → 1 packet.

#### Control (Stream Type 0x03) — App → Watch

Used for configuration pushes over optical RX:
```
Byte 0:    Command ID
Byte 1-N:  Command payload

Commands:
  0x01  SET_TIME        (4 bytes: unix epoch, truncated)
  0x02  SET_ALARM       (3 bytes: hour, min, window_min)
  0x03  SET_ACTIVE_HRS  (2 bytes: start_hour, end_hour)
  0x04  ACK_SYNC        (1 byte: streams received bitmask)
  0x05  REQUEST_STREAM  (1 byte: stream type to send)
  0x10  FIRMWARE_BLOCK  (experimental — see §4)
```

### 2.3 Transfer Timing

Assumptions:
- FESK acoustic: ~10 bytes/sec reliable (conservative for buzzer→mic)
- Optical RX: ~4 bytes/sec (limited by light sensor sample rate)

| Data Set | Size | TX Time (acoustic) | RX Time (optical) |
|----------|------|--------------------|--------------------|
| Sleep (7 nights) | 70 B | ~9 sec | N/A (watch→app) |
| Light (7 days) | 168 B | ~19 sec | N/A |
| Activity (7 days) | 49 B | ~7 sec | N/A |
| **Full sync** | **287 B** | **~35 sec** | N/A |
| Set time + alarm | 8 B | N/A | ~2 sec |
| ACK + config | ~12 B | N/A | ~3 sec |

**35 seconds for a full week of data — very reasonable.** User holds watch near phone, watches progress bar fill.

### 2.4 Sync Protocol Sequence

```
1. User opens Companion App, taps "Sync"
2. App flickers: REQUEST_STREAM(0xFF) = "send everything"
   Watch light sensor decodes → enters TX mode
3. Watch buzzes: Sleep packets (seq 0-1)
4. Watch buzzes: Light packets (seq 2-4)  
5. Watch buzzes: Activity packet (seq 5)
6. Watch buzzes: END_OF_STREAM marker (seq 6, empty payload)
7. App flickers: ACK_SYNC(0b00000111) = "got all 3 streams"
8. Watch displays ✓, records sync timestamp
```

Retransmission: if app doesn't ACK within 10s, watch can re-send. App can request specific streams via REQUEST_STREAM.

---

## 3. Companion App Architecture

### 3.1 Platform Strategy

**PWA with Capacitor wrapper** for native capabilities:
- Web Audio API for FESK decoding (works in browser)
- Screen animation API for optical TX (requestAnimationFrame)
- Capacitor plugin for HealthKit (iOS native bridge)
- Works as standalone webpage for non-Apple users (minus HealthKit)

### 3.2 Core Modules

```
src/
├── comms/
│   ├── fesk-decoder.ts      # Mic → AudioWorklet → FSK demod → bytes
│   ├── blinky-encoder.ts    # Bytes → screen flicker patterns
│   ├── framing.ts           # Packet assembly, CRC, sequencing
│   └── sync-manager.ts      # Orchestrates full sync flow
├── data/
│   ├── sleep-parser.ts      # Decode sleep stream → SleepSession[]
│   ├── light-parser.ts      # Decode light stream → LightSample[]
│   ├── activity-parser.ts   # Decode activity → ActivityDay[]
│   └── store.ts             # IndexedDB for local history
├── health/
│   ├── healthkit-bridge.ts  # Capacitor HealthKit plugin wrapper
│   ├── sleep-mapper.ts      # SleepSession → HKCategorySample
│   ├── light-mapper.ts      # LightSample → HKQuantitySample (UV/daylight)
│   └── activity-mapper.ts   # ActivityDay → HKActivitySummary
├── ui/
│   ├── sync-screen.tsx      # Main sync interface
│   ├── dashboard.tsx        # Historical data viewer
│   ├── settings.tsx         # Watch config editor
│   └── health-status.tsx    # HealthKit sync status
└── app.ts
```

### 3.3 HealthKit Mapping

| Watch Data | HealthKit Type | Category/Quantity | Notes |
|------------|---------------|-------------------|-------|
| Sleep orientations | `HKCategoryTypeIdentifierSleepAnalysis` | `.asleepCore`, `.asleepDeep`, `.awake` | face-up→core, face-down→deep, tilted→awake |
| Bedtime/wake | `HKCategoryTypeIdentifierSleepAnalysis` | `.inBed` | Envelope of the session |
| Light exposure | `HKQuantityTypeIdentifierTimeInDaylight` | Minutes | iOS 17+, count hours ≥1000 lux |
| Motion events | `HKQuantityTypeIdentifierStepCount` | Proxy count | Not real steps but motion proxy, label clearly |
| Active Hours | `HKCategoryTypeIdentifierAppleStandHour` | `.stood` | Maps well conceptually |
| Smart alarm | `HKCategoryTypeIdentifierSleepAnalysis` | Wake time | End-of-sleep marker |

**Key insight:** iOS 17 added `HKQuantityTypeIdentifierTimeInDaylight` — this maps *perfectly* to the light sensor data. This alone could be a killer feature for people tracking circadian health.

### 3.4 UI Design — Sync Screen

```
┌─────────────────────────────┐
│  🔄 Sensor Watch Sync       │
│                              │
│  ┌───────────────────────┐  │
│  │                       │  │
│  │   [Flicker Zone]      │  │  ← This region flickers for optical TX
│  │   (place watch here)  │  │
│  │                       │  │
│  └───────────────────────┘  │
│                              │
│  Hold watch face-down on    │
│  the flicker zone, then     │
│  press START                │
│                              │
│  ┌─────────────────────┐    │
│  │    ▶ START SYNC      │    │
│  └─────────────────────┘    │
│                              │
│  Status: Waiting...          │
│  ░░░░░░░░░░░░░░░░░░░░ 0%   │
│                              │
│  Last sync: 3 days ago       │
│  Pending: 287 bytes          │
└─────────────────────────────┘
```

After optical handshake, screen shows "Listening..." and uses mic. User flips watch face-up (buzzer toward phone). Progress bar fills as packets arrive.

---

## 4. Creative Use Cases

### 4.1 Circadian Score & Coaching

Combine sleep timing + light exposure + Active Hours compliance into a single **Circadian Score (0-100)**. 

- Light before noon? +points
- Consistent bedtime? +points  
- Active Hours compliance? +points
- Late-night screen light detected? -points

Surface this in Apple Health as a custom metric. Use Shortcuts to trigger morning notifications: *"Your circadian score yesterday was 72. You got less morning light than usual — try 15 min outside before 10am."*

### 4.2 Sleep Environment Monitor

The light sensor logs overnight ambient light. If the companion app detects light intrusions during sleep hours (streetlight, partner's phone, early dawn), it can:
- Show a "sleep darkness score" 
- Recommend blackout curtains if light > threshold before wake time
- Correlate light intrusions with restlessness (tilted orientation bins)

### 4.3 Watch-to-Watch Sync (Social Sleep)

Two watches within buzzer/mic range of each other can exchange sleep scores acoustically. Couples' sleep dashboard:
- "You both fell asleep within 8 minutes of each other"
- "Partner was restless 2-4am — correlated with your waking at 3am?"

Protocol: one watch TX while other RX via light sensor pointed at phone screen (relay through app). Or direct acoustic if second watch has a mic mod.

### 4.4 Siri Shortcuts Integration

Expose sync as a Shortcut action. Automate:
- **"Hey Siri, sync my watch"** → Opens app, starts flicker, listens
- **Morning automation:** Auto-sync at 7am when phone is on nightstand (NFC tag trigger → Shortcut → sync → push summary notification)
- **Weekly report:** Shortcut queries HealthKit for the week's Sensor Watch data, generates a summary, reads it aloud

### 4.5 Jet Lag Protocol

When user changes time zones (detected by phone), the app can:
1. Calculate optimal light exposure schedule for circadian reset
2. Push new Active Hours config to watch via optical
3. Track compliance via light sensor ("Did you actually get morning light?")
4. Adjust smart alarm window during adaptation period

Uses the comms system for real-time protocol adjustment — the phone has timezone intelligence, the watch has the sensors.

### 4.6 Power Nap Detector

Activity data + sleep orientation can detect daytime naps (face-down on desk, no motion for 15+ min during Active Hours). The app can:
- Log naps separately in HealthKit
- Warn if naps are too late (after 3pm → circadian disruption)
- Track nap frequency trends

### 4.7 Acoustic "Beacon" for Find-My-Watch

Reverse use: app sends a specific optical command, watch responds with a distinctive buzzer pattern (not FESK, just audible chirps). Helps find a misplaced watch in the room. Silly but useful.

---

## 5. Technical Feasibility Notes

### Easy (v1 candidates)
- **FESK TX of sleep data** — 70 bytes, well-understood, ~9 seconds
- **Basic companion PWA** — Web Audio API for FESK is proven tech
- **Optical RX for time set** — Small payload, simple command
- **IndexedDB local storage** — Standard web API
- **HealthKit sleep mapping** — Well-documented API, clear mapping

### Medium (v1-stretch or v2)
- **Light exposure logging** — Needs firmware work for hourly averaging + rolling buffer, but straightforward
- **HealthKit TimeInDaylight** — iOS 17+ only, but the mapping is clean
- **Activity data export** — Needs to define what "motion event" means precisely
- **Circadian Score calculation** — Algorithm design, but no hard technical barriers
- **Capacitor native wrapper** — Boilerplate but necessary for HealthKit

### Hard (v2+)
- **Reliable FESK at 10+ bytes/sec** — Needs real-world testing across phone models, ambient noise conditions, buzzer variations. The codec works in lab; production reliability is the challenge
- **Optical RX reliability** — Light sensor bandwidth limits, ambient light interference, user needs to hold steady
- **Watch-to-watch communication** — No direct path without phone relay; protocol complexity jumps
- **Jet lag protocol** — Needs robust timezone detection + circadian model

### Experimental (research/v3)
- **Firmware update over acoustic** — At 10 B/s, a 16KB bootloader update = ~27 minutes. Technically possible with a robust bootloader that checksums blocks, but UX is brutal. Better approach: update only config/parameters acoustically, firmware via USB
- **Acoustic security** — Anyone in earshot can capture FESK data. Mitigations: (a) data is low-sensitivity health data, (b) could add simple XOR obfuscation with shared key set during first optical pair, (c) acoustic range is ~30cm with watch buzzer, so practical risk is low
- **Multi-watch arbitration** — Need TDMA or similar if multiple watches TX simultaneously

---

## 6. Implementation Phases

### Phase 1 — "It Works" (4-6 weeks)

**Watch firmware:**
- Unified Comms Face with TX mode only (acoustic)
- Sends sleep data (Stream 4) as FESK packets
- Basic framing: sync byte, length, CRC8
- Progress display using watch segments

**Companion app:**
- Static HTML page with Web Audio FESK decoder
- Decodes sleep packets → displays as table
- "Copy as JSON" button for manual export
- No HealthKit, no optical, no native wrapper

**Milestone:** Successfully transfer 7 nights of sleep data from watch to phone browser in <15 seconds.

### Phase 2 — "It's Useful" (6-8 weeks after Phase 1)

**Watch firmware:**
- Add RX mode (optical) for time sync + config
- Add light exposure logging (hourly buffer)
- Add activity summary stream
- Full bidirectional sync protocol with ACK

**Companion app:**
- Capacitor wrapper for iOS
- HealthKit integration (sleep + daylight)
- Optical TX for config (screen flicker)
- Persistent local history (IndexedDB)
- Dashboard with sleep/light/activity charts

**Milestone:** One-button sync that pushes a week of data to Apple Health. Partner sees your sleep data in the Health app's sharing tab.

### Phase 3 — "It's Smart" (ongoing after Phase 2)

- Circadian Score algorithm
- Siri Shortcuts integration
- Jet lag protocol
- Sleep environment analysis
- Notification coaching ("get outside!")
- Weekly email/notification summaries
- Watch-to-watch relay experiments

### Phase 4 — "It's an Ecosystem" (future)

- Android companion (Web Bluetooth? or stick with acoustic)
- Community sleep data research (anonymized, opt-in)
- Third-party integrations (Oura ring correlation, f.lux sync)
- Firmware parameter updates over acoustic
- Custom watch face configs pushed from app

---

## Appendix: Key Design Decisions

**Why PWA + Capacitor, not pure native?**
- FESK decoder (Web Audio) works in any browser — widest reach
- HealthKit requires native — Capacitor bridges this cleanly
- Single codebase, deploy as both website and iOS app
- Users without iPhones still get sync + local dashboard

**Why not Bluetooth?**
- Sensor Watch doesn't have BLE hardware
- The whole point is using existing hardware (buzzer, light sensor, screen)
- It's more interesting this way

**Why CRC8 not CRC16?**
- Packets are small (≤63 bytes). CRC8 catches >99.6% of errors at this size
- Saves 1 byte per packet = measurable at 10 B/s
- Retransmission handles the rare miss

**Why 60-byte max payload?**
- ~6 seconds per packet at 10 B/s — reasonable attention span
- Fits in watch RAM without issues
- Allows per-packet ACK without long waits
- Sleep data (largest single-type export) fits in 2 packets
