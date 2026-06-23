#include "protocol.h"

#include "gcode.h"
#include "homing.h"
#include "planner.h"
#include "../drivers/drv_motor.h"
#include "../drivers/drv_serial.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static const uint8_t LINE_BUFFER_SIZE = 96;
static char line_buffer[LINE_BUFFER_SIZE];
static uint8_t line_length = 0;

static void protocol_send_ok() {
    drv_serial_write("ok\r\n");
}

static void protocol_send_error(const char *message) {
    drv_serial_write("error:");
    drv_serial_write(message == nullptr ? "bad command" : message);
    drv_serial_write("\r\n");
}

static const char *machine_state_text() {
    switch (homing_state()) {
    case HOMING_RUNNING:
        return "Home";
    case HOMING_FAILED:
        return "Alarm";
    case HOMING_IDLE:
    case HOMING_COMPLETE:
        break;
    }

    return planner_has_work() ? "Run" : "Idle";
}

static void append_response(char *response, size_t response_size, const char *text) {
    if (response == nullptr || response_size == 0 || text == nullptr) {
        return;
    }

    const size_t used = strlen(response);
    if (used >= response_size - 1) {
        return;
    }

    snprintf(response + used, response_size - used, "%s", text);
}

static bool is_blank_line(const char *line) {
    if (line == nullptr) {
        return true;
    }

    while (*line != '\0') {
        if (!isspace(*line)) {
            return false;
        }
        ++line;
    }
    return true;
}

static void strip_comments(char *line) {
    bool in_paren_comment = false;
    char *write = line;

    for (char *read = line; *read != '\0'; ++read) {
        if (*read == '(') {
            in_paren_comment = true;
            continue;
        }
        if (*read == ')') {
            in_paren_comment = false;
            continue;
        }
        if (*read == ';') {
            break;
        }
        if (!in_paren_comment) {
            *write = *read;
            ++write;
        }
    }

    *write = '\0';
}

static void strip_line_number_and_checksum(char *line) {
    if (line == nullptr) {
        return;
    }

    char *checksum = strchr(line, '*');
    if (checksum != nullptr) {
        *checksum = '\0';
    }

    char *p = line;
    while (isspace(*p)) {
        ++p;
    }

    if (toupper(*p) != 'N') {
        if (p != line) {
            memmove(line, p, strlen(p) + 1);
        }
        return;
    }

    ++p;
    while (*p != '\0' && (isdigit(*p) || *p == '+' || *p == '-')) {
        ++p;
    }
    while (isspace(*p)) {
        ++p;
    }

    memmove(line, p, strlen(p) + 1);
}

static void uppercase_line(char *line) {
    for (char *p = line; *p != '\0'; ++p) {
        *p = static_cast<char>(toupper(*p));
    }
}

void protocol_report_status() {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    planner_current_position(&x, &y, &z);

    char response[96];
    snprintf(response, sizeof(response), "<%s|MPos:%.3f,%.3f,%.3f|Bf:%d,%d>\r\n",
             machine_state_text(),
             x,
             y,
             z,
             planner_buffer_available(),
             drv_serial_available());
    drv_serial_write(response);
}

static void report_help(char *response, size_t response_size) {
    append_response(response, response_size, "[HLP:$$ $G $H ? ! ~ M112 M17 M18 M84 M3 M5 G0 G1 G2 G3 G20 G21 G90 G91 G92]\n");
}

static void report_settings(char *response, size_t response_size) {
    char line[48];
    snprintf(line, sizeof(line), "$100=%.3f (x,step/mm)\n", planner_steps_per_mm(MOTOR_AXIS_X));
    append_response(response, response_size, line);
    snprintf(line, sizeof(line), "$101=%.3f (y,step/mm)\n", planner_steps_per_mm(MOTOR_AXIS_Y));
    append_response(response, response_size, line);
    snprintf(line, sizeof(line), "$102=%.3f (z,step/mm)\n", planner_steps_per_mm(MOTOR_AXIS_Z));
    append_response(response, response_size, line);
}

