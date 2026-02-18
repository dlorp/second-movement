# FESK Decoder - Visual Description

## What You'll See

Imagine a **GameBoy screen from 1989** displaying your sleep data. That's the vibe.

### Color Experience
The entire interface uses the authentic **GameBoy DMG 4-shade green palette**:
- Deep forest green background (#0f380f)
- Dark moss green panels (#306230)
- Bright lime green borders (#8bac0f)
- Phosphor green text (#9bbc0f)

No gradients. No shadows. Just crisp pixel-perfect monochrome green like you're staring at a classic handheld console screen.

### Layout Flow (Top to Bottom)

#### 1. HEADER
```
        ◆ FESK DECODER ◆
Frequency Encoded Sleep Kit · Sensor Watch
```
- Large monospace title with diamond bullets
- Centered, bold, 2px letter spacing
- Retro terminal energy

#### 2. HEX INPUT FRAME
ASCII-bordered text box with frame title:
```
┌─HEX INPUT─────────────────────────┐
│ [Paste 574 hex characters...]    │
│                                   │
└───────────────────────────────────┘
  ▸ DECODE    ✗ CLEAR    ⚡ TEST
```
- Monospace textarea (dark green background)
- 3 buttons in a row (chunky GameBoy aesthetic)
- ASCII box-drawing characters for borders

#### 3. CIRCADIAN SCORE DASHBOARD
**When decoded, this appears:**

Giant metric in center:
```
┌─CIRCADIAN SCORE──────────────────┐
│                                  │
│      OVERALL CS                  │
│         82                       │  ← 32px bold number
│                                  │
│  SRI   DUR   EFF   CMP   LGT    │
│  85    92    87    100   68     │  ← 20px numbers
└──────────────────────────────────┘
```
- Main score dominates (huge font)
- 5 subscores in grid below
- Each metric in its own bordered box
- Dense but readable

#### 4. 7-DAY SLEEP TIMELINE
**Canvas-rendered bar chart:**
```
┌─7-DAY SLEEP TIMELINE─────────────┐
│                                  │
│ 10h ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─  │
│  8h ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─  │
│        ███     ███     ███       │  ← Quality-colored bars
│  6h ─ ─ ███ ─ ─ ███ ─ ─ ███ ─  │
│        ███     ███     ███       │
│  4h ─ ─ ███ ─ ─ ███ ─ ─ ███ ─  │
│        7.2h    7.8h    7.6h      │  ← Duration labels
│         Su      Mo      Tu       │  ← Day labels
└──────────────────────────────────┘
```
- 7 vertical bars (one per night)
- Bar height = sleep duration
- Bar color = quality score (4 shades of green)
- Grid lines every 2 hours
- Pixelated rendering (crisp edges)
- Feels like a DOS/Amiga demo

#### 5. NIGHT DETAILS
**Scrollable list of night cards:**
```
┌─NIGHT DETAILS────────────────────┐
│                                  │
│ ┌────────────────────────────┐  │
│ │ Night 1: Sun Feb 09        │  │
│ │ 23:15 → 06:27 · 7.2h       │  │
│ │ EFF: 89% · WASO: 18m       │  │
│ │ AWK: 2 · LIGHT: 142        │  │
│ │ QUAL: 85                   │  │
│ │ ███████████████████████    │  │ ← Quality bar
│ └────────────────────────────┘  │
│                                  │
│ ┌────────────────────────────┐  │
│ │ Night 2: Mon Feb 10        │  │
│ │ 22:45 → 06:33 · 7.8h       │  │
│ │ ...                        │  │
└──────────────────────────────────┘
```
- Each night in its own bordered card
- Dense text layout (every line counts)
- Quality bar at bottom (visual feedback)
- Hover effect: inverts colors (green bg → text)

#### 6. EXPORT CONTROLS
```
┌─EXPORT───────────────────────────┐
│  ↓ CSV    ↓ JSON    ⌂ HISTORY   │
└──────────────────────────────────┘
```
- 3 chunky buttons
- Glyphs instead of words (space-efficient)

#### 7. FOOTER
```
[D]ECODE · [C]LEAR · [E]XPORT · ESC=RESET
```
- Tiny 8px font
- Keyboard shortcuts reminder
- Terminal hacker aesthetic

---

## The Feel

### When You First Load
- Deep green screen
- Blinking cursor vibes (though no actual cursor)
- Empty text box waiting for hex input
- Feels like booting a vintage terminal

### When You Decode Data
- All sections appear instantly (no slow animations)
- Chart renders with pixelated precision
- Numbers snap into place
- Like watching a demoscene intro unfold

### When You Interact
- Buttons respond with crisp 0.1s transitions
- Hover inverts colors (GameBoy press effect)
- Active press shifts button 1px (tactile feel)
- Keyboard shortcuts work immediately

### Mobile Experience
- Scales perfectly to iPhone width
- Touch-friendly button sizes (48px min)
- No pinch-to-zoom needed
- Feels native (PWA fullscreen mode)
- Add-to-home-screen = instant app

---

## Aesthetic Comparisons

**It's like:**
- ✓ GameBoy playing a sleep tracker ROM
- ✓ BBS login screen for your circadian rhythm
- ✓ Warez NFO file came to life
- ✓ Commodore 64 health dashboard
- ✓ Demoscene intro for sleep optimization

**It's NOT like:**
- ✗ Apple Health (too corporate, too white)
- ✗ Fitbit dashboard (too colorful, too rounded)
- ✗ Google Material Design (too fluffy)
- ✗ Modern minimalism (too much whitespace)
- ✗ Neumorphism (too soft, too 2020)

---

## Why This Design Works

### 1. Information Density
Every pixel justified. No decorative elements. Maximum data in minimum space. You can see **all your sleep data** without scrolling (on desktop).

### 2. Retro Constraint = Focus
4-color palette forces clarity. Can't hide bad UX with pretty gradients. Structure must be perfect.

### 3. Terminal Aesthetics = Power User Respect
Monospace fonts, keyboard shortcuts, no hand-holding. Treats user as intelligent.

### 4. Underground Polish
Rough edges are intentional, but everything works flawlessly. Like a well-coded demoscene production.

### 5. Nostalgia Meets Function
Looks like 1989, works like 2024. PWA, IndexedDB, responsive design—all invisible.

---

## Technical Beauty

### What You Don't See:
- **No framework bloat** - Pure vanilla JS (8KB)
- **No external dependencies** - Self-contained
- **No build step** - Just HTML + JS
- **No tracking** - Pure local storage
- **No loading spinners** - Everything instant

### What You Do Feel:
- **Snappy** - 60fps chart rendering
- **Reliable** - IndexedDB persistence
- **Accessible** - Keyboard-first design
- **Portable** - Works offline after first load

---

## The "Demoscene Precision" Details

### Alignment
Every element aligns to **8px grid**. No half-pixels. No fuzzy anti-aliasing. Sharp edges.

### Typography
**Monospace everywhere.** Character grid alignment. Glyphs as UI elements (│ ─ ▸ ◆). Like coding a UI in ASCII art.

### Color Usage
**No color mixing.** Pure palette colors only. No rgba() transparency tricks. Solid fills.

### Animation
**Minimal.** Only 0.1s button transitions. No bouncing, no fading, no easing curves. Instant state changes.

### Performance
**Frame budget awareness.** Canvas renders once, not 60fps. IndexedDB queries async. No janky scrolling.

---

## User Stories

### "I'm a sleep optimization nerd"
→ **Dense metrics, CSV export, keyboard shortcuts.** All the data, all the control.

### "I just want to see if I'm sleeping well"
→ **Big green number at top.** CS score = instant feedback. Don't need to understand subscores.

### "I'm on my iPhone in bed"
→ **PWA install, works offline, touch-friendly.** Add to home screen = bedside app.

### "I'm nostalgic for retro computing"
→ **GameBoy palette, ASCII borders, terminal vibes.** This UI is the data visualization.

### "I'm a developer checking out the code"
→ **View source = learn.** Clean vanilla JS, well-commented, no minification.

---

## Summary

This is a **health dashboard that respects your intelligence and screen space**. It doesn't baby you with rounded corners and pastel colors. It shows you **287 bytes of binary sleep data** transformed into **crisp monochrome visualization** that feels like operating a precision instrument.

Every design choice serves the data. Every pixel justified. Every interaction considered.

**Demoscene aesthetics meet sleep science.**  
**Underground polish meets health optimization.**  
**1989 visuals meet 2024 functionality.**

That's the FESK Decoder.
