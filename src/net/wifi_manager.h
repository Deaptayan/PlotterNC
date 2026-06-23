#ifndef PLOTTER_WIFI_MANAGER_H
#define PLOTTER_WIFI_MANAGER_H

enum WifiState {
    WIFI_STATE_OFF,
    WIFI_STATE_CONNECTING,
    WIFI_STATE_CONNECTED,
    WIFI_STATE_AP_SETUP
};

void wifi_manager_init();
void wifi_manager_task();
WifiState wifi_manager_state();
const char *wifi_manager_state_text();

#endif
