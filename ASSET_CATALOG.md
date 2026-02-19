# ASSET CATALOG - Priority 3 Integration

## Purpose
Catalog of dlorp's Google Drive assets (`inspo(?)` folder) matching the amber CRT aesthetic for second-movement firmware builder.

**Current aesthetic:** Amber monochrome (#FFB000), scanline overlay, Share Tech Mono font, terminal-inspired UI

**Priority 3 Goals:**
1. Face preview wireframes
2. Background textures (scanlines, glyphs, technical patterns)
3. Custom retro/technical fonts

---

## 🎯 HIGH PRIORITY - Perfect Matches

### 1. Watch Face Wireframes
**Location:** `30 Wireframe Vectors Pack/PNG/`
**Files:** 30 PNG wireframe graphics (1.png - 30.png)
**Sizes:** 75 KB - 1.3 MB each
**Format:** PNG (transparent backgrounds likely)

**Use case:** Visual previews for each watch face in the builder face list
- Match each face ID to a wireframe graphic
- Display alongside face name in the face list
- Small thumbnail size (40-60px) to keep UI compact
- Could also use in documentation/README

**Alternative:** `Abstract Wireframe Backgrounds/` (backup option)

### 2. CRT/Scanline Textures
**Location:** `Frequencies Scan Texture Pack/`
**Files:** 17 TIF scan textures
- `public-domain-texture-raw-by-wendelin-jacober-06-2020 (1-16).tif`
- Original: `public-domain-texture-raw-by-wendelin-jacober-06-2020.tif`

**Format:** TIF (high quality, will need conversion to PNG/WebP for web)
**License:** Public domain (Wendelin Jacober, 2020)

**Use cases:**
- Enhanced scanline overlay (replace current CSS gradient)
- Subtle background texture behind UI elements
- Loading screen texture
- Section dividers
- Could animate slowly for subtle CRT flicker effect

### 3. Custom Fonts
**Location:** `font/@daniel schriër/`
**Found:** `Heritage Display - pixel blackletter typeface`

**Style:** Pixel blackletter (retro + technical)
**Potential use:** Headers, section titles, or alternative to Share Tech Mono

**Notes:**
- Blackletter might be too ornate for primary UI text
- Could work well for branding/headers ("SECOND MOVEMENT BUILDER")
- Test readability at small sizes
- Currently using Share Tech Mono (Google Fonts) - works well, keep as primary

---

## 📋 MEDIUM PRIORITY - Complementary Assets

### 4. **Thermal/Heat Map Textures** ⚡ NEW DISCOVERY
**Location:** `@GFXDatabase/X10 Thermo Texture Pack V1 - thermal/JPG/`
**Files:** 10 thermal texture images (GFX_DATABASE-ThermoTexture-1.jpg through 10.jpg)
**Sizes:** 17-36 MB each (high-res, will need downsampling)
**Use case:** 
- Heat map overlays for system stats/performance
- Technical background patterns
- Loading indicators
- Data visualization aesthetics
**Vibe:** Infrared/thermal camera, technical diagnostics, cyberpunk monitoring

### 5. Technical Glitch Textures
**Location:** `Abstract Outline Glitch Texture/`
**Use case:** Subtle background patterns, error states, loading indicators
**Vibe:** Technical, digital corruption aesthetic

### 6. Retro Print Effects
**Location:** `90s Magazine Print Effect/`
**Use case:** Optional texture overlay for vintage authenticity
**Vibe:** CMYK halftone, newsprint feel

### 7. Analog Fax Textures
**Location:** `Fax Textures/`
**Use case:** Background paper texture, document-like feel
**Vibe:** Analog transmission artifacts

### 8. Retro Patterns
**Location:** `Patterns and Backgrounds - retro cult film, geometric 60s 70s/`
**Use case:** Decorative backgrounds, section dividers
**Vibe:** Geometric, vintage sci-fi

---

## 🎨 OTHER AVAILABLE ASSETS

**Creator Collections (20+ designers):**
- @COLORPONG, @CreativeArtX, @Design Syndrome, @Dreadlabs, @featherandsage
- @FLYERWRK, @Frankentoon Studio, @GFXDatabase, @hellomartco, @InArtFlow
- @Jesse Nunez, @KAT, @Mad Supply, @Martyr, @NickJayDesign
- @offsetcollective.com, @Pixflow, @Rule By Art, @Shi Mino, @Softulka
- @TypeDuNord

**Additional Packs:**
- 58 High Res Collage Textures
- Divinity - gfx kit
- Holographic Papers textures
- Ink and Photocopy Texture Effect
- Nerve Textures (abstract biologic)
- SMOKEY GFX KIT
- Venom (snake vector illustrations)
- Vintage Collage Creator (750+ assets)

**Notes:** These are available but less aligned with the current terminal/CRT aesthetic. Could explore if expanding to other projects or visual styles.

---

## 📝 IMPLEMENTATION RECOMMENDATIONS

### Phase 1: Wireframes (Immediate)
1. **Download:** `30 Wireframe Vectors Pack/PNG/` (all 30 files)
2. **Optimize:** Convert to WebP, resize to 60px thumbnails
3. **Integrate:** Add wireframe previews to face list items
4. **Map:** Create face_id → wireframe_filename mapping
   - Could use face category to pick appropriate wireframe style
   - Or manually curate best matches

### Phase 2: Enhanced Textures (Quick Win)
1. **Download:** 2-3 scan textures from `Frequencies Scan Texture Pack/`
2. **Convert:** TIF → PNG/WebP (optimize for web)
3. **Integrate:** Replace CSS scanline gradient with subtle texture overlay
   - Test opacity levels (10-20% likely)
   - Consider animated subtle movement
4. **Optional:** Add to specific sections (header, footer, modal backgrounds)

### Phase 3: Custom Fonts (Experimental)
1. **Download:** `Heritage Display` font files
2. **Test:** Readability at 11px, 14px, 18px
3. **Implement:** If suitable, use for:
   - Page title: "SECOND MOVEMENT BUILDER"
   - Section headers: "WATCH FACES", "SETTINGS", etc.
   - Keep Share Tech Mono for body text
4. **Fallback:** If too ornate, stick with current font stack

### Phase 4: Background Textures (Polish)
1. **Download:** 1-2 files from `Abstract Outline Glitch Texture/`
2. **Integrate:** Subtle background layer behind elevated UI elements
3. **Test:** Ensure doesn't interfere with readability
4. **Optional:** Fax texture for documentation pages

---

## 🚀 NEXT STEPS

**For Priority 3 kickoff:**
1. **You decide:** Which assets to download first?
2. **I can:**
   - Download and optimize selected assets
   - Create wireframe mapping (face_id → graphic)
   - Integrate into builder UI
   - Test different texture opacity levels
   - Document asset usage and licensing

**Questions for you:**
- Should I start with wireframes only, or combine with textures?
- Any specific wireframe styles you prefer (more technical, more artistic, etc.)?
- Keep current scanline CSS or replace with actual texture?
- Stick with Share Tech Mono or try Heritage Display for headers?

---

## 📄 LICENSE NOTES

- **Wireframes:** Verify licensing in pack (likely commercial-friendly)
- **Scan Textures:** Public domain (Wendelin Jacober, 2020) ✅
- **Heritage Display:** Check font license in download
- All other packs: Verify licensing before commercial use

**Recommendation:** Download license files alongside assets for record-keeping.
