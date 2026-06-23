#if defined(PLATFORMIO)
#include <Arduino.h>

#include "hal/hal.h"
#include "os/os.h"

#include "core/gcode.h"
#include "core/homing.h"
#include "core/planner.h"
#include "core/protocol.h"
#include "core/storage.h"
#include "drivers/drv_display.h"
#include "drivers/drv_encoder.h"
#include "drivers/drv_motor.h"
#include "drivers/drv_pen.h"
#include "drivers/drv_serial.h"
#include "net/web_server.h"
#include "net/wifi_manager.h"

static void task_motion() {
    homing_task();
    planner_task();
}

static void task_encoder() {
    drv_encoder_update();
}

static void task_serial() {
    protocol_task();
}

static void task_display() {
    if (homing_state() == HOMING_RUNNING) {
        drv_display_set_status("Plotter", "Homing");
    } else if (planner_has_work()) {
        drv_display_set_status("Plotter", "Running");
    } else {
        drv_display_set_status("Plotter", wifi_manager_state_text());
    }
    drv_display_update();
}

static void task_wifi() {
    wifi_manager_task();
    web_server_task();
}

void setup() {
    hal_init();

    os_init();
    drv_serial_init(115200);
    storage_init();
    drv_motor_init();
    drv_pen_init();
    drv_encoder_init();
    drv_display_init();
    planner_init();
    homing_init();
    wifi_manager_init();
    web_server_init();
    protocol_init();

    os_task_add("motion", task_motion, 1, 0);
    os_task_add("serial", task_serial, 2, 0);
    os_task_add("encoder", task_encoder, 5, 1);
    os_task_add("display", task_display, 100, 2);
    os_task_add("wifi", task_wifi, 20, 3);
}

void loop() {
    os_run();
}
#endif
