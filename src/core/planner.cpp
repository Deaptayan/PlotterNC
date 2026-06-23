#include "planner.h"

#include "../config.h"
#include "../drivers/drv_motor.h"
#include "../hal/hal.h"
#include "../os/os_queue.h"

#include <math.h>
#include <string.h>

static PlannerMove queue_storage[PLANNER_QUEUE_CAPACITY];
static OsQueue move_queue;
static float current_x = 0.0f;
static float current_y = 0.0f;
static float current_z = 0.0f;
static float axis_steps_per_mm[MOTOR_AXIS_COUNT] = {
    X_STEPS_PER_MM,
    Y_STEPS_PER_MM,
    Z_STEPS_PER_MM
};

struct ActiveMove {
    bool active;
    PlannerMove target;
    long steps[MOTOR_AXIS_COUNT];
    long counters[MOTOR_AXIS_COUNT];
    long event_count;
    long events_done;
    uint32_t base_interval_us;
    uint32_t next_step_us;
};

static ActiveMove active_move;
static bool planner_paused = false;

static long steps_for_delta(float delta_mm, float steps_per_mm) {
    return lroundf(fabsf(delta_mm) * steps_per_mm);
}

static long max3(long a, long b, long c) {
    long max_value = a > b ? a : b;
    return max_value > c ? max_value : c;
}

static void complete_active_move() {
    current_x = active_move.target.x_mm;
    current_y = active_move.target.y_mm;
    current_z = active_move.target.z_mm;
    active_move.active = false;
}

static uint32_t interval_with_ramp() {
    uint32_t interval = active_move.base_interval_us;
    const long ramp_events = active_move.event_count / 5;
    if (ramp_events <= 1) {
        return interval;
    }

    long ramp_position = ramp_events;
    if (active_move.events_done < ramp_events) {
        ramp_position = active_move.events_done + 1;
    } else if ((active_move.event_count - active_move.events_done) < ramp_events) {
        ramp_position = active_move.event_count - active_move.events_done;
    }

    if (ramp_position < 1) {
        ramp_position = 1;
    }

    const float scale = 2.0f - ((float)ramp_position / (float)ramp_events);
    return (uint32_t)((float)interval * scale);
}

static void start_move(const PlannerMove *move) {
    memset(&active_move, 0, sizeof(active_move));
    active_move.target = *move;

    const float dx = move->x_mm - current_x;
    const float dy = move->y_mm - current_y;
    const float dz = move->z_mm - current_z;

    active_move.steps[MOTOR_AXIS_X] = steps_for_delta(dx, axis_steps_per_mm[MOTOR_AXIS_X]);
    active_move.steps[MOTOR_AXIS_Y] = steps_for_delta(dy, axis_steps_per_mm[MOTOR_AXIS_Y]);
    active_move.steps[MOTOR_AXIS_Z] = steps_for_delta(dz, axis_steps_per_mm[MOTOR_AXIS_Z]);
    active_move.event_count = max3(active_move.steps[MOTOR_AXIS_X], active_move.steps[MOTOR_AXIS_Y], active_move.steps[MOTOR_AXIS_Z]);

    drv_motor_set_direction(MOTOR_AXIS_X, dx >= 0.0f);
    drv_motor_set_direction(MOTOR_AXIS_Y, dy >= 0.0f);
    drv_motor_set_direction(MOTOR_AXIS_Z, dz >= 0.0f);

    if (active_move.event_count <= 0) {
        complete_active_move();
        return;
    }

    const float distance_mm = sqrtf((dx * dx) + (dy * dy) + (dz * dz));
    float feedrate = move->feedrate_mm_min;
    if (feedrate < 1.0f) {
        feedrate = 1.0f;
    }

    const float move_time_s = (distance_mm <= 0.0f) ? 0.0f : (distance_mm / (feedrate / 60.0f));
    active_move.base_interval_us = (uint32_t)((move_time_s * 1000000.0f) / (float)active_move.event_count);
    if (active_move.base_interval_us < 1200) {
        active_move.base_interval_us = 1200;
    }

    active_move.next_step_us = hal_micros();
    active_move.active = true;
}

