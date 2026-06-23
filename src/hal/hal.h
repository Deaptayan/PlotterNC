#ifndef PLOTTER_HAL_H
#define PLOTTER_HAL_H

#include <stddef.h>
#include <stdint.h>

enum HalPinMode {
    HAL_PIN_INPUT,
    HAL_PIN_INPUT_PULLUP,
    HAL_PIN_OUTPUT
};

enum HalPinLevel {
    HAL_LOW = 0,
    HAL_HIGH = 1
};

void hal_init();

void hal_gpio_mode(uint8_t pin, HalPinMode mode);
void hal_gpio_write(uint8_t pin, HalPinLevel level);
HalPinLevel hal_gpio_read(uint8_t pin);

bool hal_i2c_begin(uint8_t sda_pin, uint8_t scl_pin, uint32_t frequency_hz);
bool hal_i2c_write(uint8_t address, const uint8_t *data, size_t length);
bool hal_i2c_read(uint8_t address, uint8_t *data, size_t length);

bool hal_uart_begin(uint8_t port, uint32_t baud);
size_t hal_uart_write(uint8_t port, const uint8_t *data, size_t length);
int hal_uart_read(uint8_t port);
int hal_uart_available(uint8_t port);

uint32_t hal_millis();
uint32_t hal_micros();
void hal_delay_ms(uint32_t delay_ms);
void hal_reboot();

#endif
