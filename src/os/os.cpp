#include "os.h"

#include "../hal/hal.h"

#include <stddef.h>
#include <string.h>

static const uint8_t OS_MAX_TASKS = 12;
static OsTask tasks[OS_MAX_TASKS];
static uint8_t task_count = 0;

void os_init() {
    task_count = 0;
    memset(tasks, 0, sizeof(tasks));
}

bool os_task_add(const char *name, OsTaskCallback callback, uint32_t interval_ms, uint8_t priority) {
    if (task_count >= OS_MAX_TASKS || callback == nullptr || interval_ms == 0) {
        return false;
    }

    OsTask task;
    task.name = name;
    task.callback = callback;
    task.interval_ms = interval_ms;
    task.priority = priority;
    task.next_run_ms = hal_millis() + interval_ms;
    task.enabled = true;

    uint8_t insert_at = task_count;
    while (insert_at > 0 && tasks[insert_at - 1].priority > priority) {
        tasks[insert_at] = tasks[insert_at - 1];
        --insert_at;
    }

    tasks[insert_at] = task;
    ++task_count;
    return true;
}

void os_task_enable(const char *name, bool enabled) {
    if (name == nullptr) {
        return;
    }

    for (uint8_t i = 0; i < task_count; ++i) {
        if (tasks[i].name != nullptr && strcmp(tasks[i].name, name) == 0) {
            tasks[i].enabled = enabled;
            return;
        }
    }
}

void os_run() {
    const uint32_t now = hal_millis();
    for (uint8_t i = 0; i < task_count; ++i) {
        OsTask *task = &tasks[i];
        if (!task->enabled) {
            continue;
        }

        if ((int32_t)(now - task->next_run_ms) >= 0) {
            task->next_run_ms = now + task->interval_ms;
            task->callback();
        }
    }
}

bool os_timer_expired(uint32_t *last_ms, uint32_t interval_ms) {
    if (last_ms == nullptr) {
        return false;
    }

    const uint32_t now = hal_millis();
    if ((uint32_t)(now - *last_ms) >= interval_ms) {
        *last_ms = now;
        return true;
    }
    return false;
}

void os_sleep_ms(uint32_t delay_ms) {
    hal_delay_ms(delay_ms);
}
