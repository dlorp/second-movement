# Phase Engine Integration Guide for Watch Face Developers

## Overview

The **Phase Engine** is a circadian rhythm tracking system that computes a real-time "phase score" (0-100) representing how well-aligned you are with your body's natural rhythms based on:

- **Time of day and season** (via location-specific homebase table)
- **Activity level** (step count, movement)
- **Environmental inputs** (temperature, light)

This guide shows you how to integrate the phase engine into your watch face.

---

## Quick Start

### 1. Enable Phase Engine in Your Face

In your watch face's `.c` file, include the phase engine header:

```c
#include "../../lib/phase/phase_engine.h"
```

### 2. Add Phase State to Your Face State

```c
typedef struct {
    // ... your existing state fields ...
    
#ifdef PHASE_ENGINE_ENABLED
    phase_state_t phase;  // Phase engine state (~64 bytes)
#endif
} my_face_state_t;
```

### 3. Initialize Phase Engine

In your `setup()` function:

```c
void my_face_setup(movement_settings_t *settings, uint8_t watch_face_index, void **context_ptr) {
    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(my_face_state_t));
        my_face_state_t *state = (my_face_state_t *)*context_ptr;
        memset(state, 0, sizeof(my_face_state_t));
        
#ifdef PHASE_ENGINE_ENABLED
        phase_engine_init(&state->phase);
#endif
    }
}
```

### 4. Compute Phase Score

Call `phase_compute()` periodically (e.g., every minute or when display updates):

```c
#ifdef PHASE_ENGINE_ENABLED
    // Get current time
    watch_date_time now = watch_rtc_get_date_time();
    uint8_t hour = now.unit.hour;
    uint16_t day_of_year = movement_get_day_of_year(now);
    
    // Get sensor inputs (or use defaults)
    uint16_t activity_level = 500;  // TODO: Get from movement tracker
    int16_t temp_c10 = 200;         // 20.0°C - TODO: Get from temp sensor
    uint16_t light_lux = 100;       // TODO: Get from light sensor
    
    // Compute phase score
    uint16_t phase_score = phase_compute(&state->phase,
                                         hour,
                                         day_of_year,
                                         activity_level,
                                         temp_c10,
                                         light_lux);
#endif
```

### 5. Display Phase Score

```c
#ifdef PHASE_ENGINE_ENABLED
    // Show phase score (0-100)
    char buf[16];
    sprintf(buf, "PH %3d", phase_score);
    watch_display_string(buf, 0);
#endif
```

---

## Complete Example: Minimal Phase Face

Here's a complete minimal watch face that displays the phase score:

```c
#include <stdlib.h>
#include <string.h>
#include "minimal_phase_face.h"
#include "watch.h"
#include "../../lib/phase/phase_engine.h"

typedef struct {
#ifdef PHASE_ENGINE_ENABLED
    phase_state_t phase;
#endif
    uint8_t last_update_minute;
} minimal_phase_state_t;

void minimal_phase_face_setup(movement_settings_t *settings, uint8_t watch_face_index, void **context_ptr) {
    (void) settings;
    (void) watch_face_index;
    
    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(minimal_phase_state_t));
        minimal_phase_state_t *state = (minimal_phase_state_t *)*context_ptr;
        memset(state, 0, sizeof(minimal_phase_state_t));
        
#ifdef PHASE_ENGINE_ENABLED
        phase_engine_init(&state->phase);
#endif
    }
}

void minimal_phase_face_activate(movement_settings_t *settings, void *context) {
    (void) settings;
    minimal_phase_state_t *state = (minimal_phase_state_t *)context;
    state->last_update_minute = 0xFF;  // Force update on first activate
}

bool minimal_phase_face_loop(movement_event_t event, movement_settings_t *settings, void *context) {
    minimal_phase_state_t *state = (minimal_phase_state_t *)context;
    
    switch (event.event_type) {
        case EVENT_ACTIVATE:
        case EVENT_TICK:
        {
            watch_date_time now = watch_rtc_get_date_time();
            
            // Update once per minute
            if (now.unit.minute != state->last_update_minute) {
                state->last_update_minute = now.unit.minute;
                
#ifdef PHASE_ENGINE_ENABLED
                uint8_t hour = now.unit.hour;
                uint16_t day_of_year = movement_get_day_of_year(now);
                
                // Simple defaults - replace with real sensor data
                uint16_t activity = 500;
                int16_t temp_c10 = 200;
                uint16_t light = 100;
                
                uint16_t phase_score = phase_compute(&state->phase,
                                                     hour,
                                                     day_of_year,
                                                     activity,
                                                     temp_c10,
                                                     light);
                
                // Display phase score
                char buf[16];
                sprintf(buf, "PH  %3d   ", phase_score);
                watch_display_string(buf, 0);
#else
                watch_display_string("PH  --    ", 0);
#endif
            }
            break;
        }
        case EVENT_MODE_BUTTON_UP:
            movement_move_to_next_face();
            return false;
        default:
            break;
    }
    
    return true;
}

void minimal_phase_face_resign(movement_settings_t *settings, void *context) {
    (void) settings;
    (void) context;
}
```

