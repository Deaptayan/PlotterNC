# Plotter Firmware

Portable plotter firmware organized into application, driver, OS, HAL, and platform layers.

## Layers

- `plotter/src/core/`: G-code, planner, homing, and storage machine logic. Uses drivers and OS APIs only.
- `plotter/src/drivers/`: Motor, display, encoder, and endstop drivers. Uses HAL APIs only.
- `plotter/src/os/`: Cooperative scheduler, timers, queues, and mutexes.
- `plotter/src/hal/`: Fixed hardware interface plus the active platform mapping.
- `plotter/src/net/`: WiFi and web server modules.
- `data/`: Static web UI assets for the plotter.

## Porting

To support another MCU, add a new platform implementation such as `hal_avr.cpp` and a board mapping header such as `hal_avr.h`, then update `hal/hal_platform.h` or the build settings to select it. The `core/`, `drivers/`, and `os/` layers should not need source changes.
