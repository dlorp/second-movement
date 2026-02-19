# Download Plan - Priority 3 Assets

## ✅ WHAT TO DOWNLOAD (In Order of Priority)

### TIER 1: Essential (Do First)

#### 1. **30 Wireframe Vectors Pack**
- **Path:** `30 Wireframe Vectors Pack/PNG/`
- **What:** All 30 wireframe PNG files
- **Why:** Face preview icons
- **Size:** ~10-15 MB
- **Action:** Download entire PNG folder

#### 2. **Frequencies Scan Texture Pack**
- **Path:** `Frequencies Scan Texture Pack/`
- **What:** Pick 3-4 best scan textures (TIF files)
- **Why:** CRT scanline overlay (public domain)
- **Size:** ~5-10 MB (3-4 files)
- **Action:** Preview and download your favorites

#### 3. **Heritage Display Font**
- **Path:** `font/@daniel schriër/Heritage Display - pixel blackletter typeface/`
- **What:** Font files (TTF/OTF)
- **Why:** Headers and branding
- **Size:** ~1-2 MB
- **Action:** Download entire font folder

### TIER 2: Enhancement (Nice to Have)

#### 4. **Thermal Texture (1-2 files)**
- **Path:** `@GFXDatabase/X10 Thermo Texture Pack V1 - thermal/JPG/`
- **What:** Pick 1-2 thermal images
- **Why:** Heat map/diagnostic aesthetic
- **Size:** 17-36 MB each (pick smallest)
- **Action:** Download GFX_DATABASE-ThermoTexture-2.jpg and -8.jpg

#### 5. **Abstract Glitch Texture**
- **Path:** `Abstract Outline Glitch Texture/`
- **What:** Glitch texture files
- **Why:** Error states, loading
- **Size:** Unknown (likely 5-10 MB)
- **Action:** Download folder

#### 6. **Designer Dingbats**
- **Path:** `@TypeDuNord/Designer Dingbats - 120 shapes/`
- **What:** Shape/icon library
- **Why:** UI accents, decorative elements
- **Size:** Unknown (likely 5-10 MB)
- **Action:** Download folder

### TIER 3: Future Exploration

- Fax Textures
- Patterns and Backgrounds (60s/70s geometric)
- 90s Magazine Print Effect
- Collage Textures
- Other creator packs

---

## 📥 DOWNLOAD INSTRUCTIONS

Since I can't reliably automate Google Drive downloads via browser, here's the manual process:

### Option 1: Download via Browser
1. Navigate to https://drive.google.com/drive/folders/1qjhACfUvawpuQdpV8hdHVglGHIFC6UMu
2. For each folder/file:
   - Right-click → Download
   - Wait for ZIP to download to ~/Downloads/
3. Tell me when done, I'll extract and organize

### Option 2: Use gdown (if available)
```bash
# Install gdown
pip3 install gdown

# Download entire folder (if folder sharing allows)
gdown --folder https://drive.google.com/drive/folders/FOLDER_ID
```

### Option 3: Tell Me What You Want
You pick which assets sound good and download them manually. I'll handle the rest.

---

## 🎯 RECOMMENDED START

**Minimum viable Priority 3:** Download just these 2:
1. **30 Wireframe Vectors Pack/PNG/**
2. **Frequencies Scan Texture Pack/** (3 files)

Total: ~15-20 MB, gives us the core aesthetic upgrade.

**Full Priority 3:** Add these:
3. Heritage Display font
4. 1-2 Thermal textures
5. Glitch textures
6. Designer Dingbats

Total: ~40-50 MB, complete visual overhaul.

---

## 🚀 WHAT I'LL DO AFTER DOWNLOAD

Once files are in `~/Downloads/`, I'll automatically:

1. **Extract ZIPs**
   ```bash
   cd ~/Downloads
   unzip "30 Wireframe*.zip" -d ~/repos/second-movement/assets/wireframes/
   unzip "Frequencies*.zip" -d ~/repos/second-movement/assets/textures/
   unzip "Heritage*.zip" -d ~/repos/second-movement/assets/fonts/
   ```

2. **Optimize for web**
   - Convert TIF → WebP (smaller, faster)
   - Resize wireframes to 60px thumbnails
   - Compress thermal textures (36MB → ~500KB)
   - Generate multiple sizes for responsive design

3. **Integrate into builder**
   - Update `builder/index.html` with wireframe previews
   - Replace CSS scanline with texture overlay
   - Add Heritage Display font to CSS
   - Create `face_icon_map.json` (face_id → wireframe.png)

4. **Create PR**
   - Before/after screenshots
   - Asset licensing info
   - Integration documentation

5. **Test**
   - Desktop/mobile responsiveness
   - Opacity levels for readability
   - Loading performance
   - Accessibility (ensure wireframes don't break screen readers)

---

## 📊 EXPECTED RESULTS

**Visual impact:**
- ✨ Wireframe icons make face list more scannable
- 🎨 Authentic CRT texture > simple CSS gradient
- 🔤 Heritage Display headers = stronger branding
- 🌡️ Optional thermal overlay = cyberpunk monitoring vibe

**Technical impact:**
- 📦 ~15-50 MB assets (optimized)
- 🚀 Minimal performance impact (WebP, lazy loading)
- ♿ Maintains accessibility
- 📱 Responsive across devices

Ready when you are! 🫡
