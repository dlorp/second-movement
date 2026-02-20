# Phase 2 Quality Toggle - Testing Guide

## Quick Test Checklist

### Builder App Testing

1. **Open builder/index.html**
   ```bash
   open builder/index.html
   # OR serve locally:
   python3 -m http.server 8000
   # Then: http://localhost:8000/builder/
   ```

2. **Verify Settings UI**
   - [ ] Scroll to SETTINGS section
   - [ ] CRT EFFECTS toggle appears after 24H MODE
   - [ ] 5 buttons visible: AUTO, OFF, MINIMAL, MEDIUM, HIGH
   - [ ] AUTO button highlighted by default (amber glow)
   - [ ] Help text visible: "Retro terminal visual effects"

3. **Test Quality Switching**
   - [ ] Click OFF → Alert appears → Reload page
   - [ ] After reload: Canvas hidden (black background)
   - [ ] Click MINIMAL → Alert → Reload
   - [ ] After reload: See pixelation + scanlines + vignette (no bloom)
   - [ ] Click HIGH → Alert → Reload
   - [ ] After reload: Full effects visible (bloom glow)
   - [ ] Click AUTO → Alert → Reload
   - [ ] After reload: Auto-detect based on device

4. **Test Persistence**
   - [ ] Set to MINIMAL → Reload
   - [ ] Close tab → Reopen builder/index.html
   - [ ] MINIMAL still active (button highlighted)
   - [ ] Effects match MINIMAL quality

5. **Console Verification**
   - [ ] Open DevTools Console (F12)
   - [ ] Look for log: `[CRT] User preference: MINIMAL quality`
   - [ ] Or: `[CRT] Auto mode - High-tier device: HIGH quality`

### Companion App Testing

1. **Open companion-app/index.html**
   ```bash
   open companion-app/index.html
   ```

2. **Verify Settings UI**
   - [ ] CRT EFFECTS toggle appears on home screen
   - [ ] Located below TX/RX mode cards
   - [ ] Same 5 buttons as builder
   - [ ] AUTO button highlighted by default

3. **Test Quality Switching**
   - [ ] Same tests as builder app
   - [ ] Navigate to TX mode → Effects still active
   - [ ] Navigate to RX mode → Effects still active
   - [ ] Return to home → Setting persists

4. **Cross-App Persistence**
   - [ ] Set builder to MINIMAL
   - [ ] Open companion-app → Should also show MINIMAL
   - [ ] Both apps share localStorage key: `crt-quality`

### Mobile Device Testing (Critical!)

#### iOS Safari (iPhone)
1. **Open on iPhone**
   - [ ] Visit builder/index.html (deploy or use ngrok/localhost)
   - [ ] Console shows: `[CRT] Auto mode - Low-tier device: MINIMAL quality`
   - [ ] Effects visible but lightweight (no bloom)
   - [ ] Smooth performance (30+ FPS)
   - [ ] Can override to HIGH if desired

#### Android Chrome
1. **Open on Android phone**
   - [ ] Same tests as iOS
   - [ ] Auto mode defaults to MINIMAL
   - [ ] User can change to OFF if needed

#### Tablet (iPad/Android)
1. **Open on tablet**
   - [ ] Console shows: `[CRT] Auto mode - Medium-tier device: MEDIUM quality`
   - [ ] Effects visible (barrel distortion but no bloom)
   - [ ] Smooth performance (40-50 FPS)

### Visual Quality Verification

