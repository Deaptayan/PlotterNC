#include "gcode.h"

#include "planner.h"
#include "../config.h"
#include "../drivers/drv_pen.h"
#include "../hal/hal.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static float modal_x = 0.0f;
static float modal_y = 0.0f;
static float modal_z = DEFAULT_PEN_UP_Z_MM;
static float modal_feedrate = 1000.0f;
static int modal_motion = 1;
static GcodeUnits modal_units = GCODE_UNITS_MM;
static GcodeDistanceMode modal_distance = GCODE_DISTANCE_ABSOLUTE;

static bool read_word(const char *line, char word, float *value) {
    if (line == nullptr || value == nullptr) {
        return false;
    }

    for (const char *p = line; *p != '\0'; ++p) {
        if (toupper(*p) == word) {
            *value = strtof(p + 1, nullptr);
            return true;
        }
    }
    return false;
}

static bool has_word(const char *line, char word) {
    if (line == nullptr) {
        return false;
    }

    for (const char *p = line; *p != '\0'; ++p) {
        if (toupper(*p) == word) {
            return true;
        }
    }
    return false;
}

static float units_to_mm(float value) {
    return modal_units == GCODE_UNITS_INCH ? value * 25.4f : value;
}

static void update_axis_target(const char *line, char word, float *target) {
    float value = 0.0f;
    if (!read_word(line, word, &value) || target == nullptr) {
        return;
    }

    value = units_to_mm(value);
    if (modal_distance == GCODE_DISTANCE_INCREMENTAL) {
        *target += value;
    } else {
        *target = value;
    }
}

static bool enqueue_linear(float x, float y, float z) {
    PlannerMove move = {x, y, z, modal_feedrate};
    return planner_enqueue_move(&move);
}

static bool enqueue_arc(const char *line, bool clockwise) {
    float target_x = modal_x;
    float target_y = modal_y;
    float target_z = modal_z;
    update_axis_target(line, 'X', &target_x);
    update_axis_target(line, 'Y', &target_y);
    update_axis_target(line, 'Z', &target_z);

    float i = 0.0f;
    float j = 0.0f;
    if (!read_word(line, 'I', &i) || !read_word(line, 'J', &j)) {
        return false;
    }
    i = units_to_mm(i);
    j = units_to_mm(j);

    const float center_x = modal_x + i;
    const float center_y = modal_y + j;
    const float radius = sqrtf((i * i) + (j * j));
    if (radius <= 0.0f) {
        return false;
    }

    const float start_angle = atan2f(modal_y - center_y, modal_x - center_x);
    const float end_angle = atan2f(target_y - center_y, target_x - center_x);
    float sweep = end_angle - start_angle;

    if (clockwise && sweep >= 0.0f) {
        sweep -= 2.0f * (float)M_PI;
    } else if (!clockwise && sweep <= 0.0f) {
        sweep += 2.0f * (float)M_PI;
    }

    int segments = (int)ceilf(fabsf(sweep) * radius / 1.0f);
    if (segments < 4) {
        segments = 4;
    }
    if (segments > 12) {
        segments = 12;
    }

    for (int s = 1; s <= segments; ++s) {
        const float t = (float)s / (float)segments;
        const float angle = start_angle + (sweep * t);
        const float x = center_x + cosf(angle) * radius;
        const float y = center_y + sinf(angle) * radius;
        const float z = modal_z + ((target_z - modal_z) * t);
        if (!enqueue_linear(x, y, z)) {
            return false;
        }
    }

    modal_x = target_x;
    modal_y = target_y;
    modal_z = target_z;
    return true;
}

bool gcode_execute_line(const char *line) {
    if (line == nullptr) {
        return false;
    }

    if (has_word(line, 'F')) {
        float feedrate = modal_feedrate;
        read_word(line, 'F', &feedrate);
        if (feedrate < 1.0f) {
            feedrate = 1.0f;
        }
        modal_feedrate = units_to_mm(feedrate);
    }

    float code = -1.0f;
    if (read_word(line, 'G', &code)) {
        const int g = (int)code;

        if (g == 0 || g == 1) {
            modal_motion = g;
        } else if (g == 2 || g == 3) {
            modal_motion = g;
            return enqueue_arc(line, g == 2);
        } else if (g == 4) {
            float dwell_ms = 0.0f;
            if (read_word(line, 'P', &dwell_ms) && dwell_ms > 0.0f) {
                hal_delay_ms((uint32_t)dwell_ms);
            }
            return true;
        } else if (g == 20) {
            modal_units = GCODE_UNITS_INCH;
            return true;
        } else if (g == 21) {
            modal_units = GCODE_UNITS_MM;
            return true;
        } else if (g == 90) {
            modal_distance = GCODE_DISTANCE_ABSOLUTE;
            return true;
        } else if (g == 91) {
            modal_distance = GCODE_DISTANCE_INCREMENTAL;
            return true;
        } else if (g == 92) {
            update_axis_target(line, 'X', &modal_x);
            update_axis_target(line, 'Y', &modal_y);
            update_axis_target(line, 'Z', &modal_z);
            planner_set_position(modal_x, modal_y, modal_z);
            return true;
        } else {
            return false;
        }
    }

    if (has_word(line, 'X') || has_word(line, 'Y') || has_word(line, 'Z')) {
        update_axis_target(line, 'X', &modal_x);
        update_axis_target(line, 'Y', &modal_y);
        update_axis_target(line, 'Z', &modal_z);

        if (modal_motion != 0 && modal_motion != 1) {
            return false;
        }
        return enqueue_linear(modal_x, modal_y, modal_z);
    }

    float m_code = -1.0f;
    if (read_word(line, 'M', &m_code)) {
        const int m = (int)m_code;
        if (m == 0 || m == 1) {
            planner_pause();
            return true;
        }
        if (m == 2 || m == 30) {
            planner_stop();
            drv_pen_up();
            return true;
        }
        if (m == 3) {
            drv_pen_down();
            return true;
        }
        if (m == 5 || m == 9) {
            drv_pen_up();
            return true;
        }
        if (m == 112) {
            planner_stop();
            drv_pen_up();
            return true;
        }
        return false;
    }

    if (!has_word(line, 'G')) {
        return true;
    }

    return false;
}

void gcode_mode_report(char *buffer, unsigned int buffer_size) {
    if (buffer == nullptr || buffer_size == 0) {
        return;
    }

    const char *units = modal_units == GCODE_UNITS_MM ? "G21" : "G20";
    const char *distance = modal_distance == GCODE_DISTANCE_ABSOLUTE ? "G90" : "G91";
    snprintf(buffer, buffer_size, "[GC:G%d %s %s G94 %s M9 T0 F%.0f]",
             modal_motion,
             units,
             distance,
             drv_pen_is_down() ? "M3" : "M5",
             modal_feedrate);
}

void gcode_task() {
}