# Sleep Tracker Integration Guide

This document explains how to integrate the sleep tracking algorithm with the existing Active Hours system in `movement.c`.

---

## Integration Points

### 1. Add Sleep Tracker State to Movement State

```c
// In movement.c

#include "sleep_tracker_face.h"

static sleep_tracker_state_t global_sleep_tracker = {0};
```

### 2. Initialize Sleep Tracker

```c
// In movement_setup()

void movement_setup(void) {
    // Existing setup code...
    
    // Initialize sleep tracker with default modifiers
    memcpy(global_sleep_tracker.light_modifiers, 
           (int16_t[]){-200, -50, +100, +400}, 
           sizeof(global_sleep_tracker.light_modifiers));
}
```

### 3. Hook Into Active Hours Sleep Detection

```c
// In cb_accelerometer_wake() - called every minute during sleep

static void cb_accelerometer_wake(void) {
    // Existing code...
    
    if (is_confirmed_asleep() && !is_approaching_alarm_window()) {
        // We're in confirmed sleep - run sleep detection
        
        // 1. Get activity count from LIS2DW12
        uint16_t activity_count = lis2dw12_get_interrupt_count_and_reset();
        
        // 2. Read light sensor
        #ifdef HAS_IR_SENSOR
            uint8_t light_level = adc_get_analog_value(HAL_GPIO_IRSENSE_pin());
        #else
            uint8_t light_level = 0;  // Assume dark if no sensor
        #endif
        
        // 3. Classify epoch
        bool is_asleep = sleep_tracker_classify_epoch(&global_sleep_tracker, 
                                                       activity_count, 
                                                       light_level);
        
        // 4. Update metrics
        sleep_tracker_update_metrics(&global_sleep_tracker, is_asleep);
        
        // 5. Stream 4: Log orientation if asleep (existing code)
        if (is_asleep) {
            uint8_t orientation = get_current_orientation();
            sleep_tracking_log_orientation(orientation);
        }
    }
}
```

### 4. Session Management with Active Hours

```c
// In active_hours_state_change()

static void active_hours_state_change(bool entering_sleep_window) {
    if (entering_sleep_window) {
        // Entering sleep window (e.g., 23:00)
        // Start a new sleep tracking session
        sleep_tracker_start_session(&global_sleep_tracker);
        
    } else {
        // Leaving sleep window (e.g., 04:00)
        // End sleep tracking session
        sleep_tracker_end_session(&global_sleep_tracker);
        
        // Optionally: Save session data for Circadian Score calculation
        save_sleep_session_data(&global_sleep_tracker);
    }
}
```

### 5. Export Sleep Data for Circadian Score

```c
// Export function for Circadian Score calculation

typedef struct {
    uint16_t total_sleep_minutes;
    uint16_t sleep_efficiency;  // 0-100%
    uint16_t waso_minutes;
    uint8_t num_awakenings;
    uint8_t light_exposure_quality;  // % time in darkness
    uint32_t sleep_onset_time;
    uint32_t sleep_offset_time;
} sleep_session_export_t;

sleep_session_export_t get_last_sleep_session(void) {
    sleep_session_export_t export = {0};
    
    export.total_sleep_minutes = global_sleep_tracker.total_sleep_minutes;
    export.sleep_efficiency = sleep_tracker_calculate_efficiency(&global_sleep_tracker);
    export.waso_minutes = global_sleep_tracker.total_wake_minutes;
    export.num_awakenings = global_sleep_tracker.num_awakenings;
    
    // Calculate light exposure quality (% time in darkness)
    if (global_sleep_tracker.total_sleep_minutes > 0) {
        export.light_exposure_quality = 
            (global_sleep_tracker.total_dark_minutes * 100) / 
            global_sleep_tracker.total_sleep_minutes;
    }
    
    export.sleep_onset_time = global_sleep_tracker.sleep_onset_time;
    export.sleep_offset_time = global_sleep_tracker.sleep_offset_time;
    
    return export;
}
```

