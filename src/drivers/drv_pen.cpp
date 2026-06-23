#include "drv_pen.h"

#include "../config.h"
#include "../hal/hal.h"
#include "../hal/hal_platform.h"

static bool pen_down = false;

#if defined(ARDUINO)
#include <Arduino.h>
#if __has_include(<esp_arduino_version.h>)
#include <esp_arduino_version.h>
#endif

static const uint32_t SERVO_FREQ_HZ = 50;
static const uint8_t SERVO_RESOLUTION_BITS = 16;
static const uint8_t PEN_SERVO_CHANNEL = 0;

static uint32_t pulse_to_duty(uint16_t pulse_us) {
    const uint32_t max_duty = (1UL << SERVO_RESOLUTION_BITS) - 1UL;
    return (uint32_t)(((uint64_t)pulse_us * max_duty) / PEN_SERVO_REFRESH_US);
}

static void pen_attach_pwm() {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcAttach(PEN_SERVO_PIN, SERVO_FREQ_HZ, SERVO_RESOLUTION_BITS);
#else
    ledcSetup(PEN_SERVO_CHANNEL, SERVO_FREQ_HZ, SERVO_RESOLUTION_BITS);
    ledcAttachPin(PEN_SERVO_PIN, PEN_SERVO_CHANNEL);
#endif
}

static void pen_write_pulse(uint16_t pulse_us) {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcWrite(PEN_SERVO_PIN, pulse_to_duty(pulse_us));
#else
    ledcWrite(PEN_SERVO_CHANNEL, pulse_to_duty(pulse_us));
#endif
}
#endif

void drv_pen_init() {
#if defined(ARDUINO)
    pen_attach_pwm();
#else
    hal_gpio_mode(PEN_SERVO_PIN, HAL_PIN_OUTPUT);
#endif
    drv_pen_up();
}

void drv_pen_up() {
#if defined(ARDUINO)
    pen_write_pulse(PEN_UP_SERVO_US);
#else
    hal_gpio_write(PEN_SERVO_PIN, HAL_LOW);
#endif
    pen_down = false;
}

void drv_pen_down() {
#if defined(ARDUINO)
    pen_write_pulse(PEN_DOWN_SERVO_US);
#else
    hal_gpio_write(PEN_SERVO_PIN, HAL_HIGH);
#endif
    pen_down = true;
}

bool drv_pen_is_down() {
    return pen_down;
}