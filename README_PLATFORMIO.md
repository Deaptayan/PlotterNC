# VS Code + PlatformIO

Open this folder in VS Code

Install the PlatformIO extension if VS Code asks.

Useful PlatformIO actions:

```text
PlatformIO: Build
PlatformIO: Upload
PlatformIO: Monitor
```

Or from a terminal with PlatformIO installed:

```powershell
pio run
pio run --target upload
pio device monitor -b 115200
```

The project keeps `plotter.ino` for Arduino IDE, but PlatformIO builds from `src/main.cpp`.