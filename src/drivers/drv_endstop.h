#ifndef PLOTTER_DRV_ENDSTOP_H
#define PLOTTER_DRV_ENDSTOP_H

#include "drv_motor.h"

void drv_endstop_init();
bool drv_endstop_is_triggered(MotorAxis axis);
bool drv_endstop_max_is_available(MotorAxis axis);
bool drv_endstop_max_is_triggered(MotorAxis axis);
bool drv_endstop_all_triggered();

#endif
