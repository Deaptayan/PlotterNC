#ifndef PLOTTER_DRV_SERIAL_H
#define PLOTTER_DRV_SERIAL_H

#include <stddef.h>
#include <stdint.h>

void drv_serial_init(uint32_t baud);
int drv_serial_available();
int drv_serial_read();
void drv_serial_write(const char *text);
void drv_serial_write_data(const uint8_t *data, size_t length);

#endif
