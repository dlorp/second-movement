# Tap-to-Wake Feature

## Overview
Tap-to-wake enables instant display wake by tapping the watch crystal. This feature uses the LIS2DW12 accelerometer's dedicated tap detection hardware on Sensor Watch Pro boards.

## How It Works

### Hardware
- **Accelerometer:** LIS2DW12 (Pro board only)
- **Detection:** Hardware-based tap recognition on Z-axis
- **Interrupt:** Routed to INT1 (pin A3)
- **Sampling rate:** 400Hz during tap detection
- **Modes:** Single tap and double tap both supported

### Software
When a tap is detected:
1. Accelerometer generates interrupt on INT1 (A3)
2. `cb_accelerometer_event()` callback sets `has_pending_accelerometer` flag
3. Main loop calls `_movement_get_accelerometer_events()`
4. Tap events (SINGLE_TAP or DOUBLE_TAP) are generated
5. `_movement_reset_inactivity_countdown()` wakes display immediately
6. Events are passed to current watch face (can be handled by faces if desired)

## Power Consumption

**Active tap detection:**
- Accelerometer: ~400µA (400Hz sampling, low power mode)
- Always-on (tap detection enabled at boot)

**Trade-off:**
- Higher baseline power vs motion-only detection
- Instant response (<100ms) vs wrist-raise delay (~500ms)
- Crystal tap = intuitive UX gesture

## Configuration

### Tap Sensitivity
Configured in `movement_enable_tap_detection_if_available()`:
```c
// Z-axis threshold: 12 (range 0-31)
lis2dw_configure_tap_threshold(0, 0, 12, LIS2DW_REG_TAP_THS_Z_Z_AXIS_ENABLE);

// Tap duration: shock(2), quiet(2), latency(2)
lis2dw_configure_tap_duration(2, 2, 2);
```

**Tuning:**
- Increase threshold (12 → 15+) for firmer taps required
- Decrease threshold (12 → 8-10) for more sensitive detection
- Adjust durations to distinguish tap from sustained pressure

### Disabling Tap Detection
Watch faces can disable tap detection to save power:
```c
movement_disable_tap_detection_if_available();
```

Re-enable when returning to normal mode:
```c
movement_enable_tap_detection_if_available();
```

## Board Support

### Sensor Watch Pro
✅ **Full support** - Has LIS2DW12 accelerometer
- Crystal tap → instant wake
- Both single and double tap detected
- ~400µA additional power draw

### Sensor Watch Green
⚠️ **No accelerometer** - Tap detection not available
- `movement_enable_tap_detection_if_available()` returns false
- Falls back to button-only wake
- No additional power consumption

## User Experience

### Tap-to-Wake Flow
```
User taps crystal
    ↓
<100ms response time
    ↓
Display wakes
    ↓
Current face visible
    ↓
Inactivity timer reset (won't sleep immediately)
```

### Coexistence with Other Features
Tap detection works alongside:
- **Motion detection** (INT2/A4) - For wrist-raise wake (if enabled)
- **Button wake** - Always available as fallback
- **Step counting** - Can share accelerometer (different data rate/FIFO)

### Integration with Watch Faces
Watch faces receive tap events and can:
- Ignore taps (default - just wake display)
- Handle EVENT_SINGLE_TAP for actions
- Handle EVENT_DOUBLE_TAP for different actions
- See `ping_face.c`, `countdown_face.c`, etc. for examples

## Implementation Details

### Initialization (movement.c)
```c
// During app_init(), after accelerometer basic setup:
lis2dw_enable_interrupts();
movement_enable_tap_detection_if_available(); // ← NEW
```

### Event Generation (_movement_get_accelerometer_events)
```c
uint8_t int_src = lis2dw_get_interrupt_source();

if (int_src & LIS2DW_REG_ALL_INT_SRC_DOUBLE_TAP) {
    accelerometer_events |= 1 << EVENT_DOUBLE_TAP;
    _movement_reset_inactivity_countdown(); // ← Wake display
}

if (int_src & LIS2DW_REG_ALL_INT_SRC_SINGLE_TAP) {
    accelerometer_events |= 1 << EVENT_SINGLE_TAP;
    _movement_reset_inactivity_countdown(); // ← Wake display
}
```

## Testing

### Emulator
The web emulator doesn't simulate tap detection (no accelerometer model). Test on actual hardware.

### Hardware Testing
1. Flash `firmware-sensorwatch_pro-classic-tap-to-wake.uf2` to Pro board
2. Let display sleep (wait for inactivity timeout)
3. Tap crystal firmly with fingertip
4. Display should wake immediately

**Troubleshooting:**
- Too sensitive? → Increase threshold in `lis2dw_configure_tap_threshold()`
- Not sensitive enough? → Decrease threshold or check Z-axis orientation
- No response? → Verify INT1 (A3) connection, check with printf debugging

## Future Enhancements

Potential additions:
- [ ] Configurable tap sensitivity via settings face
- [ ] Single vs double tap preference (disable one to save cycles)
- [ ] Tap-only mode (disable motion detection, rely on tap + buttons)
- [ ] Tap gesture recognition (double-tap → navigate, triple-tap → light)
- [ ] Sleep mode tap disable (reduce power when sleeping overnight)

## References

- **LIS2DW12 Datasheet:** https://www.st.com/en/mems-and-sensors/lis2dw12.html
- **Application Note AN5038:** Recommendations for 2g range, tap detection config
- **second-movement movement.h:** Event type definitions (EVENT_SINGLE_TAP, EVENT_DOUBLE_TAP)
- **Step counter PR #9:** Shows FIFO usage and motion detection coexistence
