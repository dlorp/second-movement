# Second Movement - Enhancement Roadmap

**Last Updated:** 2026-02-18  
**Source:** Dogfood feedback + dlorp feature requests

---

## 🎯 Priority 1: Font Consistency

**Issue:** Companion app missing Google Fonts link for Share Tech Mono  
**Current state:**
- Builder: ✅ Loads Share Tech Mono from Google Fonts
- Companion app: ❌ CSS specifies it but doesn't load it (falls back to IBM Plex Mono)

**Fix required:**
```html
<!-- Add to companion-app/index.html <head> -->
<link rel="preconnect" href="https://fonts.googleapis.com">
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
<link href="https://fonts.googleapis.com/css2?family=Share+Tech+Mono&display=swap" rel="stylesheet">
```

**Also update CSP:**
```html
<meta http-equiv="Content-Security-Policy" content="default-src 'self'; style-src 'self' 'unsafe-inline' fonts.googleapis.com; font-src fonts.gstatic.com; ...">
```

**Files to modify:**
- `companion-app/index.html` (lines 1-20, add font links and update CSP)

---

## 🎯 Priority 2: Builder UX Improvements

### 2.1 GitHub Token Validation
**What:** Real-time token validation with visual feedback  
**Why:** Users don't know if token works until build dispatch fails  

**Implementation:**
```javascript
async function validateToken(token) {
    const resp = await fetch('https://api.github.com/user', {
        headers: { 'Authorization': 'Bearer ' + token }
    });
    return resp.ok;
}

// Add token status indicator to UI
<div class="token-status">
    <span id="tokenIndicator">⬤</span>
    <span id="tokenLabel">NO TOKEN</span>
</div>

// Real-time validation on input
document.getElementById('githubToken').addEventListener('input', async e => {
    const token = e.target.value;
    if (token.length > 0) {
        const valid = await validateToken(token);
        updateTokenStatus(valid);
    }
});
```

**Files to modify:**
- `builder/index.html` (~line 1600, add validation logic)

### 2.2 Flash Usage Warning
**What:** Estimate flash usage and warn before build if too many faces selected  
**Why:** Prevent wasted build cycles when firmware exceeds 256KB limit  

**Implementation:**
```javascript
// Add estimated_flash_bytes to face_registry.json entries
{
    "id": "clock_face",
    "estimated_flash_bytes": 4096,
    ...
}

// Calculate total estimated flash
const estimatedFlashKB = state.activeFaces
    .map(id => registry.faces.find(f => f.id === id)?.estimated_flash_bytes || 6000)
    .reduce((sum, bytes) => sum + bytes, 0) / 1024;

const flashLimitKB = 240; // Leave 16KB margin

if (estimatedFlashKB > flashLimitKB) {
    showStatus('⚠️ TOO MANY FACES - Reduce to ~' + Math.floor(flashLimitKB / 6) + ' faces', 'warning');
    document.getElementById('buildBtn').disabled = true;
}
```

**Files to modify:**
- `builder/face_registry.json` (add `estimated_flash_bytes` to all faces)
- `builder/index.html` (~line 1900, add flash calculation to renderActiveFaces)

### 2.3 Visual Sleep Window Indicator
**What:** Horizontal bar showing active vs sleep hours visually  
**Why:** Makes Active Hours range easier to understand at a glance  

**Implementation:**
```html
<div class="ah-visual-range">
    <div class="ah-bar">
        <div class="ah-sleep-segment" style="width: 16.67%;"></div>  <!-- 00:00-04:00 -->
        <div class="ah-active-segment" style="width: 79.17%;"></div> <!-- 04:00-23:00 -->
        <div class="ah-sleep-segment" style="width: 4.17%;"></div>   <!-- 23:00-00:00 -->
    </div>
</div>
```

**CSS:**
```css
.ah-bar {
    height: 16px;
    display: flex;
    border: 1px solid var(--color-border);
    margin: 8px 0;
}
.ah-active-segment { background: var(--color-amber-dim); }
.ah-sleep-segment { background: var(--color-bg-recessed); }
```

**Files to modify:**
- `builder/index.html` (~line 600, add visual indicator + update on change)