### 6. Integration with Smart Alarm

```c
// In smart_alarm_face.c - check if user is in light sleep

bool is_in_light_sleep(void) {
    // Check recent epochs for wake/sleep pattern
    // Light sleep = more frequent transitions
    
    if (global_sleep_tracker.current_epoch < 10) return false;
    
    uint8_t wake_count = 0;
    for (int i = 1; i <= 10; i++) {
        uint16_t epoch = global_sleep_tracker.current_epoch - i;
        if (!sleep_tracker_get_sleep_bit(&global_sleep_tracker, epoch)) {
            wake_count++;
        }
    }
    
    // Light sleep proxy: 2-4 wake epochs in last 10 minutes
    return (wake_count >= 2 && wake_count <= 4);
}
```

---

## Power Optimization

### LIS2DW12 Configuration for Sleep Tracking

```c
void configure_sleep_tracking_mode(void) {
    // Set to Low-Power Mode 1 (45-90µA)
    lis2dw12_power_mode_set(LIS2DW12_LOW_POWER_MODE_1);
    
    // Configure wake-on-motion interrupt on INT2
    lis2dw12_int2_route_t int2_route = {0};
    int2_route.int2_wu = 1;  // Wake-up interrupt
    lis2dw12_int2_route_set(&int2_route);
    
    // Set wake-up threshold (tune for sensitivity)
    lis2dw12_wkup_threshold_set(3);  // ~47mg threshold
    
    // Set wake-up duration (number of samples above threshold)
    lis2dw12_wkup_dur_set(1);
    
    // Enable activity count mode
    lis2dw12_act_mode_set(LIS2DW12_DETECT_ACT_INACT);
}
```

### Light Sensor Sampling

```c
// Read light sensor once per epoch (negligible power)
uint8_t read_light_sensor_efficient(void) {
    #ifdef HAS_IR_SENSOR
        // Enable IR sensor power
        HAL_GPIO_IR_ENABLE_clr();
        
        // Wait 1ms for stabilization
        delay_ms(1);
        
        // Single ADC read
        uint8_t light = adc_get_analog_value(HAL_GPIO_IRSENSE_pin());
        
        // Disable IR sensor power
        HAL_GPIO_IR_ENABLE_set();
        
        return light;
    #else
        return 0;  // No sensor, assume dark
    #endif
}
```

---

## Data Storage

### Circular Buffer for 8 Hours

```c
// Storage: 60 bytes for sleep/wake log (480 bits)
// Plus 22 bytes for state = 82 bytes total

// For 7-day history (for Circadian Score SRI calculation)
typedef struct {
    sleep_session_export_t sessions[7];  // 7 nights
    uint8_t current_index;  // Circular buffer index
} sleep_history_t;

// Store in BKUP registers or flash
```

---

## Validation Hooks

### Export Raw Data for Analysis

```c
// For validation testing: export raw sleep/wake log

void export_sleep_log_json(sleep_tracker_state_t *state) {
    // Output format for companion app analysis
    printf("{\"epochs\":[");
    
    for (int i = 0; i < state->current_epoch; i++) {
        bool asleep = sleep_tracker_get_sleep_bit(state, i);
        printf("%d%s", asleep ? 1 : 0, (i < state->current_epoch - 1) ? "," : "");
    }
    
    printf("],\"metrics\":{");
    printf("\"sleep_minutes\":%d,", state->total_sleep_minutes);
    printf("\"efficiency\":%d,", sleep_tracker_calculate_efficiency(state));
    printf("\"awakenings\":%d", state->num_awakenings);
    printf("}}\n");
}
```

---

## Testing Procedure

### 1. Unit Testing (No Hardware)