#### OFF Mode
- [ ] Canvas element hidden (`display: none`)
- [ ] Solid amber background (#0B0900)
- [ ] No 3D rendering
- [ ] Zero GPU usage

#### MINIMAL Mode (Mobile)
- [ ] ✅ RetroPass pixelation (blocky, retro look)
- [ ] ✅ Scanlines (horizontal lines)
- [ ] ✅ Vignette (darkened edges)
- [ ] ❌ No barrel distortion (straight edges)
- [ ] ❌ No chromatic aberration (no RGB shift)
- [ ] ❌ No bloom (no glow)

#### MEDIUM Mode (Tablet)
- [ ] ✅ All MINIMAL effects
- [ ] ✅ Barrel distortion (CRT curve visible)
- [ ] ❌ No chromatic aberration
- [ ] ❌ No bloom

#### HIGH Mode (Desktop)
- [ ] ✅ All MEDIUM effects
- [ ] ✅ Chromatic aberration (subtle RGB separation)
- [ ] ✅ Bloom (amber glow around bright elements)

### DevTools Performance Testing

1. **Open Performance Monitor**
   - DevTools → More tools → Performance monitor
   - Watch GPU activity
   - Watch FPS

2. **Test Each Quality Level**
   ```
   OFF:      ~0% GPU, N/A FPS (not rendering)
   MINIMAL:  ~2-5% GPU, 30-60 FPS
   MEDIUM:   ~5-10% GPU, 40-60 FPS
   HIGH:     ~10-15% GPU, 50-60 FPS (desktop)
   ```

### Regression Testing (No Breaking Changes)

#### Builder App
- [ ] Face drag-and-drop still works
- [ ] Templates load correctly
- [ ] Build button functional
- [ ] 24H MODE toggle works
- [ ] LED sliders work
- [ ] All existing settings functional

#### Companion App
- [ ] TX mode transmits time sync
- [ ] RX mode decodes sleep data
- [ ] Navigation between modes works
- [ ] No console errors

### Browser Compatibility

- [ ] Chrome 120+ (desktop)
- [ ] Safari 16.4+ (desktop + iOS)
- [ ] Firefox 108+ (desktop)
- [ ] Edge 120+ (desktop)
- [ ] Chrome Android 120+
- [ ] Samsung Internet 23+

### LocalStorage Inspector

1. **Check stored value**
   - DevTools → Application → Local Storage
   - Look for key: `crt-quality`
   - Values: `auto`, `off`, `minimal`, `medium`, `high`

2. **Test manual override**
   - In Console: `localStorage.setItem('crt-quality', 'high')`
   - Reload page
   - HIGH quality applied
   - HIGH button highlighted

### Known Issues / Expected Behavior

1. **Reload Required**
   - Quality changes require page reload
   - Alert notifies user
   - This is intentional (reinitializing Three.js is complex)

2. **AUTO Mode Detection**
   - Uses `navigator.userAgent`, `deviceMemory`, `hardwareConcurrency`
   - Some devices may be misclassified
   - User can always override with manual selection

3. **Mobile Detection**
   - iPhones/iPads → AUTO defaults to MINIMAL
   - Android phones → AUTO defaults to MINIMAL
   - Android tablets → AUTO defaults to MEDIUM
   - User can force HIGH if device can handle it

### Success Criteria Summary

- ✅ UI appears in both apps
- ✅ All 5 quality levels work
- ✅ LocalStorage persists preference
- ✅ Mobile defaults to MINIMAL (not OFF)
- ✅ No existing features broken
- ✅ Visual quality matches spec
- ✅ Performance acceptable on target devices

---

## Quick Fix Commands

If something's wrong:

```bash
# Reset to default
localStorage.removeItem('crt-quality')
location.reload()

# Force specific quality
localStorage.setItem('crt-quality', 'minimal')
location.reload()

# Check current setting
console.log(localStorage.getItem('crt-quality'))
```

---

## Manual Testing Workflow

1. **Desktop Chrome** (5 min)
   - Test all 5 quality levels
   - Verify localStorage persistence
   - Check console logs

2. **iPhone Safari** (5 min)
   - Verify AUTO defaults to MINIMAL
   - Test manual override to HIGH
   - Check performance/smoothness

3. **Android Phone** (5 min)
   - Same as iPhone tests
   - Verify cross-device localStorage (different key space!)

4. **Tablet** (optional, 3 min)
   - Verify AUTO defaults to MEDIUM
   - Check visual quality

**Total testing time:** ~15-20 minutes

---

## Bug Reporting Template

If you find issues:

```markdown
**Device:** iPhone 14 Pro / Chrome 120 / macOS Sonoma
**Quality Setting:** MINIMAL
**Expected:** Scanlines visible
**Actual:** No scanlines, only pixelation
**Console Errors:** None
**localStorage value:** "minimal"
**Steps to reproduce:** 
1. Set to MINIMAL
2. Reload page
3. Check console for quality log
```

---

**Testing Guide Version:** 1.0  
**Last Updated:** 2026-02-19  
**Branch:** `feature/phase2-crt-shaders`
