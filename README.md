![Version](https://img.shields.io/badge/version-v0.3-blue)
![License](https://img.shields.io/badge/license-AGPL--3.0-green)
![Platform](https://img.shields.io/badge/platform-ESP32--S2-orange)
![Status](https://img.shields.io/badge/status-Alpha-orange)
![Stability](https://img.shields.io/badge/stability-Experimental-red)
![IDE](https://img.shields.io/badge/IDE-Arduino%20IDE%20%7C%20PlatformIO-blue)
![G-Code](https://img.shields.io/badge/G--Code-Supported-success)
![WebUI](https://img.shields.io/badge/WebUI-Integrated-blueviolet)
![Motion](https://img.shields.io/badge/Motion-DDA%20Planner-red)
![Pen Control](https://img.shields.io/badge/Pen%20Lift-Stepper%20Driven-yellow)

# PlotterNC Firmware

Portable CNC and Pen Plotter firmware designed for ESP32-class microcontrollers with a layered architecture, web control interface, G-code execution, coordinated motion planning, and pen control support.

Current development and testing are performed on ESP32-S2 hardware using both Arduino IDE and PlatformIO environments.

---

## Repository

Firmware Repository:

https://github.com/Deaptayan/PlotterNC

Companion Software:

https://github.com/Deaptayan/PlotterNCStudio

---

## Highlights

✅ Coordinated XY Motion

✅ DDA/Bresenham Motion Planner

✅ G0 / G1 Linear Interpolation

✅ G2 / G3 Arc Support

✅ Stepper-Driven Pen Lift

✅ Web-Based Control Interface

✅ Pause / Resume / Emergency Stop

✅ Hardware Abstraction Layer (HAL)

✅ Portable Architecture

---

> [!WARNING]
> PlotterNC Firmware is currently in **v0.3 Alpha** and under active development.
>
> While core functionality is implemented, the firmware is not yet considered stable or production-ready. Motion quality, drawing accuracy, hardware compatibility, and reliability are still being improved.
>
> Users may encounter:
>
> * Motion inaccuracies
> * Drawing quality issues
> * Unexpected firmware bugs
> * Incomplete hardware support
> * Missing safety features
> * Unfinished functionality
>
> This release is intended primarily for development, testing, and experimentation.

---

## Project Status

PlotterNC Firmware is an experimental open-source firmware for pen plotters and lightweight CNC machines.

Current development focuses on:

* Motion planner improvements
* Hardware abstraction layer expansion
* Web-based machine control
* Plotter-specific workflow features
* Multi-platform support preparation

---

## Current Release

| Item                      | Value                         |
| ------------------------- | ----------------------------- |
| Version                   | v0.3 Alpha                    |
| Status                    | Experimental                  |
| Firmware Architecture     | Portable HAL-Based Design     |
| Current Supported MCU     | ESP32-S2                      |
| Current Supported Motors  | 28BYJ-48                      |
| Current Supported Drivers | ULN2003                       |
| Pen Actuation             | Stepper Motor Driven          |
| Supported IDEs            | Arduino IDE, PlatformIO       |
| Tested Hardware           | ESP32-S2 + 28BYJ-48 + ULN2003 |
| License                   | AGPL-3.0                      |
| Development               | Active                        |

---

## Current Hardware Support

### Microcontroller

* ESP32-S2

### Stepper Motors

* 28BYJ-48

### Motor Drivers

* ULN2003

### Pen Actuation

* Stepper Motor Driven Pen Lift

### Supported Machine Type

* Cartesian Pen Plotters

---

## Development Environment

### Supported IDEs

* Arduino IDE
* Visual Studio Code + PlatformIO

### Supported Framework

* Arduino Framework for ESP32

Developers may use either Arduino IDE or PlatformIO depending on their preferred workflow.

---

## Photos

Coming Soon

---

## Features

### Motion Control

* Coordinated X/Y motion using DDA/Bresenham-style step generation
* True linear interpolation for G0/G1 moves
* Diagonal and multi-axis movement
* Non-blocking motion execution
* Basic acceleration and deceleration ramping
* Microsecond timing using `hal_micros()`

### G-Code Support

| Command | Function              |
| ------- | --------------------- |
| G0      | Rapid Move            |
| G1      | Linear Move           |
| G2      | Clockwise Arc         |
| G3      | Counter Clockwise Arc |
| M3      | Pen Down              |
| M5      | Pen Up                |
| M9      | Pen Up                |
| M0      | Pause                 |
| M1      | Pause                 |
| M2      | End Job               |
| M30     | End Job               |
| M112    | Emergency Stop        |

### Pen Control

Pen actuation is currently performed using a dedicated stepper motor mechanism.

```gcode
M3   ; Pen Down
M5   ; Pen Up
M9   ; Pen Up
```

### Runtime Control

```text
!      Pause
~      Resume
M112   Emergency Stop
M0     Pause
M1     Pause
M2     End Job
M30    End Job
```

---

## Supported Commands

### Motion

* G0
* G1
* G2
* G3

### Pen Control

* M3
* M5
* M9

### Job Control

* M0
* M1
* M2
* M30
* M112

### Runtime Commands

* !
* ~

---

## Firmware Architecture

```text
Web UI
   │
Net Layer
   │
Core Layer
   │
Driver Layer
   │
OS Layer
   │
HAL Layer
   │
Platform Layer
```

### Layer Overview

| Layer    | Responsibility                             |
| -------- | ------------------------------------------ |
| Core     | G-code parsing, planner, homing, storage   |
| Drivers  | Motors, displays, encoders, endstops       |
| OS       | Scheduler, timers, queues, synchronization |
| HAL      | Hardware abstraction interfaces            |
| Platform | Board-specific implementation              |
| Net      | WiFi and Web Server                        |
| Data     | Web UI assets                              |

---

## Roadmap

### v0.4

* Endstop integration
* Hard limit handling
* Persistent calibration storage
* SD card support

### v0.5

* Trapezoidal acceleration
* Look-ahead motion planning
* R-format arc support

### v1.0

* First stable release
* Multi-board support
* Production-ready web interface
* Full job recovery support

---

## Future Hardware Support

### Microcontrollers

* ESP32-S3
* ESP32-C3
* STM32
* RP2040
* AVR

### Stepper Drivers

* A4988
* DRV8825
* TMC2208
* TMC2209

### Machine Types

* Cartesian Plotters
* CoreXY Plotters
* Lightweight CNC Machines

---

## Companion Software

### PlotterNC Studio

https://github.com/Deaptayan/PlotterNCStudio

PlotterNC Studio is the companion CAM and G-code generation software for PlotterNC Firmware.

Features include:

* SVG Import
* Toolpath Generation
* Path Optimization
* G-code Export
* Plotter Configuration
* Web-Based Interface

---

## Contributing

Bug reports, feature requests, and pull requests are welcome.

Please open an issue before making major architectural changes.

---

## License

Licensed under the GNU Affero General Public License v3.0 (AGPL-3.0).

If you distribute modified versions of PlotterNC Firmware or provide it as a network-accessible service, you must also make the complete corresponding source code available under the same license.

---

## Author

**Deaptayan Bondopadhay**

Creator of PlotterNC Firmware

---

Made with ❤️ for DIY Plotters, CNC Machines, and Open Hardware.
