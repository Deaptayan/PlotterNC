#include "drv_serial.h"

#include "../hal/hal.h"

#include <string.h>

static const uint8_t SERIAL_PORT = 0;

void drv_serial_init(uint32_t baud) {
    hal_uart_begin(SERIAL_PORT, baud);
}

int drv_serial_available() {
    return hal_uart_available(SERIAL_PORT);
}

int drv_serial_read() {
    return hal_uart_read(SERIAL_PORT);
}

void drv_serial_write(const char *text) {
    if (text == nullptr) {
        return;
    }

    drv_serial_write_data(reinterpret_cast<const uint8_t *>(text), strlen(text));
}

void drv_serial_write_data(const uint8_t *data, size_t length) {
    if (data == nullptr || length == 0) {
        return;
    }

    hal_uart_write(SERIAL_PORT, data, length);
}