### 2.4 Cross-Link Builder ↔ Companion App
**What:** Add navigation link between builder and companion app  
**Why:** Users don't know companion app exists  

**Implementation:**
```html
<!-- Add to builder -->
<div class="section">
    <div class="section-title">◆ OPTICAL COMMS</div>
    <p>Time sync via screen flash (Timex DataLink-style)</p>
    <a href="../companion-app/index.html" class="btn-link">OPEN COMPANION APP →</a>
</div>

<!-- Add to companion app -->
<div class="nav-link">
    <a href="../builder/index.html">← BACK TO FIRMWARE BUILDER</a>
</div>
```

**Files to modify:**
- `builder/index.html` (~line 800, add new section)
- `companion-app/index.html` (~line 150, add nav link)

---

## 🎯 Priority 3: Asset Integration

**Asset Library:** https://drive.google.com/drive/folders/1qjhACfUvawpuQdpV8hdHVglGHIFC6UMu

**Available assets:**
- Wireframe vectors (30 pack)
- Glitch textures (Abstract Outline Glitch)
- Retro patterns (60s/70s geometric)
- Collage textures (750+ vintage assets)
- Scan line textures (Frequencies Scan)
- Holographic papers
- Fonts (various retro/technical)

**Potential uses:**

### 3.1 Face Preview Images
Use wireframe vectors or low-poly renders for face thumbnails in builder.

**Example workflow:**
1. Export SVG wireframes from asset pack
2. Generate PNG previews (480x320 or similar)
3. Add to `builder/previews/` directory
4. Reference in face_registry.json:
```json
{
    "id": "clock_face",
    "preview_image": "./previews/clock_face.png"
}
```

### 3.2 Background Textures
Use scan line/glitch textures for subtle background variation.

**Example:**
```css
body::after {
    content: '';
    position: fixed;
    inset: 0;
    background-image: url('./assets/scan-texture.png');
    opacity: 0.05;
    mix-blend-mode: screen;
    pointer-events: none;
}
```

### 3.3 Custom Fonts
Explore retro/technical fonts from collection for headings or emphasis.

**Requirements:**
- Must maintain readability at 11px
- Must support monospace layout
- Consider loading as woff2 for performance

---

## 🎯 Priority 4: Apple Health Integration

**Feature:** Export sleep tracking data to Apple Health  
**Timeline:** Future enhancement (post-MVP)  

**Architecture:**

### 4.1 Data Flow
```
Watch (Sleep Tracker) 
    ↓ (optical TX or USB serial)
Companion App (Web) 
    ↓ (WebUSB or screen flash RX)
iOS App (Swift + HealthKit) 
    ↓
Apple Health
```

### 4.2 Data Format
Sleep data export from watch:
```c
// Packet structure for sleep data export
typedef struct {
    uint8_t  len;              // Packet length
    uint8_t  type;             // PACKET_TYPE_SLEEP_DATA
    uint32_t date;             // Unix timestamp (midnight of sleep date)
    uint16_t duration_min;     // Total sleep duration (minutes)
    uint8_t  efficiency;       // Sleep efficiency (0-100%)
    uint8_t  awakenings;       // Number of awakenings
    uint16_t waso_min;         // Wake After Sleep Onset (minutes)
    uint8_t  sleep_score;      // Overall sleep score (0-100)
    uint8_t  crc8;             // CRC-8/MAXIM checksum
} sleep_data_packet_t;
```

### 4.3 HealthKit Integration
iOS app would use HealthKit categories:
```swift
import HealthKit

// Sleep analysis sample
let sleepType = HKCategoryType.categoryType(forIdentifier: .sleepAnalysis)!

let sleepSample = HKCategorySample(
    type: sleepType,
    value: HKCategoryValueSleepAnalysis.asleep.rawValue,
    start: bedTime,
    end: wakeTime
)

healthStore.save(sleepSample) { success, error in
    // Handle result
}
```

### 4.4 Implementation Phases

**Phase 1: Web Companion RX Enhancement**
- Add sleep data packet type to optical RX protocol
- Parse sleep data packets in companion app
- Display in web UI with export option (CSV/JSON)

**Phase 2: iOS App Development**
- Create native iOS app with HealthKit integration
- Implement optical RX or WebUSB serial connection
- Request HealthKit write permissions
- Auto-sync sleep data to Health app

