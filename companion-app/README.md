# FESK DECODER - Sensor Watch Companion App

**Frequency Encoded Sleep Kit** data visualization and decoder.

## Overview

GameBoy DMG-inspired sleep data dashboard. Decodes 287 bytes (574 hex chars) of 7-night sleep metrics from Sensor Watch FESK protocol. Dense information display, retro terminal aesthetic, every pixel counts.

## Features

### Data Decoding
- **Hex → Binary**: 574 hex characters → 287 bytes
- **Binary Parsing**: 7 nights × 41 bytes per night structure
- **Validation**: Checks data integrity and valid flags

### Visualizations

1. **Circadian Score Dashboard**
   - Overall CS (0-100)
   - 5 subscores: SRI, Duration, Efficiency, Compliance, Light
   - Large metric display with GameBoy color coding

2. **7-Day Sleep Timeline**
   - Canvas-based bar chart
   - Duration bars (height = sleep hours)
   - Color-coded by quality (4 shades of green)
   - Grid lines every 2 hours
   - Day labels (Su-Sa)

3. **Night Detail Cards**
   - Per-night drill-down
   - Onset/Offset times
   - Duration, Efficiency, WASO, Awakenings
   - Light exposure metric
   - Quality score bar

### Data Management
- **IndexedDB Storage**: Persistent local storage
- **History Tracking**: All decoded datasets saved with timestamps
- **CSV Export**: Sleep metrics + Circadian Score components
- **JSON Export**: Full structured data with metadata

### UX Features
- **Keyboard Shortcuts**: D=decode, C=clear, E=export, ESC=reset
- **Responsive Design**: Works on iPhone, iPad, desktop
- **PWA-Ready**: Add-to-home-screen support
- **Accessible**: Keyboard navigation, screen reader friendly
- **Test Data**: Built-in sample generator for testing

## Design Specification

### Color Palette (GameBoy DMG)
```
#0f380f - Darkest green (background)
#306230 - Dark green (containers)
#8bac0f - Light green (borders, accents)
#9bbc0f - Lightest green (text, highlights)
```

### Typography
- **Font**: Monospace (Courier New, Consolas, Monaco)
- **Base Size**: 11px (mobile), 12px (desktop)
- **Headings**: 16px (mobile), 20px (desktop)
- **Metrics**: 20-40px bold
- **Line Height**: 1.4 (readability)
- **Character Grid**: Aligned to 8px intervals

### Layout
- **Grid System**: 8px base unit
- **Container Padding**: 8px
- **Frame Borders**: 2px solid
- **ASCII Decorations**: Box drawing characters (┌ ┐ └ ┘)
- **Responsive Breakpoint**: 640px

### Performance
- **Canvas Rendering**: Hardware-accelerated, pixelated rendering
- **Minimal JS**: Vanilla JavaScript, no frameworks
- **IndexedDB**: Async storage, non-blocking UI
- **File Size**: <25KB total (HTML+JS+CSS inline)

## Binary Format

Per-night structure (41 bytes):
```c
typedef struct {
    uint32_t onset_unix;      // [0-3]   Sleep onset (Unix timestamp)
    uint32_t offset_unix;     // [4-7]   Sleep offset (Unix timestamp)
    uint16_t duration_min;    // [8-9]   Duration in minutes
    uint8_t  efficiency_pct;  // [10]    Efficiency percentage (0-100)
    uint16_t waso_min;        // [11-12] Wake After Sleep Onset (minutes)
    uint8_t  awakenings;      // [13]    Number of awakenings
    uint8_t  light_exposure;  // [14]    Light exposure metric (0-255)
    uint8_t  valid;           // [15]    Validity flag (0=invalid, 1=valid)
    uint8_t  padding[25];     // [16-40] Reserved
} circadian_night_data_t;
```

7 nights × 41 bytes = 287 bytes total → 574 hex characters

## Circadian Score Calculation

**Overall CS** (weighted average):
- 25% SRI (Sleep Regularity Index - onset time consistency)
- 25% Duration (optimal 7-9h)
- 25% Efficiency (average efficiency percentage)
- 15% Compliance (valid nights / 7)
- 10% Light (normalized light exposure)

**Quality Score** (per-night):
- 40% Efficiency
- 30% Duration (7-9h optimal)
- 20% Awakenings (fewer is better)
- 10% WASO (lower is better)

## Usage

### Basic Flow
1. **Input**: Paste 574 hex characters into textarea
2. **Decode**: Click "DECODE" button or press D
3. **View**: Circadian Score, 7-day chart, night details appear
4. **Export**: Download CSV/JSON or view history

### Keyboard Shortcuts
- **D**: Decode hex input
- **C**: Clear all data
- **E**: Export to CSV
- **ESC**: Reset interface

### Test Data
Click "TEST DATA" button to generate 7 sample nights with realistic metrics.

## File Structure
```
companion-app/
├── fesk-decoder.html  (Main UI - 23KB)
├── decoder.js         (Data parsing & storage - 8KB)
└── README.md          (This file)

lib/
└── circadian_score.h  (Binary format reference)
```

## PWA Installation

### iPhone/iPad Safari
1. Open fesk-decoder.html in Safari
2. Tap Share button
3. Select "Add to Home Screen"
4. Icon appears on home screen
5. Opens fullscreen (no browser chrome)

### Android Chrome
1. Open in Chrome
2. Menu → "Add to Home Screen"
3. App icon created

## Technical Notes

### Little-Endian Byte Order
All multi-byte values (uint16_t, uint32_t) use little-endian encoding. JavaScript DataView handles conversion automatically with `true` parameter.

### IndexedDB Schema
- **Database**: `FESKData`
- **Store**: `sleepData`
- **Key**: `timestamp` (Date.now())
- **Index**: `timestamp` (non-unique)

### Browser Compatibility
- **Safari iOS 12+**: Full support
- **Chrome/Edge**: Full support
- **Firefox**: Full support
- **IE11**: Not supported (uses modern JS)

### Data Privacy
- **Local Only**: All data stored in browser's IndexedDB
- **No Network**: No external API calls or tracking
- **No Cookies**: Pure client-side application

## Design Philosophy

**Demoscene precision meets health tracking.** Every pixel earned, no corporate jargon, underground polish. Inspired by:
- GameBoy DMG hardware constraints
- BBS terminal interfaces
- Warez NFO file aesthetics
- Demoscene scrollers
- Early PDA applications

**Dense information displays** - maximum data in minimum space. **Retro terminal feel** - monospace fonts, ASCII borders, CRT color palette. **Functional beauty** - aesthetics serve usability.

## Future Enhancements

Potential additions (not yet implemented):
- **Audio Input**: FESK protocol via Web Audio API (mic → decoder)
- **Service Worker**: Offline PWA caching
- **Data Trends**: Multi-week visualization
- **Export Formats**: PDF reports, chart images
- **Settings Panel**: Color themes, metric preferences
- **Sync**: Optional cloud backup (encrypted)

## Credits

- **Design**: frontend-designer agent (demoscene aesthetic)
- **Protocol**: Sensor Watch FESK specification
- **Palette**: Nintendo GameBoy DMG (1989)
- **Philosophy**: Every pixel counts, underground polish

---

**Version 1.0** - Initial release  
**License**: MIT (assumed, confirm with project owner)  
**Contact**: See main Sensor Watch project repository
