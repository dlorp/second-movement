# Optical TX Companion App

Send time sync and config updates to Sensor Watch Pro via phone screen flashing.

## Quick Start

1. Open `optical-tx.html` in your phone browser (iOS Safari or Android Chrome)
2. On watch, navigate to Comms face (CO)
3. Long-press LIGHT button until display shows `RX RDY` (RX mode)
4. Press ALARM button on watch (watch displays `RX SYNC`)
5. On phone, tap "Send Time Sync"
6. **Hold phone screen facing watch** (distance: 2-6 inches)
7. Keep still during transmission (~10 seconds)
8. Watch displays `RX OK` when complete

## How It Works

### Protocol
- **Encoding:** Manchester (self-clocking, error-detecting)
- **Bit rate:** 10 bps (bits per second)
- **Packet format:** `SYNC (0xAA) + LEN + TYPE + DATA + CRC8`
- **CRC checksum:** CRC-8/MAXIM for error detection

### Screen Flashing
- **White = logical HIGH**
- **Black = logical LOW**
- **Manchester encoding:** Each bit has a transition
  - `0` = HIGH → LOW (white → black)
  - `1` = LOW → HIGH (black → white)
- **Bit duration:** 100ms (50ms per half-bit)

### Transmission Times
- **Time sync:** ~10 seconds (10 bytes)
- **Config update:** ~20 seconds (15 bytes)
- **Maximum packet:** ~68 seconds (68 bytes max)

## Packet Types

### 1. Time Sync (TYPE=0x01)
Syncs watch time with phone clock.

**Payload (6 bytes):**
- Timestamp (4 bytes, little-endian): Unix timestamp (UTC)
- Timezone offset (2 bytes, little-endian, signed): Minutes from UTC

**Example:**
```
Current time: 2026-02-16 01:00:00 AKST (-09:00 = -540 minutes)
Timestamp: 1739682000 (Unix)
Timezone: -540 minutes

Packet:
SYNC:  0xAA (10101010 binary)
LEN:   0x06 (6 bytes)
TYPE:  0x01 (time sync)
DATA:  0x90 0x47 0xB1 0x67  (timestamp)
       0x24 0xFE             (timezone: -540)
CRC8:  0xXX (calculated)
```

### 2. Config Update (TYPE=0x02)
Updates watch settings (not yet implemented).

**Config keys:**
- `0x10`: Active Hours Start (2 bytes, minutes since midnight)
- `0x11`: Active Hours End (2 bytes, minutes since midnight)
- `0x12`: Smart Alarm Window (1 byte, minutes)
- `0x13`: Smart Alarm Enabled (1 byte, 0/1)

### 3. ACK (TYPE=0x03)
Acknowledgment packet (future bidirectional sync).

## Tips for Best Results

### Positioning
- **Distance:** 2-6 inches from watch to phone screen
- **Angle:** Screen facing watch, perpendicular if possible
- **Keep still:** Movement can cause missed bits
- **Ambient light:** Works in any lighting (algorithm auto-calibrates)

### Troubleshooting

**"RX ERR" on watch:**
- **CRC mismatch:** Screen moved during transmission → retry
- **Timeout:** Watch stopped receiving → check positioning
- **Sync failed:** Couldn't detect sync pattern → retry, adjust distance

**Slow/choppy flashing:**
- Phone may be throttling → close other apps
- Battery saver mode → disable during transmission
- Low battery → charge phone

**No response from watch:**
- Verify watch is in RX mode (`RX RDY` displayed)
- Verify watch pressed ALARM button before phone transmission
- Check phone screen brightness (should be max)

## Browser Compatibility

### iOS Safari ✅
- Full support
- Add to Home Screen for best experience (prevents interruptions)
- Screen wake lock supported (iOS 16.4+)

### Android Chrome ✅
- Full support
- Screen wake lock supported

### Desktop Browsers ⚠️
- Works, but not recommended (no light sensor on most computers)
- Can use for testing companion app logic

## Technical Details

### Manchester Encoding Example
```
Data byte: 0x5A (01011010 binary)

Bit sequence:
0  1  0  1  1  0  1  0

Manchester:
▁▔ ▔▁ ▁▔ ▔▁ ▔▁ ▁▔ ▔▁ ▁▔
↓  ↑  ↓  ↑  ↑  ↓  ↑  ↓

Screen: black white black white white black white black
        white black white black black white black white
```

### CRC-8/MAXIM
- **Polynomial:** 0x31 (x^8 + x^5 + x^4 + 1)
- **Initial value:** 0x00
- **Reflected:** Yes
- **Final XOR:** 0x00

**JavaScript implementation:**
```javascript
function crc8(data) {
    let crc = 0;
    for (let byte of data) {
        crc ^= byte;
        for (let i = 0; i < 8; i++) {
            if (crc & 0x01) {
                crc = (crc >> 1) ^ 0x8C;  // Polynomial 0x31 reversed
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}
```

## Future Features (Phase 3)

- **Bidirectional sync:** Watch sends ACK after successful RX
- **Retry logic:** Auto-retry failed transmissions
- **Sequence numbers:** Prevent duplicate packets
- **Config updates:** Send Active Hours, alarm settings to watch
- **HealthKit integration:** Use as bridge for sleep data upload

## Safety

- **No sensitive data:** Time sync and config only (no passwords, etc.)
- **No encryption:** Data visible as screen flashes (not critical for time/config)
- **Timestamp validation:** Watch rejects timestamps too far in past/future (±1 week)
- **CRC validation:** Watch rejects corrupted packets

## Development

### Testing
```bash
# Open in browser
open optical-tx.html

# Or use local server (avoids CORS issues)
python3 -m http.server 8000
# Then open: http://localhost:8000/optical-tx.html
```

### Modifying
- `flashBit()`: Change bit timing or encoding
- `flashPacket()`: Change packet structure
- `sendTimeSync()`: Add new packet types
- `crc8()`: Change checksum algorithm

### Adding New Packet Types
1. Define packet type constant (e.g., `0x04`)
2. Create packet builder function
3. Add button to UI
4. Update watch decoder (`comms_rx.c`)

## Credits

**Inspired by:**
- **Timex Datalink (1994):** Original optical watch sync (CRT patterns)
- **Manchester encoding:** IEEE 802.3 Ethernet standard
- **CRC-8/MAXIM:** Industry-standard checksum

**References:**
- [Timex Datalink Protocol](https://github.com/synthead/timex_datalink_client)
- [Manchester Encoding](https://en.wikipedia.org/wiki/Manchester_code)
- [Optical VLC Research](https://www.mdpi.com/2304-6732/11/4/293)

---

**Version:** Phase 2 (TX + RX complete)  
**Last Updated:** 2026-02-16  
**Author:** Diego Perez (dlorp)
