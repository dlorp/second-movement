# Optical RX Specification (Phase 2)

## Overview
Optical data reception for Sensor Watch Pro using ambient light sensor (ALS) to decode screen flashing from companion app.

## Hardware Requirements
- **Sensor:** Ambient light sensor (IR sensor on Sensor Watch Pro)
- **API:** `adc_get_analog_value(HAL_GPIO_IRSENSE_pin())`
- **Sampling rate:** 64 Hz minimum (16ms per sample) for reliable decoding

## Protocol Design

### Encoding: Manchester Encoding
**Why Manchester?**
- Self-clocking (no separate clock signal needed)
- DC-balanced (avoids light sensor drift)
- Error detection built-in (invalid transitions = errors)
- Standard in optical communication (used by Timex Datalink)

**Manchester Encoding Rules:**
- Logical `0`: HIGH → LOW transition (screen: light → dark)
- Logical `1`: LOW → HIGH transition (screen: dark → light)
- Transition always happens at bit center
- Clock recovery from transitions

**Example:**
```
Data:      0    1    0    0    1    1
Manchester:▁▔  ▔▁  ▁▔  ▁▔  ▔▁  ▔▁
           ↓    ↑    ↓    ↓    ↑    ↑
```

### Timing
- **Bit rate:** 10 bps (bits per second) - slow but reliable
- **Bit duration:** 100ms per bit
- **Half-bit (cell) duration:** 50ms
- **Sample rate:** 64 Hz (15.6ms per sample, ~3 samples per half-bit)

**Why 10 bps?**
- Reliable detection on phone screens (iOS/Android refresh rate variations)
- Tolerant to light sensor noise and ambient light
- Timex Datalink used ~6 bps, we use 10 bps for faster sync

### Packet Format
```
[SYNC] [LEN] [TYPE] [DATA] [CRC8]

SYNC:  0xAA (10101010 binary) - alternating pattern for clock sync
LEN:   1 byte (0-64, payload length)
TYPE:  1 byte (packet type: 0x01 = time sync, 0x02 = config, 0x03 = ack)
DATA:  0-64 bytes (variable payload)
CRC8:  1 byte (CRC-8/MAXIM checksum)
```

**Maximum packet:** 68 bytes @ 10 bps = ~54 seconds

### Packet Types

**1. Time Sync (TYPE=0x01)**
```
DATA: [timestamp (4 bytes)] [timezone_offset (2 bytes)]
timestamp = Unix timestamp (UTC)
timezone_offset = minutes from UTC (-720 to +840)

Example:
SYNC: AA
LEN:  06 (6 bytes)
TYPE: 01 (time sync)
DATA: 12 34 56 78 (timestamp = 0x78563412)
      E0 FF (offset = -32 minutes = -0.5 hours)
CRC8: XX
```

**2. Config Update (TYPE=0x02)**
```
DATA: [config_key (1 byte)] [config_value (N bytes)]

Config keys:
0x10 = Active Hours Start (2 bytes, minutes since midnight)
0x11 = Active Hours End (2 bytes, minutes since midnight)
0x12 = Smart Alarm Window (1 byte, minutes)
0x13 = Smart Alarm Enabled (1 byte, 0/1)

Example (set active hours start to 04:00):
SYNC: AA
LEN:  03
TYPE: 02
DATA: 10 F0 00 (key=0x10, value=240 minutes = 04:00)
CRC8: XX
```

**3. ACK (TYPE=0x03)**
```
DATA: [sequence_number (1 byte)]

Example:
SYNC: AA
LEN:  01
TYPE: 03
DATA: 05 (acknowledging packet #5)
CRC8: XX
```

## Decoding Algorithm

### State Machine
```
IDLE → SYNCING → RECEIVING → VALIDATING → DONE/ERROR
```

**1. IDLE**
- Poll light sensor at 64 Hz
- Look for HIGH/LOW transitions
- Measure time between transitions
- If consistent ~50ms transitions detected → SYNCING

**2. SYNCING**
- Decode SYNC byte (0xAA = 10101010)
- Establish bit clock timing
- Adjust sampling phase for optimal detection
- If 8 valid transitions (SYNC byte) → RECEIVING

**3. RECEIVING**
- Decode LEN byte (packet length)
- Decode TYPE byte (packet type)
- Decode DATA bytes (LEN bytes total)
- Decode CRC8 byte
- Buffer all bytes → VALIDATING

