#ifndef PLOTTER_PLANNER_H
#define PLOTTER_PLANNER_H

#include <stdint.h>

#include "../drivers/drv_motor.h"

struct PlannerMove {
    float x_mm;
    float y_mm;
    float z_mm;
    float feedrate_mm_min;
};

void planner_init();
bool planner_enqueue_move(const PlannerMove *move);
bool planner_has_work();
unsigned int planner_buffer_available();
void planner_pause();
void planner_resume();
void planner_stop();
bool planner_is_paused();
float planner_steps_per_mm(MotorAxis axis);
bool planner_set_steps_per_mm(MotorAxis axis, float steps_per_mm);
bool planner_calibrate_steps_per_mm(MotorAxis axis, float commanded_mm, float measured_mm, float *updated_steps_per_mm);
void planner_task();
void planner_current_position(float *x_mm, float *y_mm, float *z_mm);
void planner_set_position(float x_mm, float y_mm, float z_mm);
void planner_set_axis_position(MotorAxis axis, float position_mm);

#endif