**Phase 3: Circadian Data**
- Export 7-day rolling Circadian Score metrics
- Map to HealthKit HKQuantityType identifiers where applicable
- Consider custom HealthKit categories for CS subscores

### 4.5 Privacy Considerations
- Sleep data stays local on watch until explicit export
- User controls sync timing (manual or auto)
- No cloud servers (watch → companion → iOS → Health)
- HealthKit permission granularity (sleep only, no other health data)

### 4.6 Technical Challenges
**Optical TX bandwidth:**
- Current: 16 bps (slow for bulk data export)
- One sleep record: ~96 bits = 6 seconds TX time
- 7 days of data: ~42 seconds TX time
- **Acceptable** for manual sync, but consider USB serial for auto-sync

**WebUSB compatibility:**
- Not supported on iOS Safari (iOS doesn't support WebUSB)
- Would require native iOS app, not web-only solution
- Optical TX remains viable fallback for web companion

**Alternative: BLE (future)**
- Sensor Watch doesn't have BLE hardware
- Would require hardware revision (SAM L22 → SAM L21 + BLE module)
- Out of scope for current watch

**Recommended path:**
1. Start with optical TX for manual export
2. Build native iOS app with optical RX
3. Consider USB serial if optical proves too slow in practice

---

## 🎯 Priority 5: Advanced Features (Backlog)

### 5.1 Face Preview Emulator
Embed emscripten-compiled watch simulator in builder for live preview.

**Implementation:**
- Use existing `emmake make` build target
- Iframe embed in builder UI
- Pass config to emulator via URL params
- Real-time preview without GitHub Actions build

### 5.2 Build History
Track last N builds in localStorage with one-click rebuild.

**Features:**
- Show timestamp, board, display, face count
- Build status indicator (✓ success, ✗ failed)
- Load config from history
- Delete old builds

### 5.3 Face Category Icons
Visual icons for face categories (clock, sensor, complication, etc.)

**Use assets from Drive:**
- Wireframe vectors for category badges
- Maintain retro aesthetic

### 5.4 Firmware Diff View
When loading template or changing config, show visual diff.

**Example:**
```
CHANGES FROM CURRENT:
+ Added: moon_phase_face, beats_face
- Removed: fast_stopwatch_face
~ Modified: Active Hours 04:00-23:00 → 06:00-22:00
~ Modified: LED Green 15 → 8
```

### 5.5 Settings Registry Auto-Gen
Generate settings_registry.json from movement_config.h parsing (keep single source of truth).

---

## 📋 Implementation Checklist

### Immediate (This Week)
- [ ] Fix companion app font loading (Priority 1)
- [ ] Add GitHub token validation (Priority 2.1)
- [ ] Cross-link builder ↔ companion (Priority 2.4)

### Short-term (Next 2 Weeks)
- [ ] Flash usage warning (Priority 2.2)
- [ ] Visual sleep window indicator (Priority 2.3)
- [ ] Survey asset library for wireframe vectors (Priority 3.1)

### Mid-term (Next Month)
- [ ] Generate face preview images (Priority 3.1)
- [ ] Implement build history (Priority 5.2)
- [ ] Design Apple Health integration spec (Priority 4)

### Long-term (Future)
- [ ] Native iOS app with HealthKit
- [ ] Emulator integration (Priority 5.1)
- [ ] Firmware diff view (Priority 5.4)

---

## 🎨 Design Guidelines

**Maintain aesthetic:**
- Amber CRT glow (#FFB000, #CC8800, #7A5200)
- Share Tech Mono everywhere
- Scanline overlays
- Glyphs over emoji
- Terminal-native UI elements
- Low poly / wireframe when using assets

**Performance:**
- Keep page loads <150 KB
- Lazy-load preview images
- Inline critical CSS
- Use woff2 for custom fonts

**Accessibility:**
- Maintain ARIA attributes
- 44px touch targets
- Keyboard navigation support
- Screen reader compatibility

---

*Roadmap maintained by: Lorp Bot*  
*Asset library: https://drive.google.com/drive/folders/1qjhACfUvawpuQdpV8hdHVglGHIFC6UMu*