**4. VALIDATING**
- Calculate CRC8 over LEN + TYPE + DATA
- Compare with received CRC8
- If match → DONE
- If mismatch → ERROR

**5. DONE**
- Process packet based on TYPE
- Update RTC (time sync)
- Update config (config update)
- Display success message
- Return to IDLE

**6. ERROR**
- Display error message (CRC fail, timeout, invalid sync)
- Clear buffers
- Return to IDLE

### Threshold Calibration
**Automatic calibration on RX start:**
1. Sample light level for 1 second (64 samples)
2. Calculate mean and standard deviation
3. Set threshold = mean (midpoint between light/dark)
4. Set hysteresis = ±10% of std dev (noise rejection)

**Dynamic threshold adjustment:**
- Track ambient light drift during reception
- Re-calibrate threshold every 10 bits
- Adaptive to changing light conditions

### Edge Detection
```c
bool detect_edge(uint16_t current_light, uint16_t threshold, bool *last_state) {
    bool current_state = (current_light > threshold);
    
    if (current_state != *last_state) {
        *last_state = current_state;
        return true;  // Edge detected
    }
    
    return false;  // No edge
}
```

### Manchester Decoding
```c
// Decode Manchester bit from two half-bits
// Returns: 0, 1, or -1 (error)
int8_t decode_manchester_bit(bool first_half, bool second_half) {
    if (!first_half && second_half) {
        return 1;  // LOW → HIGH = 1
    } else if (first_half && !second_half) {
        return 0;  // HIGH → LOW = 0
    } else {
        return -1;  // Invalid transition (error)
    }
}
```

## UX Flow (RX Mode)

### Display States
```
1. RX mode selected (IDLE):
   ┌──────────┐
   │ RX   RDY │  ← Ready to receive
   └──────────┘

2. Calibrating (AUTO):
   ┌──────────┐
   │ RX  CAL. │  ← Auto-calibrating threshold
   └──────────┘

3. Waiting for sync (SYNCING):
   ┌──────────┐
   │ RX SYNC  │  ← Looking for sync pattern
   └──────────┘

4. Receiving data (RECEIVING):
   ┌──────────┐
   │ RX  12b  │  ← Received 12 bytes so far
   └──────────┘

5. Reception complete (DONE):
   ┌──────────┐
   │ RX   OK  │  ← Success, packet valid
   │   TIME   │  ← If time sync packet
   └──────────┘

6. Reception error (ERROR):
   ┌──────────┐
   │ RX  ERR  │  ← CRC fail or timeout
   │   CRC    │  ← Error type
   └──────────┘
```

### Button Controls
- **ALARM button:** Start/stop reception
- **LIGHT button:** Cycle mode (TX ↔ RX)
- **MODE button:** Exit to next face (when not receiving)

### Timeout Handling
- **Sync timeout:** 10 seconds (if no sync pattern detected)
- **Bit timeout:** 500ms (if no transition within expected window)
- **Packet timeout:** 2 minutes (entire packet must complete within 2 min)

## Companion App Requirements

### Screen Flash Implementation (JavaScript)
```javascript
// Flash screen to transmit bit
function flashBit(bit, bitDuration = 100) {
  const halfBit = bitDuration / 2;
  
  if (bit === 0) {
    // 0 = HIGH → LOW
    setScreenBrightness(1.0);  // White
    await sleep(halfBit);
    setScreenBrightness(0.0);  // Black
    await sleep(halfBit);
  } else {
    // 1 = LOW → HIGH
    setScreenBrightness(0.0);  // Black
    await sleep(halfBit);
    setScreenBrightness(1.0);  // White
    await sleep(halfBit);
  }
}

function transmitPacket(data) {
  // SYNC byte (0xAA)
  flashByte(0xAA);
  
  // Packet data
  for (let byte of data) {
    flashByte(byte);
  }
}

function flashByte(byte) {
  for (let i = 7; i >= 0; i--) {
    let bit = (byte >> i) & 0x01;
    flashBit(bit);
  }
}
```

### Screen Brightness Control
- **iOS:** Use CSS `filter: brightness()` or `background-color`
- **Android:** Same as iOS
- **Fullscreen:** Required for best results (no UI elements)
- **Screen wake lock:** Prevent sleep during transmission

