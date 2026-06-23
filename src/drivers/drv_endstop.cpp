#include "drv_endstop.h"

#include "../hal/hal.h"
#include "../hal/hal_platform.h"

static const uint8_t endstop_pins[MOTOR_AXIS_COUNT] = {
    X_LIMIT_PIN,
    Y_LIMIT_PIN,
    Z_LIMIT_PIN
};

static const uint8_t max_endstop_pins[MOTOR_AXIS_COUNT] = {
    X_MAX_LIMIT_PIN,
    Y_MAX_LIMIT_PIN,
    Z_MAX_LIMIT_PIN
};

static bool valid_axis(MotorAxis axis) {
    return axis >= MOTOR_AXIS_X && axis < MOTOR_AXIS_COUNT;
}

static bool valid_pin(uint8_t pin) {
    return pin != 255;
}

void drv_endstop_init() {
    for (uint8_t i = 0; i < MOTOR_AXIS_COUNT; ++i) {
        if (valid_pin(endstop_pins[i])) {
            hal_gpio_mode(endstop_pins[i], HAL_PIN_INPUT_PULLUP);
        }
        if (valid_pin(max_endstop_pins[i])) {
            hal_gpio_mode(max_endstop_pins[i], HAL_PIN_INPUT_PULLUP);
        }
    }
}

bool drv_endstop_is_triggered(MotorAxis axis) {
    if (!valid_axis(axis) || !valid_pin(endstop_pins[axis])) {
        return false;
    }

    return hal_gpio_read(endstop_pins[axis]) == HAL_LOW;
}

bool drv_endstop_max_is_available(MotorAxis axis) {
    return valid_axis(axis) && valid_pin(max_endstop_pins[axis]);
}

bool drv_endstop_max_is_triggered(MotorAxis axis) {
    if (!drv_endstop_max_is_available(axis)) {
        return false;
    }

    return hal_gpio_read(max_endstop_pins[axis]) == HAL_LOW;
}

bool drv_endstop_all_triggered() {
    for (uint8_t i = 0; i < MOTOR_AXIS_COUNT; ++i) {
        if (!drv_endstop_is_triggered(static_cast<MotorAxis>(i))) {
            return false;
        }
    }
    return true;
}