static void report_gcode_modes(char *response, size_t response_size) {
    char modes[80];
    gcode_mode_report(modes, sizeof(modes));
    append_response(response, response_size, modes);
    append_response(response, response_size, "\n");
}

static void format_status(char *response, size_t response_size) {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    planner_current_position(&x, &y, &z);

    snprintf(response, response_size, "<%s|MPos:%.3f,%.3f,%.3f|Bf:%d,%d>\n",
             machine_state_text(),
             x,
             y,
             z,
             planner_buffer_available(),
             drv_serial_available());
}

bool protocol_execute_command(const char *input, char *response, size_t response_size) {
    if (response != nullptr && response_size > 0) {
        response[0] = '\0';
    }

    if (input == nullptr) {
        append_response(response, response_size, "error:empty command\n");
        return false;
    }

    char line[LINE_BUFFER_SIZE];
    snprintf(line, sizeof(line), "%s", input);
    strip_comments(line);
    strip_line_number_and_checksum(line);
    uppercase_line(line);

    if (is_blank_line(line)) {
        append_response(response, response_size, "ok\n");
        return true;
    }

    if (strcmp(line, "?") == 0) {
        format_status(response, response_size);
        return true;
    }

    if (strcmp(line, "$") == 0) {
        report_help(response, response_size);
        append_response(response, response_size, "ok\n");
        return true;
    }

    if (strcmp(line, "$$") == 0) {
        report_settings(response, response_size);
        append_response(response, response_size, "ok\n");
        return true;
    }

    if (strcmp(line, "$G") == 0) {
        report_gcode_modes(response, response_size);
        append_response(response, response_size, "ok\n");
        return true;
    }

    if (strcmp(line, "$H") == 0) {
        homing_start();
        append_response(response, response_size, "ok\n");
        return true;
    }

    if (strcmp(line, "!") == 0) {
        planner_pause();
        append_response(response, response_size, "ok:paused\n");
        return true;
    }

    if (strcmp(line, "~") == 0) {
        planner_resume();
        append_response(response, response_size, "ok:resumed\n");
        return true;
    }

    if (strcmp(line, "M112") == 0) {
        planner_stop();
        append_response(response, response_size, "ok:stopped\n");
        return true;
    }
    if (strcmp(line, "M17") == 0) {
        drv_motor_set_all_enabled(true);
        append_response(response, response_size, "ok\n");
        return true;
    }

    if (strcmp(line, "M18") == 0 || strcmp(line, "M84") == 0) {
        drv_motor_set_all_enabled(false);
        append_response(response, response_size, "ok\n");
        return true;
    }

    if (gcode_execute_line(line)) {
        append_response(response, response_size, "ok\n");
        return true;
    }

    append_response(response, response_size, "error:unsupported command\n");
    return false;
}

void protocol_init() {
    drv_serial_write("\r\nGrbl 1.1f ['$' for help]\r\n");
    line_length = 0;
}

void protocol_task() {
    while (drv_serial_available() > 0) {
        const int value = drv_serial_read();
        if (value < 0) {
            return;
        }

        const char c = static_cast<char>(value);
        if (c == 0x18) {
            drv_serial_write("\r\n[MSG:Reset]\r\n");
            line_length = 0;
            protocol_init();
            continue;
        }

        if (c == '?') {
            protocol_report_status();
            continue;
        }

        if (c == '\r' || c == '\n') {
            if (line_length > 0) {
                line_buffer[line_length] = '\0';
                char response[192];
                const bool ok = protocol_execute_command(line_buffer, response, sizeof(response));
                drv_serial_write(response);
                (void)ok;
                line_length = 0;
            }
            continue;
        }

        if (c == '\b' || c == 0x7F) {
            if (line_length > 0) {
                --line_length;
            }
            continue;
        }

        if (line_length >= LINE_BUFFER_SIZE - 1) {
            line_length = 0;
            protocol_send_error("line overflow");
            continue;
        }

        line_buffer[line_length] = c;
        ++line_length;
    }
}
