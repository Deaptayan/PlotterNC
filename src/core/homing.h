#ifndef PLOTTER_HOMING_H
#define PLOTTER_HOMING_H

#include "../drivers/drv_motor.h"

enum HomingState {
    HOMING_IDLE,
    HOMING_RUNNING,
    HOMING_COMPLETE,
    HOMING_FAILED
};

void homing_init();
void homing_start();
void homing_start_axis(MotorAxis axis);
void homing_task();
HomingState homing_state();

#endif