static void service_active_move() {
    if (!active_move.active) {
        return;
    }

    const uint32_t now = hal_micros();
    if ((int32_t)(now - active_move.next_step_us) < 0) {
        return;
    }

    for (uint8_t i = 0; i < MOTOR_AXIS_COUNT; ++i) {
        active_move.counters[i] += active_move.steps[i];
        if (active_move.counters[i] >= active_move.event_count) {
            drv_motor_step(static_cast<MotorAxis>(i));
            active_move.counters[i] -= active_move.event_count;
        }
    }

    ++active_move.events_done;
    if (active_move.events_done >= active_move.event_count) {
        complete_active_move();
        return;
    }

    active_move.next_step_us = now + interval_with_ramp();
}

void planner_init() {
    os_queue_init(&move_queue, queue_storage, sizeof(PlannerMove), PLANNER_QUEUE_CAPACITY);
    current_x = 0.0f;
    current_y = 0.0f;
    current_z = 0.0f;
    memset(&active_move, 0, sizeof(active_move));
    planner_paused = false;
}

bool planner_enqueue_move(const PlannerMove *move) {
    if (move == nullptr || move->feedrate_mm_min < 1.0f || move->feedrate_mm_min > MAX_FEEDRATE_MM_MIN) {
        return false;
    }
    return os_queue_push(&move_queue, move);
}

bool planner_has_work() {
    return active_move.active || !os_queue_is_empty(&move_queue);
}

unsigned int planner_buffer_available() {
    return PLANNER_QUEUE_CAPACITY - os_queue_count(&move_queue);
}

void planner_pause() {
    planner_paused = true;
}

void planner_resume() {
    planner_paused = false;
}

void planner_stop() {
    os_queue_clear(&move_queue);
    active_move.active = false;
    planner_paused = false;
    drv_motor_set_all_enabled(false);
}

bool planner_is_paused() {
    return planner_paused;
}

float planner_steps_per_mm(MotorAxis axis) {
    if (axis < MOTOR_AXIS_X || axis >= MOTOR_AXIS_COUNT) {
        return 0.0f;
    }

    return axis_steps_per_mm[axis];
}

bool planner_set_steps_per_mm(MotorAxis axis, float steps_per_mm) {
    if (axis < MOTOR_AXIS_X || axis >= MOTOR_AXIS_COUNT || steps_per_mm < 1.0f) {
        return false;
    }

    axis_steps_per_mm[axis] = steps_per_mm;
    return true;
}

bool planner_calibrate_steps_per_mm(MotorAxis axis, float commanded_mm, float measured_mm, float *updated_steps_per_mm) {
    if (axis < MOTOR_AXIS_X || axis >= MOTOR_AXIS_COUNT || commanded_mm <= 0.0f || measured_mm <= 0.0f) {
        return false;
    }

    const float updated = axis_steps_per_mm[axis] * (commanded_mm / measured_mm);
    if (!planner_set_steps_per_mm(axis, updated)) {
        return false;
    }

    if (updated_steps_per_mm != nullptr) {
        *updated_steps_per_mm = updated;
    }
    return true;
}

void planner_task() {
    if (planner_paused) {
        return;
    }

    service_active_move();
    if (active_move.active) {
        return;
    }

    PlannerMove move;
    if (!os_queue_pop(&move_queue, &move)) {
        return;
    }

    drv_motor_set_all_enabled(true);
    start_move(&move);
}

void planner_current_position(float *x_mm, float *y_mm, float *z_mm) {
    if (x_mm != nullptr) {
        *x_mm = current_x;
    }
    if (y_mm != nullptr) {
        *y_mm = current_y;
    }
    if (z_mm != nullptr) {
        *z_mm = current_z;
    }
}

void planner_set_position(float x_mm, float y_mm, float z_mm) {
    current_x = x_mm;
    current_y = y_mm;
    current_z = z_mm;
}

void planner_set_axis_position(MotorAxis axis, float position_mm) {
    if (axis == MOTOR_AXIS_X) {
        current_x = position_mm;
    } else if (axis == MOTOR_AXIS_Y) {
        current_y = position_mm;
    } else if (axis == MOTOR_AXIS_Z) {
        current_z = position_mm;
    }
}