```c
void test_cole_kripke_classification(void) {
    sleep_tracker_state_t state = {0};
    memcpy(state.light_modifiers, DEFAULT_LIGHT_MODIFIERS, sizeof(DEFAULT_LIGHT_MODIFIERS));
    
    // Test case 1: Restful sleep (low activity, dark)
    bool result1 = sleep_tracker_classify_epoch(&state, 1, 5);  // activity=1, light=5
    assert(result1 == true);  // Should classify as SLEEP
    
    // Test case 2: Phone in bed (low activity, bright)
    bool result2 = sleep_tracker_classify_epoch(&state, 2, 150);  // activity=2, light=150
    assert(result2 == false);  // Should classify as WAKE
    
    // Test case 3: Tossing and turning (high activity, dark)
    bool result3 = sleep_tracker_classify_epoch(&state, 15, 3);  // activity=15, light=3
    assert(result3 == false);  // Should classify as WAKE
}
```

### 2. Hardware Testing

1. **Wear watch overnight**
2. **Keep sleep diary** (manual log of when you think you fell asleep/woke)
3. **Export raw data** via USB/UART or companion app
4. **Compare** watch classification vs. diary
5. **Tune light_modifiers** if needed

### 3. Validation Metrics

```c
// Compare against ground truth (diary or another device)
typedef struct {
    uint16_t true_positives;   // Correctly detected sleep
    uint16_t true_negatives;   // Correctly detected wake
    uint16_t false_positives;  // Incorrectly detected sleep (was awake)
    uint16_t false_negatives;  // Incorrectly detected wake (was asleep)
} validation_metrics_t;

void calculate_validation_metrics(sleep_tracker_state_t *watch_data,
                                  sleep_tracker_state_t *ground_truth,
                                  validation_metrics_t *metrics) {
    // Compare epoch-by-epoch
    for (int i = 0; i < MAX_SLEEP_EPOCHS; i++) {
        bool watch_sleep = sleep_tracker_get_sleep_bit(watch_data, i);
        bool truth_sleep = sleep_tracker_get_sleep_bit(ground_truth, i);
        
        if (watch_sleep && truth_sleep) metrics->true_positives++;
        else if (!watch_sleep && !truth_sleep) metrics->true_negatives++;
        else if (watch_sleep && !truth_sleep) metrics->false_positives++;
        else if (!watch_sleep && truth_sleep) metrics->false_negatives++;
    }
    
    // Calculate accuracy, sensitivity, specificity
    float accuracy = (float)(metrics->true_positives + metrics->true_negatives) / MAX_SLEEP_EPOCHS;
    float sensitivity = (float)metrics->true_positives / (metrics->true_positives + metrics->false_negatives);
    float specificity = (float)metrics->true_negatives / (metrics->true_negatives + metrics->false_positives);
    
    printf("Accuracy: %.2f%%\n", accuracy * 100);
    printf("Sensitivity: %.2f%%\n", sensitivity * 100);
    printf("Specificity: %.2f%%\n", specificity * 100);
}
```

---

## Next Steps

1. **Add to movement.c** - Integrate hooks as shown above
2. **Enable in build** - Add `sleep_tracker_face.c` to watch face list
3. **Flash to hardware** - Test on Sensor Watch Pro
4. **Validate with sleep diary** - 7 nights minimum
5. **Tune light modifiers** if accuracy < 85%
6. **Export to Circadian Score** - Feed metrics to scoring algorithm

---

## Expected Performance

| Metric | Target | Notes |
|--------|--------|-------|
| Sleep/wake accuracy | >85% | Validated against diary |
| Sleep duration bias | <30 min | Mean error vs ground truth |
| Sensitivity (detect sleep) | >88% | Cole-Kripke baseline |
| Specificity (detect wake) | >45% | Enhanced by light sensor |
| Power consumption | <5µA | LIS2DW12 LP1 + light ADC |
| RAM usage | 82 bytes | Circular buffer + state |

---

**The implementation is complete and ready for testing.** 🫡
