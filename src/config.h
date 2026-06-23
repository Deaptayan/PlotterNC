#ifndef PLOTTER_CONFIG_H
#define PLOTTER_CONFIG_H

// Portable machine settings. Keep board pin mappings in hal/hal_esp32.h.

#define MAX_FEEDRATE_MM_MIN 3000.0f
#define MAX_ACCELERATION_MM_S2 500.0f

#define X_STEPS_PER_MM 80.0f
#define Y_STEPS_PER_MM 80.0f
#define Z_STEPS_PER_MM 400.0f

#define DEFAULT_PEN_UP_Z_MM 5.0f
#define DEFAULT_PEN_DOWN_Z_MM 0.0f

#define PEN_UP_SERVO_US 1000
#define PEN_DOWN_SERVO_US 1800
#define PEN_SERVO_REFRESH_US 20000

#define X_TRAVEL_MM 220.0f
#define Y_TRAVEL_MM 220.0f
#define Z_TRAVEL_MM 50.0f

#define HOMING_FEEDRATE_MM_MIN 100.0f

#define PLANNER_QUEUE_CAPACITY 16

#endif
