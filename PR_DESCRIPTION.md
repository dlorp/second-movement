# Priority 3: Asset Integration (Phases 1-2)

## 🎯 What This PR Does

Integrates authentic retro assets into the Second Movement Builder UI:
- ✅ **Phase 1:** Wireframe icons + Heritage Display font
- ✅ **Phase 2:** Authentic CRT scan texture overlays
- ⏭️ **Phase 3:** Data visualization graphics (follow-up PR - needs Ghostscript)

**Visual transformation:** CSS demo → Authentic retro terminal aesthetic

---

## ✨ Changes

### Assets Added
- **32 wireframe icons** (60×60px thumbnails, ~1.6 MB total)
- **Heritage Display font** (pixel blackletter, 96 KB)
- **3 CRT scan textures** (optimized PNGs, ~13 MB total)

**Total new assets:** ~15 MB (optimized from 893 MB source)

### UI Updates

#### 1. Heritage Display Font Integration
- Added `@font-face` for Heritage Display (pixel blackletter typeface)
- Applied to main title: "SECOND MOVEMENT BUILDER"
- Larger, bolder header (18px → 22px) with enhanced glow

**Before:** Share Tech Mono only
**After:** Heritage Display headers + Share Tech Mono body

#### 2. Wireframe Face Icons
- Hash-based assignment: each face ID maps to one of 32 wireframes
- Icons appear between handle and face name
- Amber-tinted to match CRT aesthetic (`filter: sepia hue-rotate saturate`)
- Lazy-loaded for performance

**Before:** Text-only face list
**After:** Icon + text face list (easier scanning)

#### 3. Authentic CRT Scan Texture
- Replaced CSS gradient scanline with real scan texture
- Public domain texture by Wendelin Jacober (2020)
- Subtle overlay (`opacity: 0.12, mix-blend-mode: overlay`)
- Repeating pattern across viewport

**Before:** `repeating-linear-gradient` (CSS)
**After:** Real CRT scan texture (PNG)

---

## 📊 Performance Impact

### Asset Sizes
- Wireframes: 1.6 MB (32 files @ ~50 KB each, lazy-loaded)
- Font: 96 KB (loaded once, cached)
- Textures: 13 MB (3 files @ 4-5 MB each)

**Total:** ~15 MB initial load (textures), 1.7 MB progressive (wireframes)

### Optimization Applied
- **Wireframes:** Resized from ~300 KB → ~50 KB each (85% reduction)
- **Textures:** Converted TIF → PNG, resized to 1920px (from 25-135 MB → 4-5 MB, 70-85% reduction)
- **Lazy loading:** Wireframe icons load on-demand

### Load Time Impact
- Font: +96 KB (negligible, `font-display: swap`)
- Textures: +13 MB (background image, cached)
- Wireframes: +1.6 MB (lazy-loaded per face)

**Recommendation:** Consider WebP conversion for textures (further 30-50% reduction)

---

## 🔐 Licensing

### Public Domain
- **Frequencies Scan Texture Pack** - Public domain by Wendelin Jacober, 2020
  - Source: [Creator attribution required]
  - License: CC0 / Public Domain

### Verify Before Merge
- **30 Wireframe Vectors Pack** - Check commercial usage rights
- **Heritage Display Font** (@daniel schriër) - Verify web font license
- **Dataism III** (@COLORPONG) - Check license (Phase 3)

**TODO:** Create `builder/assets/LICENSES.md` with full attribution

---

## 🧪 Testing

### Manual Testing Checklist
- [ ] Font loads correctly (check "SECOND MOVEMENT BUILDER" header)
- [ ] Wireframe icons appear next to face names
- [ ] Icons are amber-tinted (not grayscale)
- [ ] CRT scan texture visible as subtle overlay
- [ ] No console errors for missing assets
- [ ] Performance acceptable (no janky scrolling)

### Browser Testing
- [ ] Chrome/Edge (latest)
- [ ] Firefox (latest)
- [ ] Safari (macOS/iOS)
- [ ] Mobile responsiveness

### Accessibility
- [ ] Wireframe icons have `alt=""` (decorative)
- [ ] Text remains readable with texture overlay
- [ ] Font remains legible at all zoom levels

---

## 📸 Screenshots

### Before (Current Main)
```
+----------------------------------+
| SECOND MOVEMENT BUILDER          |
| (Share Tech Mono, CSS scanline)  |
+----------------------------------+
| FACE LIST                        |
| 1 ::::: simple_clock_face    x   |
| 2 ::::: stopwatch_face       x   |
| 3 ::::: world_clock_face     x   |
+----------------------------------+
```

### After (This PR)
```
+----------------------------------+
| 𝕾𝕰𝕮𝕺𝕹𝕯 𝕸𝕺𝖁𝕰𝕸𝕰𝕹𝕿 𝕭𝖀𝕀𝕷𝕯𝕰𝕽  |
| (Heritage Display, real texture) |
+----------------------------------+
| FACE LIST                        |
| 1 ::::: [📐] simple_clock    x   |
| 2 ::::: [🔷] stopwatch       x   |
| 3 ::::: [⚙️] world_clock     x   |
+----------------------------------+
```
*(Icons are wireframe graphics, not emoji)*

---

## 🚀 Future Work (Phase 3)

**Blocked by:** Ghostscript installation for EPS conversion

**When unblocked:**
1. Install Ghostscript: `brew install ghostscript`
2. Convert 5-10 Dataism III graphics (EPS → PNG/SVG)
3. Add as subtle background elements (very low opacity)
4. Create PR: "Priority 3.3 - Data Visualization Backgrounds"

**Estimated impact:** +5-10 MB assets, enhanced cyberpunk/technical aesthetic

---

## 📝 Notes

### Source Assets
- Downloaded from provided Google Drive (link not shared externally)
- Source files in `~/repos/second-movement/assets/` (893 MB unoptimized)
- Builder copies in `~/repos/second-movement/builder/assets/` (15 MB optimized)

### Optimization Tools Used
- `sips` (macOS built-in) - Image conversion and resizing
- `ImageMagick` - Attempted EPS conversion (needs Ghostscript for full support)

### Design Decisions
- **Wireframe assignment:** Hash-based (deterministic, no manual mapping needed)
- **Texture opacity:** 12% (subtle, doesn't obscure text)
- **Icon size:** 32px (balances visibility and space)
- **Font size:** 22px (large enough for impact, small enough for header)

---

## ✅ Ready for Review

**Phases 1-2 complete:**
- Wireframes ✓
- Font ✓
- CRT textures ✓

**Phase 3 deferred:**
- Data viz graphics (needs Ghostscript)
- Will be separate PR after install

**What to review:**
1. Visual aesthetic (does it match the retro CRT vibe?)
2. Performance (acceptable load times?)
3. Licensing (all assets properly attributed?)
4. Accessibility (text still readable?)
5. Code quality (clean integration?)

---

**Estimated review time:** 15-20 minutes
**Merge confidence:** High (self-contained, low risk)
**Follow-up needed:** Licensing verification, Phase 3 PR

🫡
