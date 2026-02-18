# DESIGN SYSTEM — COLD WAR TERMINAL AESTHETIC
## Second Movement / Companion App
### Revision 1.0 — February 2026

> *"The machine didn't notify you, didn't scroll infinitely, didn't fight for your attention. It just waited — ready when you were."*

---

## Table of Contents

1. [Inspiration Sources](#1-inspiration-sources)
2. [Color Palette](#2-color-palette)
3. [Typography](#3-typography)
4. [Layout System](#4-layout-system)
5. [Component Patterns](#5-component-patterns)
6. [Motion & Effects](#6-motion--effects)
7. [Implementation Notes](#7-implementation-notes)
8. [Anti-Patterns](#8-anti-patterns)

---

## 1. Inspiration Sources

### 1.1 Soviet / Eastern Bloc Systems

**БЭСМ-6 (BESM-6) — Soviet Supercomputer Terminals**
*~1966–1994*  
The BESM-6 was the USSR's flagship scientific computer. Operator terminals were phosphor monochrome CRTs connected via coaxial — green phosphor P31 variants. Operators worked in low-ambient-light conditions. Interfaces were purely alphanumeric with no graphical ornamentation. Status was communicated via uppercase text columns, separator lines of repeated `═` or `─` characters, and abbreviated field labels (3–5 chars). Classified military derivatives drove the USSR's missile defense networks. Visual archetype: maximum density, no decorative elements whatsoever.

**ЕС ЭВМ (ES EVM) — Soviet IBM-Compatible Mainframe Terminals**
*~1969–1991*  
The USSR's reverse-engineered IBM S/360 clone ecosystem. Terminals resembled IBM 3270 visually — monochrome green, block-mode interaction, fixed-pitch displays at 80×24 characters. The ЕС-7920 display matched the IBM 3278's green-on-black with identical field attribute highlighting conventions (bright for selected, blinking for alert, underline for input fields).

**ЦУП (TsUP) — Soviet Mission Control, Kaliningrad (Korolev)**
*~1960–present*  
The Soviet/Russian equivalent of NASA's Houston MCC. Control room operators sat at consoles with multiple monochrome CRTs per station. Key visual features: world maps rendered as ASCII grid overlays with labeled geographical reference points, orbital track overlays in brighter phosphor intensity, time-synchronized counters in HH:MM:SS format across every terminal, and dedicated "hot" status lights (physical, not on-screen) in amber/red/green.

**БРВИТ / Broadcast Control Terminals**
*Soviet TV broadcast infrastructure, 1960s–1980s*  
Broadcast control rooms used dense multi-row button panels with backlit labels, paired with vector-scan oscilloscope displays for waveform monitoring. Text panels were amber phosphor (easier on operators during long shifts than green). Key UI pattern: labeled button grids in grouped clusters of 4–8, separated by physical gaps or dividers. Each button had a state lamp — off (dark), ready (amber dim), active (amber bright), fault (blinking).

**Soviet Air Defense — ARGON / A-35 System Terminals**
*~1971–present*  
The A-35 Moscow ABM (anti-ballistic missile) system used dedicated ARGON computers with operator displays. Radar return data was shown as vector phosphor dots on dark background — the familiar SAGE-style "radar sweep" aesthetic. Target designations were letter-number codes. No color. Brightness = priority. Blinking = threat. The world map background was a static etched grid, not a raster image.

---

### 1.2 Western Cold War Equivalents

**SAGE — Semi-Automatic Ground Environment**
*IBM / MITRE / MIT Lincoln Lab, 1954–1963*  
The most influential military HMI of the Cold War era. The AN/FSQ-7 computer drove 27 Direction Centers across North America. Each operator station had a circular radar display (P7 long-persistence blue-white phosphor), plus rectangular text status panels (green P31). Key innovations: the first light gun interaction (operators selected tracks by touching the phosphor dot with a light pen), and the first real-time digital status display. Visual grammar: circular radar sweep as focal element, alphanumeric tracks labeled with 3-letter identifiers, the entire UI operating on a black field.

**Apollo Mission Control — MCC-H Houston**
*NASA / MIT Instrumentation Lab, 1965–1972*  
The most photographed Cold War-era control room. Green console cabinets (U.S. government seafoam green, ~Munsell 5BG 5/2). Each console had one or two 9" monochrome CRT displays — amber or green phosphor depending on the specific build lot. The famous "big board" rear projection displays showed world map with vehicle track, ground station status, and trajectory data. Key typography: all-caps, no lowercase anywhere. Fixed-pitch courier-style on CRTs. Condensed hand-lettered labels on physical switches. Color-coded by department (not individual screen colors — the console cabinet colors coded the role: FIDO/GUIDO/RETRO/GNC etc.).

**NORAD / DEW Line Consoles**
*Cheyenne Mountain Complex, 1958–present*  
Deep-underground hardened facility. CRT displays used P7 long-persistence blue-white phosphor for radar (enabling sweeps to persist visually). Secondary text status panels used green P31. Key visual: the famous "missile attack" world map display — a Mercator projection with polar region emphasis (Soviet ICBM trajectories cross the pole), impact footprint overlays, and time-to-impact countdown in red physical numbers. Operators wore headsets; stations were ergonomically tilted. Physical button panels used mil-spec labeled illuminated pushbuttons in aluminum housings.

**IBM 2250 Graphic Display Unit**
*IBM, 1964–1970*  
Vector-scan graphics terminal. The 2250 used point-to-point line drawing with light pen. Key aesthetic: crisp vector lines on pure black, no raster artifacts, a grid-based compositional approach to UI because vectors cost compute time per segment. Menus were rendered as boxes. Labels were terse. Everything was white-on-black vector.

**IBM 3270 / 3278 / 3279 Family**
*IBM, 1971–1990s*  
The workhorse mainframe terminal. 3270 (green phosphor P31), 3278 (improved green CRT), 3279 (first color model — 8 colors: green, red, blue, yellow, white, pink, turquoise, default-green). The color model is instructive: IBM's color semantics for business terminals became a de-facto standard:
- Green: normal text
- Red: protected/error fields  
- Blue: low intensity (dim/secondary)  
- Yellow/Amber: output fields, data values  
- White: high-intensity emphasis  
- Pink: header labels  
- Turquoise: input fields  
This 8-color grammar — rooted in utility not aesthetics — is the grandfather of modern dashboard color conventions.

**Early TV Broadcast Control Rooms (BBC / NBC)**
*1950s–1970s*  
BBC and NBC broadcast control rooms used dense analog VU meter walls, rack-mounted oscilloscopes, and labeled button panels. Key aesthetic: rows of backlit buttons in muted olive/grey housings, with key indicators in amber or red. Typography on panels: Helvetica-adjacent stencil lettering, condensed, all-caps, white on dark surfaces. Waveform monitors as secondary displays.

---

### 1.3 Modern Implementations & References

**Cool Retro Term** — Terminal emulator with exact CRT profiles (Default Amber, IBM 3278, Green Scanlines, IBM DOS). Source for authentic color calibration.

**eiDotter** — A CSS design system explicitly modeled on DOS/CGA amber phosphor. Has real hardware-derived color tokens.

**DEFCON (Introversion Software, 2006)** — The most authentic "cold war display" game aesthetic. Key design choices: world map on black with vector country outlines, white-on-black icons for military units, minimal color (3–4 states max), no texture, no gradients. Game UI overlays use the SAGE radar paradigm.

**Hacknet** — Terminal-based hacking sim. Clean green-on-black, scanline texture as the only effect, monospace throughout, zero decorative elements.

**TIS-100, EXAPUNKS (Zachtronics)** — Authentic fixed-pitch assembly language interfaces. Key reference for information density and operator-facing text design.

**ISA-101 Standard** — International industrial HMI standard. Directly inspired by military/nuclear plant control room ergonomics. Core principle: the neutral state should be *calm* (gray, low-saturation). Alert states *stand out* by contrast. Color is a signal, not decoration.

---

## 2. Color Palette

### 2.1 Phosphor Science → Exact Hex Values

Real phosphor displays have distinct spectral characteristics. The two primary phosphors used in Cold War-era terminals were:

**P31 Phosphor (Green)** — standard green terminals (VT100, IBM 3270, Soviet ЕС-7920)  
The emission peak is ~530nm (yellow-green). At full brightness it appears almost lime, not deep green.

**P3 / P7 Phosphor (Amber)** — preferred for long-duration operator use  
Emission peak ~580–590nm (orange-amber). Warmer than neon orange, softer than pure yellow.

---

### 2.2 Primary Palette — Amber Variant (Recommended)

Preferred for the Cold War / Soviet military aesthetic — amber is the operator console variant, chosen specifically to reduce eye strain for 8-hour shifts. More period-accurate for command-and-control contexts than green.

```
Background (chassis black, not pure black):
  --color-bg:           #0B0900   /* warm near-black, slight sepia cast */
  --color-bg-elevated:  #130F07   /* panel/card surface above bg */
  --color-bg-recessed:  #060500   /* input wells, inactive areas */

Amber Phosphor Scale:
  --color-amber-hot:    #FFD060   /* absolute maximum brightness, white-core */
  --color-amber-bright: #FFB000   /* primary text, active elements (P3 peak) */
  --color-amber-mid:    #CC8800   /* secondary text, borders, dividers */
  --color-amber-dim:    #7A5200   /* inactive/disabled, placeholder */
  --color-amber-trace:  #3D2900   /* faint grid lines, background texture */

State Colors (monochromatic — brightness shifts only, not hue shifts):
  --color-normal:       #CC8800   /* default/nominal — mid amber */
  --color-active:       #FFB000   /* interactive/selected — bright amber */
  --color-armed:        #FFD060   /* armed/ready-to-fire — hot amber */
  --color-alert:        #FF6B00   /* fault/warn — orange shift (still amber family) */
  --color-critical:     #FF3300   /* only for genuine emergencies — warm red */
  --color-offline:      #3D2900   /* unpowered/inactive — trace amber */

Neutral / Structural:
  --color-border:       #7A5200   /* panel separators, box borders */
  --color-border-active:#FFB000   /* focused/selected panel border */
  --color-separator:    #3D2900   /* between sections (subtle) */
```

### 2.3 Alternate Palette — Green Variant (SAGE / VT100 Reference)

Use this if you want the "hacker terminal / SAGE radar" aesthetic instead of Soviet operator aesthetic.

```
Background:
  --color-bg:           #030D03   /* near-black with green cast */
  --color-bg-elevated:  #0A1A0A
  --color-bg-recessed:  #020802

Green Phosphor Scale:
  --color-green-hot:    #CCFF66   /* bloom/glow center */
  --color-green-bright: #33FF00   /* primary text (P31 emission peak) */
  --color-green-mid:    #00BB00   /* secondary text, borders */
  --color-green-dim:    #006600   /* inactive/disabled */
  --color-green-trace:  #002B00   /* faint grid, background */

State Colors:
  --color-normal:       #00BB00
  --color-active:       #33FF00
  --color-armed:        #CCFF66
  --color-alert:        #FFFF00   /* yellow alert — classic military */
  --color-critical:     #FF4400
  --color-offline:      #002B00
```

### 2.4 IBM 3279 Extended Color Reference

For multi-state dashboards, the IBM 3279 color grammar is battle-tested:

| Purpose | IBM Name | Recommended Hex |
|---------|----------|-----------------|
| Normal text | Green | `#33FF00` or `#FFB000` (per palette) |
| Errors, protected | Red | `#FF3300` |
| Low-intensity, secondary | Blue | `#0055CC` |
| Data values, output fields | Yellow | `#FFB000` (amber) |
| Emphasis, selected | White | `#E8E8D0` (warm white) |
| Headers, labels | Pink/Magenta | `#FF88CC` |
| Input fields | Turquoise | `#00BBAA` |

**Note:** Only use the extended palette (blue/pink/turquoise) for highly complex multi-data-type displays. For 80% of use cases, stick to the monochromatic amber scale.

---

## 3. Typography

### 3.1 Font Stack

```css
/* Primary: monospace terminal */
--font-terminal: 'Share Tech Mono', 'IBM Plex Mono', 'Courier New', 'Lucida Console', monospace;

/* Secondary: condensed headers */
--font-condensed: 'B612 Mono', 'Space Mono', 'VT323', monospace;

/* Fallback system */
--font-mono-system: ui-monospace, 'Cascadia Mono', 'Menlo', 'Monaco', monospace;
```

**Preferred Google Fonts (load order):**
1. `Share Tech Mono` — closest to authentic terminal screen rendering. Slightly rough, even stroke width.
2. `Space Mono` — more designed, good for headers/labels.
3. `VT323` — pixel-perfect retro feel, use sparingly (it reads as "game" not "military").

**Avoid:** JetBrains Mono, Fira Code, any font with coding ligatures. They read as "developer tool" not "operator console."

### 3.2 Type Scale

All sizes reference character cell alignment. Use multiples of the character height.

| Role | Size | Weight | Letter-Spacing | Transform |
|------|------|--------|----------------|-----------|
| System header (app name) | 14px | 700 | 0.25em | uppercase |
| Panel label (frame title) | 11px | 400 | 0.15em | uppercase |
| Primary data / values | 12px | 400 | 0.05em | uppercase |
| Large metric display | 28px–48px | 700 | 0em | uppercase |
| Sub-metric values | 18px | 400 | 0em | uppercase |
| Status bar text | 10px | 400 | 0.1em | uppercase |
| Button labels | 10px | 700 | 0.15em | uppercase |
| Footnote / keyboard hints | 9px | 400 | 0.08em | uppercase |
| Input field text | 12px | 400 | 0.02em | uppercase |

**Critical rule:** NO lowercase text in any structural element. Lowercase only in: user-generated content, long-form descriptions, or documentation panels. The moment lowercase appears in a status bar or button, the aesthetic breaks.

### 3.3 Typography Rules

```css
/* All text elements */
font-family: var(--font-terminal);
text-transform: uppercase;
font-variant-numeric: tabular-nums;    /* essential for aligned data columns */
-webkit-font-smoothing: none;          /* disable antialiasing for sharp look */
font-smooth: never;

/* Data columns must align */
font-feature-settings: "tnum" 1;      /* tabular numbers — same-width digits */

/* No ligatures */
font-feature-settings: "liga" 0, "calt" 0;

/* Letter spacing context-dependent */
.status-label  { letter-spacing: 0.15em; }
.data-value    { letter-spacing: 0.02em; }
.panel-title   { letter-spacing: 0.2em;  }
.footer-hint   { letter-spacing: 0.1em;  }
```

---

## 4. Layout System

### 4.1 Grid & Spacing

Everything aligns to a **8px base unit** (1 character cell ≈ 8px wide for most terminal fonts at 12px size). Spacing in all directions is multiples of 8px only.

```
Base unit (u): 8px

Spacing scale:
  0.5u =  4px   (tight, within-component)
  1u   =  8px   (standard gap)
  2u   = 16px   (panel padding)
  3u   = 24px   (section separation)
  4u   = 32px   (major zone separation)

Container widths:
  Full terminal: 960px max (80 chars × 12px ≈ 960px)
  Compact:       640px max (80 chars × 8px)
  Minimal:       480px
```

### 4.2 Zone Anatomy

```
┌───────────────────────────────────────────────────────────┐  ← SYSTEM BAR (32px)
│ [CALLSIGN]  [SUBSYSTEM]  ░░░░░░░  UTC: 00:00:00  [STAT] │
├───────────────────────────────────────────────────────────┤
│  ┌─[ZONE LABEL]──────────────────────────────────────┐  │  ← STATUS BAND (opt, 24px)
│  │ STATUS  ·  STATUS  ·  STATUS  ·  STATUS           │  │
│  └──────────────────────────────────────────────────────┘  │
├───────────────────────────────────────────────────────────┤
│                                                           │  ← MAIN VIEWPORT
│   Primary data display area                              │    (flex grow)
│   Radar / map / waveform / metric grid                   │
│                                                           │
├───────────────────────────────────────────────────────────┤
│  ┌─PANEL─┐  ┌─PANEL─┐  ┌─PANEL─┐  ┌─PANEL────────┐   │  ← CONTROL PANELS
│  │  CTL  │  │  CTL  │  │  CTL  │  │ FUNCTION GRP │   │    (sidebar or bottom)
│  └───────┘  └───────┘  └───────┘  └──────────────┘   │
├───────────────────────────────────────────────────────────┤
│  [F1] DECODE  [F2] CLEAR  [F3] EXPORT   ESC=RESET        │  ← KEYBOARD ROW (28px)
└───────────────────────────────────────────────────────────┘

Zone heights:
  System bar:     32px  (always visible, fixed)
  Status band:    24px  (contextual, collapsible)
  Main viewport:  auto  (flex-grow: 1)
  Control panels: 80–160px (depends on content)
  Keyboard row:   28px  (always visible, fixed)

Total chrome height: 60–220px
```

### 4.3 Panel / Frame Anatomy

Every grouping uses a bordered frame with an inline title:

```
┌─[PANEL TITLE]───────────────────────┐
│                                     │
│   Content area (padding: 8px)       │
│                                     │
└─────────────────────────────────────┘

CSS: 
  border: 1px solid var(--color-border);
  padding: 8px;
  position: relative;

  .panel-title {
    position: absolute;
    top: -8px;           /* centers on the top border */
    left: 8px;
    padding: 0 4px;
    background: var(--color-bg-elevated);
  }
```

Panels are **flush** — no margin between adjacent panels, they share borders. No border-radius anywhere.

---

## 5. Component Patterns

### 5.1 Status Bar (Top System Bar)

**Format:**
```
[CALLSIGN] · [SUBSYSTEM] · [SECONDARY] ░░░░ [CLOCK] · [STATE]
```

**Data types and their format:**
```
System identifier:  "SYS-01" or "MCC-MAIN" (short, 3-8 chars)
Subsystem:          "TELEMETRY" "COMMS" "TRACK" "NAV"
Clock:              "UTC 14:23:05" or "T+00:12:33" or "T-00:04:11"
State indicator:    "NOMINAL" / "STANDBY" / "ALERT" / "FAULT"
Signal/quality:     "SIG:089" or "LINK:OK" or numeric bar "████░░"
```

**Separator convention:**
```css
/* Items separated by: */
·          /* U+00B7 MIDDLE DOT — for equal-weight fields */
│          /* U+2502 BOX DRAWINGS LIGHT VERTICAL — for major section breaks */
░          /* U+2591 LIGHT SHADE — for visual padding/alignment filler */

/* NEVER use: comma, semicolon, pipe | (too code-editor), dash alone */
```

**Example HTML/CSS:**
```html
<header class="sys-bar">
  <span class="sys-id">MCC-01</span>
  <span class="sys-sep">·</span>
  <span class="sys-sub">TELEMETRY</span>
  <span class="sys-spacer" aria-hidden="true"></span>
  <span class="sys-clock" id="clock">UTC 00:00:00</span>
  <span class="sys-sep">·</span>
  <span class="sys-state" data-state="nominal">NOMINAL</span>
</header>
```

```css
.sys-bar {
  display: flex;
  align-items: center;
  gap: 8px;
  height: 32px;
  padding: 0 12px;
  border-bottom: 1px solid var(--color-border);
  font-size: 10px;
  letter-spacing: 0.12em;
  color: var(--color-amber-mid);
}

.sys-spacer {
  flex: 1;
  border-bottom: 1px dotted var(--color-amber-trace);
  margin: 0 8px;
  align-self: center;
}

.sys-state[data-state="nominal"]  { color: var(--color-normal); }
.sys-state[data-state="active"]   { color: var(--color-active); }
.sys-state[data-state="alert"]    { color: var(--color-alert); animation: sys-blink 1s step-start infinite; }
.sys-state[data-state="critical"] { color: var(--color-critical); animation: sys-blink 0.5s step-start infinite; }
```

### 5.2 Button Grid

Military/control room buttons live in clustered grids. Each button is a rectangle with a label. No icons (or only ASCII glyphs). States communicate via brightness, not color variety.

**CSS Class Names:**
```
.btn              — base button
.btn--inactive    — default resting state (dim)
.btn--active      — currently engaged/selected (bright)
.btn--armed       — ready-to-execute, awaiting confirmation (brightest + distinct)
.btn--alert       — fault or requires attention (different texture or blink)
.btn--disabled    — unavailable (trace dim)
```

**State Visual Mapping (amber palette):**
```
INACTIVE:  bg #130F07, text #7A5200, border #3D2900
           → "dark, waiting"

ACTIVE:    bg #3D2900, text #FFB000, border #CC8800
           → "lit, engaged"

ARMED:     bg #7A5200, text #FFD060, border #FFB000
           → "charged, about to fire"
           + subtle pulsing glow (see motion)

ALERT:     bg #3D1500, text #FF6B00, border #FF6B00
           → "attention required"
           + 1Hz blink on border

DISABLED:  bg #0B0900, text #3D2900, border #1A1200
           → "unpowered, don't interact"
```

**Button Grid CSS:**
```css
.btn-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(80px, 1fr));
  gap: 4px;          /* physical gap between buttons — like the housing gap */
}

.btn {
  display: flex;
  align-items: center;
  justify-content: center;
  height: 32px;
  padding: 0 8px;
  border: 1px solid var(--color-border);
  background: var(--color-bg-elevated);
  color: var(--color-amber-dim);
  font-family: var(--font-terminal);
  font-size: 10px;
  font-weight: 700;
  letter-spacing: 0.15em;
  text-transform: uppercase;
  cursor: pointer;
  user-select: none;
  transition: background 0.05s, color 0.05s, border-color 0.05s;
  /* NO border-radius. NONE. */
}

.btn:hover {
  background: var(--color-amber-trace);
  color: var(--color-amber-mid);
  border-color: var(--color-amber-dim);
}

.btn:active,
.btn--active {
  background: #3D2900;
  color: var(--color-amber-bright);
  border-color: var(--color-amber-mid);
  box-shadow: 0 0 6px 0 rgba(255, 176, 0, 0.3);
}

.btn--armed {
  background: #7A5200;
  color: var(--color-amber-hot);
  border-color: var(--color-amber-bright);
  animation: btn-armed-pulse 2s ease-in-out infinite;
}

.btn--alert {
  background: #3D1500;
  color: #FF6B00;
  border-color: #FF6B00;
  animation: border-blink 1s step-start infinite;
}

.btn--disabled {
  background: var(--color-bg-recessed);
  color: var(--color-amber-trace);
  border-color: #1A1200;
  cursor: not-allowed;
  pointer-events: none;
}

/* Button cluster grouping (physical housing boundary) */
.btn-cluster {
  display: inline-flex;
  flex-direction: column;
  gap: 4px;
  padding: 8px;
  border: 1px solid var(--color-border);
  background: var(--color-bg);
}

.btn-cluster-label {
  font-size: 8px;
  color: var(--color-amber-trace);
  letter-spacing: 0.2em;
  text-align: center;
  margin-bottom: 4px;
}
```

### 5.3 Main Viewport Background Textures

Three canonical viewport backgrounds, implemented in pure CSS (no canvas for the background itself):

#### Option A: Map / Grid Overlay
```css
.viewport--map {
  background-color: var(--color-bg);
  background-image:
    /* Lat/lon grid */
    repeating-linear-gradient(
      0deg,
      transparent,
      transparent 39px,
      var(--color-amber-trace) 39px,
      var(--color-amber-trace) 40px
    ),
    repeating-linear-gradient(
      90deg,
      transparent,
      transparent 59px,
      var(--color-amber-trace) 59px,
      var(--color-amber-trace) 60px
    );
  /* Fine grid at 10px */
  /* Overlay actual SVG world map at 5% opacity in --color-amber-trace */
}
```

#### Option B: Radar Sweep
```css
/* Background radial fade */
.viewport--radar {
  background: radial-gradient(
    ellipse at center,
    #1A1200 0%,
    var(--color-bg) 60%,
    var(--color-bg-recessed) 100%
  );
  /* Sweep arm via @keyframes — see Motion section */
}

/* Concentric range rings via SVG or pseudo elements */
.viewport--radar::before {
  content: '';
  position: absolute;
  inset: 0;
  border-radius: 50%;
  box-shadow:
    inset 0 0 0 1px var(--color-amber-trace),
    inset 0 0 0 calc(25%) var(--color-amber-trace),
    inset 0 0 0 calc(50%) var(--color-amber-trace),
    inset 0 0 0 calc(75%) var(--color-amber-trace);
}
```

#### Option C: Waveform / Oscilloscope
```css
.viewport--wave {
  background: var(--color-bg);
  /* SVG path waveform overlaid in amber */
  /* Or canvas-rendered in separate layer */
}
```

**For all viewports: horizontal scan lines are applied as a separate overlay layer (see Section 6).**

### 5.4 Status Readouts (Data Panels)

Dense tabular data in the SAGE/IBM 3270 style:

```
FORMAT:  LABEL···VALUE   (label left-padded, value right-padded or right-aligned)
         LABEL───VALUE   (with dashes/dots as fill)
         [LABEL] VALUE   (bracketed label variant)
```

```css
.readout-row {
  display: flex;
  justify-content: space-between;
  gap: 8px;
  font-size: 11px;
  line-height: 1.6;
  color: var(--color-amber-mid);
}

.readout-label {
  color: var(--color-amber-dim);
  letter-spacing: 0.1em;
  flex-shrink: 0;
  /* Fill dots between label and value */
  position: relative;
}

.readout-label::after {
  content: '·····················································';
  position: absolute;
  left: calc(100% + 4px);
  color: var(--color-amber-trace);
  overflow: hidden;
  white-space: nowrap;
  max-width: 60px;
}

.readout-value {
  color: var(--color-amber-bright);
  font-variant-numeric: tabular-nums;
  text-align: right;
  flex-shrink: 0;
}

/* Alert state — value glows */
.readout-value--alert { color: var(--color-alert); }
.readout-value--armed { color: var(--color-armed); }
```

**Communicating state WITHOUT adding new colors:**
```
NOMINAL:   value displayed normally (mid amber)
ACTIVE:    value displayed bright (hot amber) + prefix ► 
ALERT:     value blinking 1Hz + prefix !! or ⚠
OFFLINE:   value replaced with "----" in trace color
ARMED:     value displayed HOT + prefix ◉ + slow pulse glow
CRITICAL:  value in alert color + faster blink + prefix ██
```

### 5.5 Bottom Action Bar (Keyboard Row)

Modeled after the function key row in DOS applications and teletype terminals:

```
[F1] DECODE · [F2] CLEAR · [F3] EXPORT │ [ESC] RESET     [?] HELP
```

```css
.keyboard-row {
  display: flex;
  align-items: center;
  gap: 4px;
  height: 28px;
  padding: 0 8px;
  border-top: 1px solid var(--color-border);
  background: var(--color-bg-recessed);
  font-size: 9px;
  letter-spacing: 0.1em;
  color: var(--color-amber-dim);
  overflow: hidden;
}

.kbd-item {
  display: inline-flex;
  gap: 3px;
  align-items: center;
  white-space: nowrap;
}

.kbd-key {
  display: inline-block;
  padding: 1px 4px;
  border: 1px solid var(--color-amber-dim);
  color: var(--color-amber-bright);
  font-size: 8px;
  font-weight: 700;
  line-height: 1.4;
  background: var(--color-bg-elevated);
}

.kbd-label {
  color: var(--color-amber-dim);
}

.kbd-spacer { flex: 1; }

.kbd-sep {
  color: var(--color-amber-trace);
  margin: 0 4px;
}
```

**HTML structure:**
```html
<footer class="keyboard-row" role="navigation" aria-label="Keyboard shortcuts">
  <div class="kbd-item">
    <span class="kbd-key">F1</span>
    <span class="kbd-label">DECODE</span>
  </div>
  <span class="kbd-sep">·</span>
  <div class="kbd-item">
    <span class="kbd-key">F2</span>
    <span class="kbd-label">CLEAR</span>
  </div>
  <span class="kbd-sep">·</span>
  <div class="kbd-item">
    <span class="kbd-key">F3</span>
    <span class="kbd-label">EXPORT</span>
  </div>
  <span class="kbd-sep">│</span>
  <div class="kbd-item">
    <span class="kbd-key">ESC</span>
    <span class="kbd-label">RESET</span>
  </div>
  <span class="kbd-spacer"></span>
  <div class="kbd-item">
    <span class="kbd-key">?</span>
    <span class="kbd-label">HELP</span>
  </div>
</footer>
```

---

## 6. Motion & Effects

**Governing principle:** Effects simulate physical phosphor and CRT properties. They are environmental, not decorative. An effect you consciously *notice* on every interaction is too strong — it should read as "the display technology" not "the designer showing off."

### 6.1 Phosphor Glow (Text & Elements)

Implemented purely via CSS `text-shadow` and `box-shadow`. No canvas. No WebGL.

```css
/* Text glow — phosphor bloom on characters */
.phosphor-glow {
  text-shadow:
    0 0 4px rgba(255, 176, 0, 0.8),   /* tight inner glow */
    0 0 12px rgba(255, 176, 0, 0.35), /* medium spread */
    0 0 24px rgba(255, 176, 0, 0.12); /* faint outer halo */
}

/* Box glow — panel borders & active elements */
.panel-glow {
  box-shadow:
    0 0 4px 0 rgba(255, 176, 0, 0.3),   /* inner */
    0 0 12px 0 rgba(255, 176, 0, 0.1);  /* outer */
}

/* Active/hot state — more intense */
.phosphor-glow--hot {
  text-shadow:
    0 0 2px rgba(255, 208, 96, 1.0),    /* white-core bloom */
    0 0 6px rgba(255, 176, 0, 0.9),
    0 0 16px rgba(255, 176, 0, 0.5),
    0 0 32px rgba(255, 176, 0, 0.2);
}

/* Button armed state glow */
@keyframes btn-armed-pulse {
  0%, 100% {
    box-shadow:
      0 0 4px 0 rgba(255, 176, 0, 0.4),
      inset 0 0 8px 0 rgba(255, 176, 0, 0.1);
  }
  50% {
    box-shadow:
      0 0 12px 0 rgba(255, 176, 0, 0.7),
      inset 0 0 16px 0 rgba(255, 176, 0, 0.2);
  }
}

/* RESTRAINT: Only apply .phosphor-glow to active/selected elements.
   Ambient text (labels, static values) should have NO glow or minimal
   single-layer glow at 0.2 opacity max. */
```

### 6.2 Scan Line Overlay

A subtle scan line texture applied to the whole interface as a fixed overlay. Critical: this should be BARELY visible — if you can clearly see the scanlines on static text, it's too strong.

```css
/* Applied to a full-screen overlay ::before pseudo-element on the root */
.crt-overlay {
  position: fixed;
  inset: 0;
  pointer-events: none;
  z-index: 9999;
  background-image: repeating-linear-gradient(
    to bottom,
    transparent 0px,
    transparent 1px,
    rgba(0, 0, 0, 0.08) 1px,  /* ← 0.08 is the MAX. Start at 0.04 */
    rgba(0, 0, 0, 0.08) 2px
  );
  background-size: 100% 2px;
}

/* Optional: very subtle vignette */
.crt-vignette {
  position: fixed;
  inset: 0;
  pointer-events: none;
  z-index: 9998;
  background: radial-gradient(
    ellipse at center,
    transparent 60%,
    rgba(0, 0, 0, 0.4) 100%
  );
}
```

**Do NOT animate the scan lines.** Animated scan lines are the #1 most overdone retro effect. The ones on real CRTs don't visibly scroll — they refresh at 60Hz which is invisible. Static scan lines are correct.

### 6.3 Blinking States

```css
/* Alert blink — 1Hz, hard step (not fade) */
@keyframes blink-alert {
  0%, 49% { opacity: 1; }
  50%, 100% { opacity: 0.15; }
}

/* Critical blink — 2Hz */
@keyframes blink-critical {
  0%, 33% { opacity: 1; }
  34%, 66% { opacity: 0.1; }
  67%, 100% { opacity: 1; }
}

/* Cursor blink — standard 1Hz block cursor */
@keyframes blink-cursor {
  0%, 49% { opacity: 1; }
  50%, 100% { opacity: 0; }
}

/* Border blink for alert states */
@keyframes border-blink {
  0%, 49% { border-color: var(--color-alert); }
  50%, 100% { border-color: var(--color-bg); }
}

/* NEVER use CSS `animation: blink 0.5s ease-in-out infinite` —
   real terminals use hard step transitions, not sinusoidal fades */
```

### 6.4 Transitions

```css
/* All interactive state changes: fast, no easing curves */
* { transition-timing-function: linear; }

button, .btn { transition: background 0.05s linear, color 0.05s linear, border-color 0.05s linear; }

/* Panels appearing: slight fade-in only */
.panel-enter { animation: panel-appear 0.1s linear forwards; }
@keyframes panel-appear {
  from { opacity: 0; }
  to   { opacity: 1; }
}

/* NO: transform transitions, scale bounce, slide-in animations, 
   spring physics, cubic-bezier ease-in-out on UI elements */
```

### 6.5 Optional: Phosphor Persistence (Text Shadow Animation)

Real phosphor "burns in" — bright areas stay slightly brighter. This can be hinted:

```css
/* Hover reveals the "hot" state faster than linear */
.btn:hover .btn-label {
  text-shadow:
    0 0 2px rgba(255, 208, 96, 0.9),
    0 0 8px rgba(255, 176, 0, 0.5);
  transition: text-shadow 0.03s linear; /* near-instant */
}
.btn:not(:hover) .btn-label {
  transition: text-shadow 0.8s linear; /* slow fade, like phosphor decay */
}
```

---

## 7. Implementation Notes

### 7.1 Base `:root` Block (Paste-Ready)

```css
:root {
  /* ── Color System ─────────────────────────────────── */

  /* Amber phosphor palette */
  --c-bg:           #0B0900;
  --c-bg-up:        #130F07;
  --c-bg-dn:        #060500;

  --c-a-hot:        #FFD060;
  --c-a-bright:     #FFB000;
  --c-a-mid:        #CC8800;
  --c-a-dim:        #7A5200;
  --c-a-trace:      #3D2900;

  /* State semantic */
  --c-normal:       var(--c-a-mid);
  --c-active:       var(--c-a-bright);
  --c-armed:        var(--c-a-hot);
  --c-alert:        #FF6B00;
  --c-critical:     #FF3300;
  --c-offline:      var(--c-a-trace);

  /* Structural */
  --c-border:       var(--c-a-dim);
  --c-border-focus: var(--c-a-bright);
  --c-sep:          var(--c-a-trace);

  /* ── Typography ───────────────────────────────────── */
  --f-mono: 'Share Tech Mono', 'IBM Plex Mono', 'Courier New', monospace;

  --fs-xs:   9px;
  --fs-sm:   10px;
  --fs-base: 12px;
  --fs-md:   14px;
  --fs-lg:   18px;
  --fs-xl:   28px;
  --fs-2xl:  48px;

  /* ── Spacing (8px base unit) ──────────────────────── */
  --sp-0: 0px;
  --sp-1: 4px;   /* 0.5u */
  --sp-2: 8px;   /* 1u */
  --sp-3: 12px;  /* 1.5u */
  --sp-4: 16px;  /* 2u */
  --sp-6: 24px;  /* 3u */
  --sp-8: 32px;  /* 4u */

  /* ── Borders ─────────────────────────────────────── */
  --border-panel:  1px solid var(--c-border);
  --border-focus:  1px solid var(--c-border-focus);
  --border-none:   1px solid transparent;

  /* ── Glows (box-shadow only) ──────────────────────── */
  --glow-subtle:   0 0 4px rgba(255,176,0,0.2);
  --glow-normal:   0 0 4px rgba(255,176,0,0.4), 0 0 12px rgba(255,176,0,0.15);
  --glow-active:   0 0 6px rgba(255,176,0,0.6), 0 0 16px rgba(255,176,0,0.25);
  --glow-armed:    0 0 8px rgba(255,208,96,0.7), 0 0 24px rgba(255,176,0,0.35);

  /* ── Layout ─────────────────────────────────────────*/
  --z-sysbar: 100;
  --z-overlay: 9998;
  --z-scanlines: 9999;

  --layout-sys-bar: 32px;
  --layout-kbd-row: 28px;
  --layout-max-w:   960px;
}

/* Global resets for terminal aesthetic */
*, *::before, *::after {
  box-sizing: border-box;
  margin: 0;
  padding: 0;
}

body {
  background: var(--c-bg);
  color: var(--c-a-mid);
  font-family: var(--f-mono);
  font-size: var(--fs-base);
  text-transform: uppercase;
  -webkit-font-smoothing: none;
  font-smooth: never;
  line-height: 1.5;
  overflow-x: hidden;
}

/* No rounded corners anywhere */
button, input, select, textarea, div, section, article, nav, header, footer {
  border-radius: 0 !important;
}

/* Selection styling */
::selection {
  background: var(--c-a-mid);
  color: var(--c-bg);
}
```

### 7.2 ASCII Box Drawing Reference

Use Unicode box-drawing characters for structural elements:

```
Single line:   ─ │ ┌ ┐ └ ┘ ├ ┤ ┬ ┴ ┼
Double line:   ═ ║ ╔ ╗ ╚ ╝ ╠ ╣ ╦ ╩ ╬
Mixed:         ╒ ╓ ╕ ╖ ╘ ╙ ╛ ╜ ╞ ╡ ╤ ╧ ╪ ╫
Block:         █ ▓ ▒ ░
Arrows:        ► ◄ ▲ ▼ ▸ ◂ ▴ ▾
Bullets:       · • ◆ ◇ ○ ●
Misc:          ► ‖ ≡ ⊕ ⊗ ⊘
```

**Use single-line for** internal panel borders and separators.  
**Use double-line for** the outermost app container (like a terminal window chrome).  
**Use block elements** for progress bars, data fill indicators.

### 7.3 Utility CSS Classes

```css
/* State utilities */
.state-nominal   { color: var(--c-normal); }
.state-active    { color: var(--c-active); text-shadow: 0 0 8px rgba(255,176,0,0.5); }
.state-armed     { color: var(--c-armed);  text-shadow: 0 0 8px rgba(255,208,96,0.7); }
.state-alert     { color: var(--c-alert);  animation: blink-alert 1s step-start infinite; }
.state-critical  { color: var(--c-critical); animation: blink-critical 0.5s step-start infinite; }
.state-offline   { color: var(--c-offline); }

/* Text utilities */
.text-label  { color: var(--c-a-dim);   font-size: var(--fs-xs); letter-spacing: 0.15em; }
.text-value  { color: var(--c-a-bright); font-variant-numeric: tabular-nums; }
.text-muted  { color: var(--c-a-trace); }
.text-hot    { color: var(--c-a-hot); }

/* Layout utilities */
.panel       { border: var(--border-panel); padding: var(--sp-4); background: var(--c-bg-up); }
.panel-inset { border: var(--border-panel); padding: var(--sp-2); background: var(--c-bg-dn); }

/* Glow utilities */
.glow        { box-shadow: var(--glow-normal); }
.glow-active { box-shadow: var(--glow-active); }
.glow-text   { text-shadow: 0 0 4px rgba(255,176,0,0.6); }

/* Progress bar */
.progress-bar {
  display: flex;
  gap: 0;
  height: 8px;
}
.progress-fill {
  background: var(--c-a-bright);
  height: 100%;
}
.progress-empty {
  background: var(--c-a-trace);
  height: 100%;
}
```

### 7.4 Accessibility Notes

Authentic terminal aesthetics are **naturally high-contrast** — this is good. Specific notes:

- `text-shadow` glow should **not** be the only way information is conveyed
- Always maintain the `color` property alongside any glow effects
- `animation: blink` for alerts: provide a `prefers-reduced-motion` fallback
- All keyboard-row items need `aria-keyshortcuts` attributes
- Keep focus indicators strong (the natural `outline: 1px solid var(--c-border-focus)` is sufficient)

```css
@media (prefers-reduced-motion: reduce) {
  *, *::before, *::after {
    animation-duration: 0.01ms !important;
    animation-iteration-count: 1 !important;
  }
  /* Replace blinking with static highlight for alert states */
  .state-alert    { animation: none; filter: brightness(1.5); }
  .state-critical { animation: none; filter: brightness(2) saturate(1.5); }
  .btn--armed     { animation: none; box-shadow: var(--glow-armed); }
}
```

---

## 8. Anti-Patterns

### 8.1 The "Halloween Terminal" (Most Common Mistake)

**What it looks like:** Blindingly bright green or orange on pure black, maxed-out scanlines, glowing everything, animated noise/static effect, neon flicker.

**Why it's wrong:** Real CRT phosphor at full brightness has a warm white-orange core with color appearing at mid-brightness. Maxing everything to 100% brightness produces unrealistic "Halloween decoration" aesthetic, not authentic equipment. Equipment operators needed to work in that light for 8 hours.

**Fix:** Keep the base text at `--c-a-mid` (#CC8800), only elevate to bright/hot for specifically active elements.

---

### 8.2 Pure Black Background (#000000)

**What it looks like:** Text floating in void.

**Why it's wrong:** CRT screens in a dark room had visible "rest glow" from the phosphor surface. The screen was never true black. Additionally, dust, fingerprints, and screen curvature created micro-variations. Modern recreations using pure black miss this depth.

**Fix:** Use `#0B0900` (amber) or `#030D03` (green) — near-black with the appropriate tint of the phosphor color.

---

### 8.3 Border Radius

**What it looks like:** Rounded buttons, rounded panels, rounded cards.

**Why it's wrong:** Every physical component in a Cold War-era control room was machined aluminum or steel. Sharp corners. 90-degree angles. Any softness comes from the screen curvature (the CRT glass), not the UI elements.

**Fix:** `border-radius: 0` everywhere. Add a very subtle `border-radius: 2px` at most for the *outermost container* only if you want to hint at the CRT bezel. Never for interior elements.

---

### 8.4 Animated Scan Lines (Scrolling)

**What it looks like:** Scan lines scrolling downward at some speed.

**Why it's wrong:** CRT refresh happens at 50–60Hz — completely invisible to the human eye. The scan line pattern is stationary in perception. Animated scrolling scan lines signal "game about retro stuff" not "actual retro equipment."

**Fix:** Static `repeating-linear-gradient` at very low opacity (≤0.08). Do not animate it.

---

### 8.5 Using Every Color in the Palette for Each Component

**What it looks like:** Headers in hot amber, labels in mid amber, values in bright amber, borders in dim amber, all on one panel, plus glow on everything.

**Why it's wrong:** Creates visual noise. Real operator consoles used ONE brightness level for normal operation and reserved high brightness for exceptions. The contrast between normal and exceptional is what makes alerts visible.

**Fix:** Pick ONE amber shade for 90% of your interface (usually `--c-a-mid`). Use bright only for values that need attention. Use hot only for armed/critical states. The whole system should look dim and calm in its default state.

---

### 8.6 Decorative Glyphs & Emoji

**What it looks like:** ⚡🔴💻🖥️🚨 mixed into the UI.

**Why it's wrong:** Military and industrial terminals had NO graphical icons in the modern sense. They had:
- ASCII/extended ASCII symbols: `► ◄ ▲ ▼ ● ○ ◆ ■ □ ⊕`
- Alphanumeric codes: `ALERT`, `ARM`, `FIRE`
- Numeric codes: `STATUS: 04`, `ERR-17`

Modern emoji are designed for messaging apps. They break the aesthetic immediately.

**Fix:** Use Unicode Box Drawing, Block Elements, and Geometric Shapes blocks only. Or write it out: `!! FAULT` instead of 🚨.

---

### 8.7 Lowercase Text in Structural Elements

**What it looks like:** Labels reading "Power Level", "click here to decode", "connected".

**Why it's wrong:** Every operator console, teletype printout, and mainframe terminal from this era was UPPERCASE only. Lowercase capability was often not even present in the hardware. Mixing case immediately dates the design to "someone who did CSS in 2019."

**Fix:** `text-transform: uppercase` globally. Allow lowercase only in: actual user-generated text output, long-form documentation, or text that is explicitly framed as a "modern" interface element.

---

### 8.8 Drop Shadows on Text

**What it looks like:** `text-shadow: 2px 2px 4px rgba(0,0,0,0.5)` — text that looks like it's floating above the screen.

**Why it's wrong:** Phosphor text glows *outward* symmetrically. It does not cast a directional shadow beneath it. This is a print/web typography convention inappropriately applied.

**Fix:** Use symmetric glow only: `text-shadow: 0 0 8px rgba(255,176,0,0.5)`. The x and y offsets should always be `0 0`.

---

### 8.9 Gradients on UI Elements

**What it looks like:** Buttons with linear-gradient fills, panels with subtle gradient backgrounds.

**Why it's wrong:** Monochrome phosphor displays were uniform — no gradients were possible in hardware. Gradients on buttons read as "skeuomorphic Web 2.0" not terminal.

**Fix:** Solid fills only. The only gradients permitted are: the vignette overlay (a radial fade to black at screen edges), and the radar glow (radial from center). No gradients on discrete UI elements.

---

### 8.10 Excessive Effects (The Overproduction Trap)

**What it looks like:** Scanlines + noise animation + CRT curvature warp + RGB chromatic aberration + phosphor bloom + screen flicker + text shake.

**Why it's wrong:** Real operators would have requested maintenance on a terminal that behaved this way. The aesthetic communicates malfunction, not authority. More importantly, it makes actual content unreadable.

**Fix:** Choose ONE effect as your signature. The recommended hierarchy:
1. **Scanlines only** (static, subtle) — minimum viable retro
2. **Scanlines + text glow** — good balance, readable
3. **Scanlines + text glow + vignette** — rich, still professional
4. Anything beyond this needs a specific design justification

---

## Appendix: Quick Reference Card

### Amber Colors (copy-paste)
```
Background:  #0B0900  #130F07  #060500
Phosphor:    #FFD060  #FFB000  #CC8800  #7A5200  #3D2900
State:       normal=#CC8800  active=#FFB000  armed=#FFD060  alert=#FF6B00  crit=#FF3300
```

### Green Colors (copy-paste)
```
Background:  #030D03  #0A1A0A  #020802
Phosphor:    #CCFF66  #33FF00  #00BB00  #006600  #002B00
State:       normal=#00BB00  active=#33FF00  armed=#CCFF66  alert=#FFFF00  crit=#FF4400
```

### Glow Formula
```css
/* Amber phosphor bloom */
text-shadow: 0 0 4px rgba(255,176,0,.8), 0 0 12px rgba(255,176,0,.35), 0 0 24px rgba(255,176,0,.12);

/* Green phosphor bloom */
text-shadow: 0 0 4px rgba(51,255,0,.8), 0 0 12px rgba(51,255,0,.35), 0 0 24px rgba(51,255,0,.12);
```

### Box Drawing Template
```
┌─[LABEL]──────────────┐
│  field:         value │
│  field:         value │
│                       │
│  ╔═══[INNER]════╗     │
│  ║  value       ║     │
│  ╚══════════════╝     │
└───────────────────────┘
```

### State Without Color (Monochrome Systems)
```
OFFLINE:  ---- (em-dashes)
NOMINAL:  normal brightness
ACTIVE:   ► VALUE (arrow prefix)
ARMED:    ◉ VALUE (filled circle)
ALERT:    !! VALUE (blinking)
CRITICAL: ██ VALUE (block prefix + fast blink)
```

---

*Document generated by OpenClaw Research Specialist — February 2026*  
*Sources: SAGE/MITRE historical documentation, IBM 3270 architecture reference, retrocomputing.SE phosphor analysis, ISA-101 HMI standard, cool-retro-term profiles, eiDotter design system, TsUP/MCC-H photographic reference.*
