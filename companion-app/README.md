# Sensor Watch Companion App

Phase 1 acoustic data receiver for Sensor Watch Pro sleep/circadian tracking.

## Quick Start (iPhone)

### 1. One-Time Setup
1. Open Safari on your iPhone
2. Navigate to `fesk-decoder.html` (via file:// or local server)
3. Tap the **Share** button (square with arrow, bottom center)
4. Scroll down and tap **"Add to Home Screen"**
5. Name it **"Watch Sync"** (or whatever you prefer)
6. Tap **Add**

### 2. Daily Usage
1. Open **Watch Sync** from your home screen (NOT Safari)
2. Tap **START LISTENING**
3. Grant microphone permission (first time only)
4. On watch: Navigate to **Comms face** (CO)
5. Press **ALARM button** to start transmission
6. Hold watch buzzer **2-3 inches from iPhone mic**
   - iPhone mic location varies by model:
     - iPhone 13/14/15: Bottom edge (near Lightning/USB-C port)
     - iPhone 12 and older: Bottom edge
     - Check Apple support for your specific model
7. Watch shows progress: **TX 0%** → **TX 100%** (~35 seconds)
8. Decoder shows packets received + signal strength

## Why "Add to Home Screen"?

iOS Safari restricts background audio processing for security. Adding to home screen:
- ✅ Enables full-screen mode (no browser UI clutter)
- ✅ Reliable microphone access without Safari limitations
- ✅ Stays active during data reception
- ✅ Works offline (no network needed)

## Troubleshooting

### "No audio detected"
- Check iOS Settings → Privacy → Microphone → Watch Sync (ON)
- Restart the app (swipe up from home, close, reopen)
- Try different buzzer-to-mic distance (1-4 inches)

### "Signal weak"
- Remove phone case if thick/metallic
- Quiet room (minimize background noise)
- Orient watch buzzer directly toward mic

### "Permission denied"
- iOS Settings → Privacy & Security → Microphone
- Find your browser/app, toggle ON
- Reload page

## Technical Details

**Protocol:** FESK (Frequency-Encoded Shift Keying) by Eirik S. Morland
- 4-FSK mode: 4 tones (D7/E7/F7#/G7#), 2 bits per symbol
- Text-based protocol (42-character alphabet: a-z, 0-9, space, punctuation)
- Binary data hex-encoded before transmission
- 287 bytes → 574 hex chars
- Built-in CRC8, START/END markers
- ~60 seconds full export (hex encoding overhead)

**Data Exported:**
- 7 nights of sleep data (orientation logs)
- Sleep onset/offset timestamps
- Efficiency, WASO, awakenings
- Light exposure during sleep
- Transmitted as uppercase hex (0-9A-F)

**Phase 1 Limitations:**
- Signal detection only (not full FSK decoding yet)
- Shows audio presence, amplitude
- Packet assembly TODO (requires signal processing library)

**Future Phases:**
- Phase 2: Full FSK decoder + packet reassembly
- Phase 3: HealthKit integration (native iOS app via Capacitor)
- Phase 4: Bidirectional comms (optical TX back to watch)

## Local Server (Alternative to Add-to-Home)

If you prefer running via local server:

```bash
# Python 3
cd companion-app
python3 -m http.server 8000

# Then visit on iPhone Safari:
http://YOUR_MAC_IP:8000/fesk-decoder.html
```

Still works, but "Add to Home Screen" gives better UX.

## Credits

- **chirpy_tx library:** Gabor L Ugray (MIT License)
- **FESK protocol:** Adapted for watch↔phone acoustic comms
- **Web Audio API:** Mozilla MDN, W3C spec
