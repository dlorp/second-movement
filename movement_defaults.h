// movement_defaults.h - First-boot runtime defaults
// These values initialize BKUP registers on first boot only.
// After first boot, user changes are preserved across resets.
// Override any value by passing -DMOVEMENT_DEFAULT_* to the compiler.
#ifndef MOVEMENT_DEFAULTS_H_
#define MOVEMENT_DEFAULTS_H_

// BKUP[1]: Location (hundredths of a degree, int16_t range -18000 to 18000)
// Default 0,0 means no location pre-configured (user sets via watch face).
#ifndef MOVEMENT_DEFAULT_LATITUDE
#define MOVEMENT_DEFAULT_LATITUDE 0
#endif

#ifndef MOVEMENT_DEFAULT_LONGITUDE
#define MOVEMENT_DEFAULT_LONGITUDE 0
#endif

// BKUP[2]: Active hours (quarter-hour increments, 0-95; 0=00:00, 4=01:00, 92=23:00)
#ifndef MOVEMENT_DEFAULT_ACTIVE_HOURS_START
#define MOVEMENT_DEFAULT_ACTIVE_HOURS_START 16  // 04:00
#endif

#ifndef MOVEMENT_DEFAULT_ACTIVE_HOURS_END
#define MOVEMENT_DEFAULT_ACTIVE_HOURS_END 92    // 23:00
#endif

#ifndef MOVEMENT_DEFAULT_ACTIVE_HOURS_ENABLED
#define MOVEMENT_DEFAULT_ACTIVE_HOURS_ENABLED true
#endif

#endif // MOVEMENT_DEFAULTS_H_
