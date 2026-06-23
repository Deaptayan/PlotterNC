#include "wifi_manager.h"

#include "../hal/hal.h"
#include "../hal/hal_platform.h"

#if defined(ARDUINO)
#include <WiFi.h>
#endif

static WifiState state = WIFI_STATE_OFF;

void wifi_manager_init() {
    hal_gpio_mode(WIFI_STATUS_LED_PIN, HAL_PIN_OUTPUT);

#if defined(ARDUINO)
    WiFi.mode(WIFI_AP);
    if (WiFi.softAP("Plotter-CNC", "plotter123")) {
        state = WIFI_STATE_AP_SETUP;
    } else {
        state = WIFI_STATE_OFF;
    }
#else
    state = WIFI_STATE_AP_SETUP;
#endif
}

void wifi_manager_task() {
    hal_gpio_write(WIFI_STATUS_LED_PIN,
                   (state == WIFI_STATE_CONNECTED || state == WIFI_STATE_AP_SETUP) ? HAL_HIGH : HAL_LOW);
}

WifiState wifi_manager_state() {
    return state;
}

const char *wifi_manager_state_text() {
    switch (state) {
    case WIFI_STATE_OFF:
        return "off";
    case WIFI_STATE_CONNECTING:
        return "connecting";
    case WIFI_STATE_CONNECTED:
        return "connected";
    case WIFI_STATE_AP_SETUP:
        return "setup";
    }
    return "unknown";
}
