# COLD WAR TERMINAL DESIGN SYSTEM (AMBER)

## COLOR TOKENS

```css
--color-bg:            #0B0900;
--color-bg-elevated:   #130F07;
--color-bg-recessed:   #060500;
--color-amber-hot:     #FFD060;
--color-amber-bright:  #FFB000;
--color-amber-mid:     #CC8800;
--color-amber-dim:     #7A5200;
--color-amber-trace:   #3D2900;
--color-alert:         #FF6B00;
--color-critical:      #FF3300;
--color-border:        #7A5200;
--color-border-active: #FFB000;
```

## TYPOGRAPHY

- Font stack: `'Share Tech Mono', 'IBM Plex Mono', 'Courier New', monospace`
- All labels are uppercase.
- Dense line-height: `1.2` to `1.3`.
- UI sizes: `10px` to `14px`.
- Base size: `11px`.
- Bright text glow only on `--color-amber-bright` elements:

```css
text-shadow: 0 0 8px rgba(255,176,0,0.4);
```

## LAYOUT

- Mobile-first width target: `390px`.
- System bar top: `24px` tall.
- Main content area: flex-grow + scrollable.
- Bottom action bar: `56px + safe-area-inset-bottom`.
- Touch target minimum: `44px x 44px`.

## SCREEN PATTERNS

### Home

- Title: `UNIFIED COMMS`.
- Subtitle: `SENSOR WATCH DATA LINK v1.0`.
- Background texture: very subtle world-map outlines at low opacity with `--color-amber-trace`.
- Mode cards:
  - `>> TX` + `OPTICAL TIME SYNC`
  - `>> RX` + `DECODE SLEEP DATA`
- Card hover/focus:
  - border to `--color-amber-bright`
  - background to `--color-bg-elevated`

### TX

- Top mode bar: `< BACK` and `TX MODE`.
- Flash area fills between top and bottom bars.
- Flash background alternates `#000000` and `#FFFFFF` during transmission.
- Center status text: `STANDBY`, `TRANSMITTING`, `COMPLETE`.
- Distance hint: `HOLD 2-6 IN FROM WATCH`.
- Bottom action:
  - `SEND TIME SYNC` (armed)
  - `CANCEL` while transmitting in alert color
  - progress line: `TRANSMITTING... Xs REMAINING`

### RX

- Top mode bar: `< BACK` and `RX MODE`.
- Input label: `HEX INPUT - 574 CHARS (287 BYTES x 7 NIGHTS)`.
- Input style:
  - background `--color-bg-recessed`
  - border `--color-amber-dim`
  - text `--color-amber-bright`
  - placeholder `--color-amber-trace`
- Metrics:
  - `CS [XX]` large centered
  - 5 subscores: `SRI`, `DUR`, `EFF`, `CMP`, `LGT`
  - 7-day pixel chart using amber quality scale (`trace -> dim -> mid -> bright -> hot`)
  - compact night rows with amber separators
- Export row buttons: `CSV` and `JSON`.
- Bottom controls:
  - `DECODE` primary
  - `CLEAR` secondary
  - `TEST` tertiary (small)

## EFFECTS

- Scanline overlay (subtle only):

```css
background: repeating-linear-gradient(
  to bottom,
  transparent 0 2px,
  rgba(0,0,0,0.15) 2px 4px
);
opacity: 0.3;
pointer-events: none;
```

- No animated glow.
- No flicker effects.
- No heavy post-processing.

## ASCII / GLYPHS

Use ASCII-friendly terminal glyphs only.

Allowed set:

`>> << | - _ = / \ [ ] ( ) * # @ ! ? . , + ~ ^ > < ◆ ▸ ●`

## ANTI-PATTERNS

- Do not introduce off-palette colors.
- Do not use rounded modern glassmorphism styles.
- Do not use oversized typography outside documented ranges.
- Do not add decorative animations.
- Do not drift from dense terminal spacing or uppercase labeling.