### User Instructions
1. Open companion app on phone
2. Select "Send Time Sync" or "Send Config"
3. Tap "START TRANSMISSION"
4. Hold phone screen facing watch (distance: 2-6 inches)
5. Keep screen still during transmission (~10-60 seconds)
6. Wait for "SUCCESS" or "ERROR" message

## Security Considerations

### Packet Validation
- **CRC8 checksum:** Detect corrupted packets
- **Length validation:** Reject oversized packets (>64 bytes)
- **Type validation:** Reject unknown packet types
- **Range validation:** Check timestamp, config values for sanity

### Replay Attack Prevention
- **Timestamp validation:** Reject timestamps too far in past/future (±1 week)
- **Sequence numbers:** Track last received sequence, reject old packets
- **Nonce:** Optional 4-byte nonce in time sync packets

### No Encryption (Phase 2)
- Data transmitted in clear (screen flashing is visible)
- Future: Could add simple XOR obfuscation or challenge-response
- Not critical for time sync, but useful for sensitive config

## Performance Metrics

### Target Specifications
- **Bit error rate (BER):** <1% (99% successful bit reception)
- **Packet success rate:** >95% (1 retry acceptable)
- **Sync time:** <5 seconds (time to detect sync pattern)
- **Total transmission time:**
  - Time sync: ~10 seconds (10 bytes @ 10 bps)
  - Config update: ~20 seconds (15 bytes @ 10 bps)
  - Max packet: ~68 seconds (68 bytes @ 10 bps)

### Error Handling
- **CRC mismatch:** Display "RX ERR CRC", return to IDLE
- **Timeout:** Display "RX ERR TMO", return to IDLE
- **Invalid sync:** Display "RX ERR SYN", return to IDLE
- **Retry:** User must manually restart reception (ALARM button)

## Testing Plan

### Unit Tests
1. Manchester encoder/decoder (all bit patterns)
2. CRC8 calculation (known test vectors)
3. Edge detection (noisy signals, threshold crossing)
4. Packet validation (valid/invalid packets)

### Integration Tests
1. Calibration (different ambient light levels)
2. Full packet RX (time sync, config update, ACK)
3. Error handling (corrupted packets, timeouts)
4. Mode switching (TX ↔ RX)

### Hardware Tests
1. Different phone models (iOS/Android)
2. Different ambient light conditions (dark room, bright office, outdoors)
3. Different distances (2-6 inches)
4. Different angles (perpendicular, 45°, shallow)
5. Battery drain measurement

## Future Enhancements (Phase 3+)

### Higher Bit Rates
- **4-FSK optical:** Use 4 brightness levels (0%, 33%, 67%, 100%)
- **Differential encoding:** More robust to ambient light
- **Target:** 40 bps (4x faster)

### Bidirectional Handshake
- Watch sends ACK after successful RX
- Phone retries if no ACK received
- Sequence numbers for ordering

### Advanced Error Correction
- **Forward Error Correction (FEC):** Reed-Solomon codes
- **Interleaving:** Spread burst errors across packets
- **Target:** 99.9% packet success rate

### HealthKit Integration
- Use optical RX to configure HealthKit sync
- Receive auth tokens from companion app
- Bi-directional: Watch sends sleep data → app → HealthKit

---

## Implementation Priority

**Phase 2a (Foundation):**
1. ✅ Basic RX mode UI (mode switching, display states)
2. ✅ Light sensor polling (64 Hz)
3. ✅ Threshold calibration
4. ✅ Edge detection

**Phase 2b (Decoder):**
1. ⏳ SYNC pattern detection
2. ⏳ Manchester decoding
3. ⏳ Packet buffering
4. ⏳ CRC8 validation

**Phase 2c (Packet Handling):**
1. ⏳ Time sync processing
2. ⏳ Config update processing
3. ⏳ Error handling and display

**Phase 2d (Companion App):**
1. ⏳ Screen flash transmitter (JavaScript)
2. ⏳ Packet encoder
3. ⏳ UI for time sync / config send

**Phase 3 (Advanced):**
1. ⏳ Bidirectional handshake
2. ⏳ FEC and interleaving
3. ⏳ HealthKit integration

---

## References
- Timex Datalink protocol: https://github.com/synthead/timex_datalink_client
- Manchester encoding: https://en.wikipedia.org/wiki/Manchester_code
- Optical VLC: "Mobile Application for Visible Light Communication Systems" (MDPI 2024)
- CRC-8/MAXIM: Polynomial 0x31 (x^8 + x^5 + x^4 + 1)
