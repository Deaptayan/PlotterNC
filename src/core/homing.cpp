#include "homing.h"

#include "planner.h"
#include "../drivers/drv_endstop.h"
#include "../drivers/drv_motor.h"

static HomingState state = HOMING_IDLE;
static uint8_t active_axes = 0;

static uint8_t axis_mask(MotorAxis axis) {
    return (uint8_t)(1 << axis);
}

void homing_init() {
    drv_endstop_init();
    state = HOMING_IDLE;
    active_axes = 0;
}

void homing_start() {
    active_axes = axis_mask(MOTOR_AXIS_X) | axis_mask(MOTOR_AXIS_Y) | axis_mask(MOTOR_AXIS_Z);
    state = HOMING_RUNNING;
}

void homing_start_axis(MotorAxis axis) {
    if (axis < MOTOR_AXIS_X || axis >= MOTOR_AXIS_COUNT) {
        return;
    }

    active_axes = axis_mask(axis);
    state = HOMING_RUNNING;
}

void homing_task() {
    if (state != HOMING_RUNNING) {
        return;
    }

    drv_motor_set_all_enabled(true);

    bool complete = true;

    if ((active_axes & axis_mask(MOTOR_AXIS_X)) && !drv_endstop_is_triggered(MOTOR_AXIS_X)) {
        drv_motor_set_direction(MOTOR_AXIS_X, false);
        drv_motor_step(MOTOR_AXIS_X);
        complete = false;
    }
    if ((active_axes & axis_mask(MOTOR_AXIS_Y)) && !drv_endstop_is_triggered(MOTOR_AXIS_Y)) {
        drv_motor_set_direction(MOTOR_AXIS_Y, false);
        drv_motor_step(MOTOR_AXIS_Y);
        complete = false;
    }
    if ((active_axes & axis_mask(MOTOR_AXIS_Z)) && !drv_endstop_is_triggered(MOTOR_AXIS_Z)) {
        drv_motor_set_direction(MOTOR_AXIS_Z, false);
        drv_motor_step(MOTOR_AXIS_Z);
        complete = false;
    }

    if (complete) {
        if (active_axes & axis_mask(MOTOR_AXIS_X)) {
            planner_set_axis_position(MOTOR_AXIS_X, 0.0f);
        }
        if (active_axes & axis_mask(MOTOR_AXIS_Y)) {
            planner_set_axis_position(MOTOR_AXIS_Y, 0.0f);
        }
        if (active_axes & axis_mask(MOTOR_AXIS_Z)) {
            planner_set_axis_position(MOTOR_AXIS_Z, 0.0f);
        }
        active_axes = 0;
        state = HOMING_COMPLETE;
    }
}

HomingState homing_state() {
    return state;
}