---

## Advanced Features

### Get Phase Trend

Track whether your phase is improving or declining:

```c
// Get trend over last 6 hours (-100 to +100)
int16_t trend = phase_get_trend(&state->phase, 6);

if (trend > 20) {
    // Improving significantly
} else if (trend < -20) {
    // Declining significantly
} else {
    // Stable
}
```

### Get Recommendations

Get context-aware suggestions based on phase score:

```c
uint8_t recommendation = phase_get_recommendation(phase_score, hour);

switch (recommendation) {
    case 0:
        watch_display_string("REST", 4);
        break;
    case 1:
        watch_display_string("LITE", 4);
        break;
    case 2:
        watch_display_string("ACT ", 4);
        break;
    case 3:
        watch_display_string("PEAK", 4);
        break;
}
```

### Display Phase Score with Visual Indicator

```c
// Show phase score with graphical bars
void display_phase_score(uint16_t phase_score) {
    char buf[16];
    
    // Main score
    sprintf(buf, "PH %3d", phase_score);
    watch_display_string(buf, 0);
    
    // Visual indicator (0-4 bars)
    uint8_t bars = (phase_score + 12) / 25;  // 0-100 -> 0-4
    
    // Use indicator segments to show bars
    if (bars >= 1) watch_set_indicator(WATCH_INDICATOR_SIGNAL);
    if (bars >= 2) watch_set_indicator(WATCH_INDICATOR_BELL);
    if (bars >= 3) watch_set_indicator(WATCH_INDICATOR_LAP);
    if (bars >= 4) watch_set_indicator(WATCH_INDICATOR_PM);
}
```

---

## Getting Sensor Data

### Activity Level

If you're tracking steps or movement:

```c
// Example: Use step count as activity proxy
uint16_t activity_level = movement_get_step_count_today();

// Or use a recent window (e.g., last hour)
uint16_t activity_level = movement_get_step_count_last_hour() * 10;
```

### Temperature

If your watch has a temperature sensor:

```c
// From thermistor driver
int16_t temp_c = thermistor_driver_get_temperature();
int16_t temp_c10 = temp_c * 10;  // Convert to required format
```

### Light Level

If your watch has a light sensor:

```c
// From light sensor driver
uint16_t light_lux = light_sensor_read();
```

### Defaults When Sensors Unavailable

```c
// Reasonable defaults for basic operation
uint16_t activity_level = 500;    // Moderate activity
int16_t temp_c10 = 200;           // 20°C / 68°F
uint16_t light_lux = 100;         // Dim indoor lighting
```

The phase engine will still provide useful scores based on time and season alone.

---

## Display Patterns

### Simple Score Display

```
 PH  75
```

### Score with Trend Arrow

```c
char buf[16];
int16_t trend = phase_get_trend(&state->phase, 3);
char arrow = (trend > 10) ? '^' : (trend < -10) ? 'v' : '-';
sprintf(buf, "PH %3d %c", phase_score, arrow);
```

```
 PH  75 ^  (improving)
 PH  42 v  (declining)
 PH  60 -  (stable)
```

### Score with Recommendation

