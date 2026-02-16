Second Movement
===============

**Note:** This is dlorp's fork of second-movement. See upstream at [joeycastillo/second-movement](https://github.com/joeycastillo/second-movement).

## Fork Changes

### Tap-to-Wake Feature
Added crystal tap detection for instant display wake on Sensor Watch Pro boards:
- Hybrid implementation using motion wake (INT2/A4) + software tap polling
- Both crystal tap and wrist-raise wake the display from sleep
- Uses LIS2DW12 accelerometer's hardware tap detection (400Hz, Z-axis threshold 12)
- Power optimized: Low-Power Mode 1 (~45-90µA continuous draw)
- Graceful fallback on Green boards (no accelerometer)
- Documentation: TAP_TO_WAKE.md and TAP_TO_WAKE_SLEEP_MODE_ANALYSIS.md

**Hardware constraint:** LIS2DW12 tap interrupts can only route to INT1 (A3), which is not an RTC wake pin. Solution uses motion detection on INT2 (A4) to wake from STANDBY sleep, then polls tap register to distinguish tap events from wrist movement.

### Active Hours Feature
**What it means:** Active Hours define when you're awake and active. Outside these hours (your sleep window), the watch enters a low-power mode that prevents accidental display wakes from rolling over in bed.

**What changes during sleep hours:**
- **Motion wake disabled:** Wrist movement no longer triggers display wake (prevents false wakes from tossing/turning)
- **Tap-to-wake still works:** Crystal tap always wakes display (intentional gesture)
- **Button wake always works:** Physical buttons always wake display
- **Sleep tracking enabled:** Orientation changes logged for sleep quality metrics (Stream 4)
- **Smart alarm active:** Light sleep detection begins 15 min before alarm window (Stream 3)

**Default configuration:**
- Active Hours: 04:00 - 23:00 (awake)
- Sleep Window: 23:00 - 04:00 (sleeping)
- User-configurable via settings face (15-minute increments)

**Components:**
- **Stream 1 (Core Logic):** Sleep detection and motion wake suppression
- **Stream 2 (Settings UI):** User-configurable Active Hours in settings face
- **Stream 3 (Smart Alarm):** Up band-style alarm with light sleep detection
- **Stream 4 (Sleep Tracking):** Orientation logging for sleep quality (70 bytes, 7 nights)

**Power impact:**
- Deep sleep mode: Minimal power draw (accelerometer at 1.6Hz stationary mode)
- Sleep tracking overhead: Under 5µA (orientation logging only)
- Smart alarm pre-wake: Ramps to 45-90µA for 15 min before alarm window

**Persistence:**
Settings stored in BKUP[2] register (battery-backed RAM). Survives normal power cycles, resets on complete battery removal (standard Sensor Watch behavior).

---

This is a work-in-progress refactor of the Movement firmware for [Sensor Watch](https://www.sensorwatch.net).


Getting dependencies
-------------------------
You will need to install [the GNU Arm Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads/) to build projects for the watch. If you're using Debian or Ubuntu, it should be sufficient to `apt install gcc-arm-none-eabi`.

You will need to fetch the git submodules for this repository too, with `git submodule update --init --recursive` 


Building Second Movement
----------------------------
You can build the default watch firmware with:

```
make BOARD=board_type DISPLAY=display_type
```

where `board_type` is any of:
- sensorwatch_pro
- sensorwatch_green  
- sensorwatch_red (also known as Sensor Watch Lite)
- sensorwatch_blue

and `display_type` is any of:
- classic
- custom

Optionally you can set the watch time when building the firmware using `TIMESET=minute`. 

`TIMESET` can be defined as:
- `year` = Sets the year to the PC's
- `day` = Sets the default time down to the day (year, month, day)
- `minute` = Sets the default time down to the minute (year, month, day, hour, minute)


If you'd like to modify which faces are built and included in the firmware, edit `movement_config.h`. You will get a compilation error if you enable more faces than the watch can store.

Installing firmware to the watch
----------------------------
To install the firmware onto your Sensor Watch board, plug the watch into your USB port and double tap the tiny Reset button on the back of the board. You should see the LED light up red and begin pulsing. (If it does not, make sure you didn’t plug the board in upside down). Once you see the `WATCHBOOT` drive appear on your desktop, type `make install`. This will convert your compiled program to a UF2 file, and copy it over to the watch.

If you want to do this step manually, copy `/build/firmware.uf2` to your watch. 


Emulating the firmware
----------------------------
You may want to test out changes in the emulator first. To do this, you'll need to install [emscripten](https://emscripten.org/), then run:

```
emmake make BOARD=sensorwatch_red DISPLAY=classic
python3 -m http.server -d build-sim
```

Finally, visit [firmware.html](http://localhost:8000/firmware.html) to see your work.
