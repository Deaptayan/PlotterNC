#include "drv_motor.h"

#include "../hal/hal.h"
#include "../hal/hal_platform.h"

static const MotorPins motor_pins[MOTOR_AXIS_COUNT] = {
    {X_IN1_PIN, X_IN2_PIN, X_IN3_PIN, X_IN4_PIN},
    {Y_IN1_PIN, Y_IN2_PIN, Y_IN3_PIN, Y_IN4_PIN},
    {Z_IN1_PIN, Z_IN2_PIN, Z_IN3_PIN, Z_IN4_PIN}
};

static const uint8_t half_step_sequence[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
};

static uint8_t motor_phase[MOTOR_AXIS_COUNT] = {0, 0, 0};
static bool motor_direction[MOTOR_AXIS_COUNT] = {true, true, true};
static bool motor_enabled[MOTOR_AXIS_COUNT] = {false, false, false};

static bool valid_axis(MotorAxis axis) {
    return axis >= MOTOR_AXIS_X && axis < MOTOR_AXIS_COUNT;
}

static void write_coils(MotorAxis axis, uint8_t phase) {
    const MotorPins *pins = &motor_pins[axis];
    hal_gpio_write(pins->in1_pin, half_step_sequence[phase][0] ? HAL_HIGH : HAL_LOW);
    hal_gpio_write(pins->in2_pin, half_step_sequence[phase][1] ? HAL_HIGH : HAL_LOW);
    hal_gpio_write(pins->in3_pin, half_step_sequence[phase][2] ? HAL_HIGH : HAL_LOW);
    hal_gpio_write(pins->in4_pin, half_step_sequence[phase][3] ? HAL_HIGH : HAL_LOW);
}

static void release_coils(MotorAxis axis) {
    const MotorPins *pins = &motor_pins[axis];
    hal_gpio_write(pins->in1_pin, HAL_LOW);
    hal_gpio_write(pins->in2_pin, HAL_LOW);
    hal_gpio_write(pins->in3_pin, HAL_LOW);
    hal_gpio_write(pins->in4_pin, HAL_LOW);
}

void drv_motor_init() {
    for (uint8_t i = 0; i < MOTOR_AXIS_COUNT; ++i) {
        hal_gpio_mode(motor_pins[i].in1_pin, HAL_PIN_OUTPUT);
        hal_gpio_mode(motor_pins[i].in2_pin, HAL_PIN_OUTPUT);
        hal_gpio_mode(motor_pins[i].in3_pin, HAL_PIN_OUTPUT);
        hal_gpio_mode(motor_pins[i].in4_pin, HAL_PIN_OUTPUT);
        motor_phase[i] = 0;
        motor_direction[i] = true;
        drv_motor_enable(static_cast<MotorAxis>(i), false);
    }
}

void drv_motor_enable(MotorAxis axis, bool enabled) {
    if (!valid_axis(axis)) {
        return;
    }

    motor_enabled[axis] = enabled;
    if (enabled) {
        write_coils(axis, motor_phase[axis]);
    } else {
        release_coils(axis);
    }
}

void drv_motor_set_direction(MotorAxis axis, bool positive) {
    if (!valid_axis(axis)) {
        return;
    }

    motor_direction[axis] = positive;
}

void drv_motor_step(MotorAxis axis) {
    if (!valid_axis(axis)) {
        return;
    }

    if (!motor_enabled[axis]) {
        drv_motor_enable(axis, true);
    }

    if (motor_direction[axis]) {
        motor_phase[axis] = (motor_phase[axis] + 1) & 0x07;
    } else {
        motor_phase[axis] = (motor_phase[axis] + 7) & 0x07;
    }

    write_coils(axis, motor_phase[axis]);
}

void drv_motor_set_all_enabled(bool enabled) {
    for (uint8_t i = 0; i < MOTOR_AXIS_COUNT; ++i) {
        drv_motor_enable(static_cast<MotorAxis>(i), enabled);
    }
}
