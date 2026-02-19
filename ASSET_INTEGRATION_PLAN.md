# Asset Integration Plan - Priority 3

## ✅ DOWNLOAD & EXTRACTION COMPLETE

**Assets downloaded:** 5 packages (~994 MB)
**Assets extracted:** 174 files
**Assets organized:** 893 MB in ~/repos/second-movement/assets/

---

## 📦 Current Status

### ✅ Ready to Use (No Optimization Needed)
- **Heritage Display Font** (96 KB) - OTF ready for @font-face
- **Wireframe PNGs** (9.5 MB, 32 files) - Can use as-is or resize to thumbnails

### ⚠️ Needs Optimization
- **CRT Scan Textures** (707 MB, 17 TIF files) - Convert to WebP + downscale
- **Dataism III Graphics** (176 MB, 22 EPS files) - Convert to SVG or PNG

---

## 🎯 NEXT STEPS

### Phase 1: Quick Win (Use What's Ready)
**Goal:** Get Priority 3 PR up with fonts + wireframes

**Actions:**
1. ✅ Copy wireframes to builder (DONE)
2. ✅ Copy font to builder (DONE)
3. Add @font-face for Heritage Display
4. Create face_id → wireframe mapping
5. Update builder UI with wireframe previews
6. Create PR #42: "Priority 3.1 - Wireframes & Font"

**Timeline:** 30-45 minutes
**Result:** Visual upgrade with minimal file size

---

### Phase 2: Texture Optimization (Requires ImageMagick)
**Goal:** Add CRT scan texture overlay

**Prerequisites:**
```bash
brew install imagemagick webp
```

**Actions:**
1. Convert TIF → WebP (quality 80, resize to 1920px):
   ```bash
   cd ~/repos/second-movement/assets/textures
   for f in *.tif; do
     convert "$f" -resize 1920x -quality 80 "${f%.tif}.webp"
   done
   ```

2. Pick best 3-4 textures (~2-3 MB total)

3. Add CSS overlay:
   ```css
   .builder::before {
     content: '';
     position: absolute;
     top: 0; left: 0; right: 0; bottom: 0;
     background: url('/assets/textures/scan-01.webp');
     opacity: 0.15;
     pointer-events: none;
     mix-blend-mode: overlay;
   }
   ```

4. Create PR #43: "Priority 3.2 - CRT Scan Texture"

**Timeline:** 1 hour (including install + conversion)
**Result:** Authentic CRT aesthetic

---

### Phase 3: Data Viz Graphics (Optional)
**Goal:** Add particle/data flow backgrounds

**Prerequisites:**
```bash
brew install inkscape
```

**Actions:**
1. Convert best 5-10 EPS → SVG:
   ```bash
   cd ~/repos/second-movement/assets/dataviz
   for f in E3931_06.eps E3931_08.eps E3931_12.eps E3931_13.eps E3931_19.eps; do
     inkscape "$f" --export-type=svg --export-filename="${f%.eps}.svg"
   done
   ```

2. Add as subtle backgrounds (very low opacity)

3. Create PR #44: "Priority 3.3 - Data Viz Backgrounds"

**Timeline:** 1-2 hours
**Result:** Technical/cyberpunk polish

---

## 📊 Estimated File Sizes

**Current (unoptimized):** 893 MB
**After Phase 1:** ~10 MB (wireframes + font only)
**After Phase 2:** ~15 MB (+ 3-4 scan textures)
**After Phase 3:** ~25 MB (+ 5-10 data viz graphics)

---

## 🚀 RECOMMENDED APPROACH

**Start with Phase 1 NOW:**
- Wireframes + font are ready
- No dependencies, no conversions needed
- Immediate visual impact
- Can ship PR today

**Phase 2 + 3:**
- Install ImageMagick + Inkscape
- Run optimization scripts
- Follow-up PRs

**OR: Skip optimization, use selectively:**
- Pick 2-3 smallest TIF files, use as-is for testing
- Pick 2-3 smallest EPS files, convert with online tools
- Ship full Phase 1-3 in one PR

---

## 🔐 Licensing TODO

Before merging ANY PR, document:
- Frequencies Scan Textures: PUBLIC DOMAIN ✓ (Wendelin Jacober, 2020)
- 30 Wireframe Vectors: Check license
- Dataism III: Check license (@COLORPONG)
- Heritage Display: Check web font usage (@daniel schriër)

Create `assets/LICENSES.md` with full attribution.

---

## ✅ READY TO START?

**I recommend:**
1. Start Phase 1 now (wireframes + font)
2. I'll create the integration code
3. Ship PR #42 today
4. Optimize textures/graphics tomorrow
5. Ship PR #43-44 as polish

**Want me to start Phase 1?** Say the word and I'll:
- Add Heritage Display to CSS
- Create wireframe → face mapping
- Update builder UI
- Create PR with before/after screenshots

🫡
