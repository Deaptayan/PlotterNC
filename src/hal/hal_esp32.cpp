#include "hal.h"
#include "hal_esp32.h"

#if defined(ARDUINO)
#include <Arduino.h>
#include <Wire.h>
#if defined(ESP32)
#include <Esp.h>
#endif
#if defined(PLATFORMIO)
#define PLOTTER_SERIAL Serial1
#else
#define PLOTTER_SERIAL Serial
#endif
#endif

static bool serial_started = false;

void hal_init() {
#if defined(ARDUINO)
    // Arduino runtime is initialized before setup().
#endif
}

void hal_gpio_mode(uint8_t pin, HalPinMode mode) {
#if defined(ARDUINO)
    uint8_t arduino_mode = OUTPUT;
    if (mode == HAL_PIN_INPUT) {
        arduino_mode = INPUT;
    } else if (mode == HAL_PIN_INPUT_PULLUP) {
        arduino_mode = INPUT_PULLUP;
    }
    pinMode(pin, arduino_mode);
#else
    (void)pin;
    (void)mode;
#endif
}

void hal_gpio_write(uint8_t pin, HalPinLevel level) {
#if defined(ARDUINO)
    digitalWrite(pin, level == HAL_HIGH ? HIGH : LOW);
#else
    (void)pin;
    (void)level;
#endif
}

HalPinLevel hal_gpio_read(uint8_t pin) {
#if defined(ARDUINO)
    return digitalRead(pin) == HIGH ? HAL_HIGH : HAL_LOW;
#else
    (void)pin;
    return HAL_LOW;
#endif
}

bool hal_i2c_begin(uint8_t sda_pin, uint8_t scl_pin, uint32_t frequency_hz) {
#if defined(ARDUINO)
    Wire.begin(sda_pin, scl_pin);
    Wire.setClock(frequency_hz);
    return true;
#else
    (void)sda_pin;
    (void)scl_pin;
    (void)frequency_hz;
    return false;
#endif
}

bool hal_i2c_write(uint8_t address, const uint8_t *data, size_t length) {
#if defined(ARDUINO)
    Wire.beginTransmission(address);
    for (size_t i = 0; i < length; ++i) {
        Wire.write(data[i]);
    }
    return Wire.endTransmission() == 0;
#else
    (void)address;
    (void)data;
    (void)length;
    return false;
#endif
}

bool hal_i2c_read(uint8_t address, uint8_t *data, size_t length) {
#if defined(ARDUINO)
    size_t received = Wire.requestFrom((int)address, (int)length);
    for (size_t i = 0; i < received && i < length; ++i) {
        data[i] = Wire.read();
    }
    return received == length;
#else
    (void)address;
    (void)data;
    (void)length;
    return false;
#endif
}

bool hal_uart_begin(uint8_t port, uint32_t baud) {
#if defined(ARDUINO)
    if (port == 0) {
        PLOTTER_SERIAL.begin(baud);
        serial_started = true;
        return true;
    }
#endif
    (void)port;
    (void)baud;
    return false;
}

size_t hal_uart_write(uint8_t port, const uint8_t *data, size_t length) {
#if defined(ARDUINO)
    if (port == 0 && serial_started) {
        return PLOTTER_SERIAL.write(data, length);
    }
#endif
    (void)port;
    (void)data;
    (void)length;
    return 0;
}

int hal_uart_read(uint8_t port) {
#if defined(ARDUINO)
    if (port == 0 && serial_started && PLOTTER_SERIAL.available() > 0) {
        return PLOTTER_SERIAL.read();
    }
#endif
    (void)port;
    return -1;
}

int hal_uart_available(uint8_t port) {
#if defined(ARDUINO)
    if (port == 0 && serial_started) {
        return PLOTTER_SERIAL.available();
    }
#endif
    (void)port;
    return 0;
}

uint32_t hal_millis() {
#if defined(ARDUINO)
    return millis();
#else
    return 0;
#endif
}

uint32_t hal_micros() {
#if defined(ARDUINO)
    return micros();
#else
    return 0;
#endif
}

void hal_delay_ms(uint32_t delay_ms) {
#if defined(ARDUINO)
    delay(delay_ms);
#else
    (void)delay_ms;
#endif
}

void hal_reboot() {
#if defined(ESP32)
    ESP.restart();
#elif defined(ARDUINO)
    while (true) {
        delay(1000);
    }
#endif
}