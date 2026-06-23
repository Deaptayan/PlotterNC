#include "drv_display.h"

#include "../hal/hal.h"
#include "../hal/hal_platform.h"

#include <stdio.h>
#include <string.h>

static char display_line1[24] = "Plotter";
static char display_line2[24] = "Idle";
static bool dirty = true;

void drv_display_init() {
    hal_i2c_begin(OLED_SDA, OLED_SCL, 400000);
    dirty = true;
}

void drv_display_clear() {
    display_line1[0] = '\0';
    display_line2[0] = '\0';
    dirty = true;
}

void drv_display_set_status(const char *line1, const char *line2) {
    snprintf(display_line1, sizeof(display_line1), "%s", line1 == nullptr ? "" : line1);
    snprintf(display_line2, sizeof(display_line2), "%s", line2 == nullptr ? "" : line2);
    dirty = true;
}

void drv_display_update() {
    if (!dirty) {
        return;
    }

    // Placeholder transport: real OLED command framing belongs here, still via HAL I2C.
    uint8_t payload[2] = {0x00, 0xAF};
    hal_i2c_write(OLED_I2C_ADDRESS, payload, sizeof(payload));
    dirty = false;
}