```c
const char *rec_str[] = {"RST", "MOD", "ACT", "PEK"};
uint8_t rec = phase_get_recommendation(phase_score, hour);
sprintf(buf, "PH%3d %s", phase_score, rec_str[rec]);
```

```
 PH 85 PEK  (peak performance time)
 PH 35 RST  (rest recommended)
```

---

## Optimization Tips

### Update Frequency

- **Every minute**: Good balance for most faces
- **Every 5 minutes**: More power-efficient for simple displays
- **On demand**: Only when user navigates to face (most efficient)

### RAM Usage

The `phase_state_t` structure uses approximately **64 bytes**. This is small enough for most watch faces, but if you're very RAM-constrained:

```c
// Option 1: Share state across multiple faces (advanced)
extern phase_state_t global_phase_state;

// Option 2: Disable hourly history to save ~30 bytes
// (Trend calculation will be limited)
```

### Flash Usage

The homebase table adds approximately **2.2 KB** to firmware. This is shared across all faces that use the phase engine.

---

## Testing Without Homebase Table

During development, you can test even without generating a homebase table:

```c
#ifdef PHASE_ENGINE_ENABLED
    // This will work even if homebase table isn't generated yet
    // (Engine provides sensible defaults)
    uint16_t score = phase_compute(&state->phase, hour, day_of_year,
                                   activity, temp, light);
#endif
```

To generate a homebase table for your location:

```bash
# From repo root
python3 utils/generate_homebase.py \
    --lat 61.2181 \
    --lon -149.9003 \
    --tz AKST \
    --year 2026
```

---

## API Reference

### `phase_engine_init(phase_state_t *state)`

Initialize phase engine state. Call once during face setup.

**Parameters:**
- `state`: Pointer to phase state structure (allocated by you)

**Returns:** None

---

### `phase_compute(state, hour, day_of_year, activity_level, temp_c10, light_lux)`

Compute current phase score.

**Parameters:**
- `state`: Phase engine state (updated in-place)
- `hour`: Current hour (0-23)
- `day_of_year`: Day of year (1-365)
- `activity_level`: Recent activity (0-1000, arbitrary units)
- `temp_c10`: Current temperature (celsius × 10, e.g., 205 = 20.5°C)
- `light_lux`: Current light level (lux)

**Returns:** Phase score (0-100)
- 0-25: Poor alignment, rest recommended
- 26-50: Below average, light activity okay
- 51-75: Good alignment, normal activity
- 76-100: Excellent alignment, peak performance

---

### `phase_get_trend(state, hours)`

Get phase trend over recent hours.

**Parameters:**
- `state`: Phase engine state
- `hours`: Number of hours to analyze (1-24)

**Returns:** Trend value (-100 to +100)
- Positive: Phase improving
- Negative: Phase declining
- ~0: Stable

---

### `phase_get_recommendation(phase_score, hour)`

Get recommended action based on phase.

**Parameters:**
- `phase_score`: Current phase score (0-100)
- `hour`: Current hour (0-23)

**Returns:** Action code
- 0: Rest (low phase or late night)
- 1: Moderate activity
- 2: Active (normal work/exercise)
- 3: Peak performance (optimal for challenging tasks)

---

## Troubleshooting

### Phase score always returns 0

- Ensure `PHASE_ENGINE_ENABLED` is defined in your build
- Check that homebase table has been generated
- Verify `phase_engine_init()` was called

### Compilation errors about missing homebase_table.h

Generate the homebase table:

```bash
python3 utils/generate_homebase.py --lat YOUR_LAT --lon YOUR_LON --tz YOUR_TZ --year 2026
```

### Score seems inaccurate

- Verify sensor inputs are in correct units
- Check that time and day_of_year are accurate
- Make sure homebase table matches your actual location

---

## Examples in the Wild

Check these watch faces for real-world integration examples:

- `circadian_face.c` - Full-featured phase tracking face
- `wellness_face.c` - Wellness metrics including phase score
- `optimal_face.c` - Minimalist phase-aware time display

---

## Questions?

For more details, see:
- `lib/phase/README.md` - Phase engine overview
- `lib/phase/phase_engine.h` - Complete API documentation
- `lib/phase/phase_engine.c` - Implementation details

Happy building! 🌙✨
