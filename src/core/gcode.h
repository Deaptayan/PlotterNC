#ifndef PLOTTER_GCODE_H
#define PLOTTER_GCODE_H

enum GcodeUnits {
    GCODE_UNITS_MM,
    GCODE_UNITS_INCH
};

enum GcodeDistanceMode {
    GCODE_DISTANCE_ABSOLUTE,
    GCODE_DISTANCE_INCREMENTAL
};

bool gcode_execute_line(const char *line);
void gcode_mode_report(char *buffer, unsigned int buffer_size);
void gcode_task();

#endif
