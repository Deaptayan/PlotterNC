#ifndef PLOTTER_DRV_MOTOR_H
#define PLOTTER_DRV_MOTOR_H

#include <stdint.h>

enum MotorAxis {
    MOTOR_AXIS_X = 0,
    MOTOR_AXIS_Y = 1,
    MOTOR_AXIS_Z = 2,
    MOTOR_AXIS_COUNT = 3
};

struct MotorPins {
    uint8_t in1_pin;
    uint8_t in2_pin;
    uint8_t in3_pin;
    uint8_t in4_pin;
};

void drv_motor_init();
void drv_motor_enable(MotorAxis axis, bool enabled);
void drv_motor_set_direction(MotorAxis axis, bool positive);
void drv_motor_step(MotorAxis axis);
void drv_motor_set_all_enabled(bool enabled);

#endif
