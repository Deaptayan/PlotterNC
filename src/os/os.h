#ifndef PLOTTER_OS_H
#define PLOTTER_OS_H

#include <stdint.h>

typedef void (*OsTaskCallback)();

struct OsTask {
    const char *name;
    OsTaskCallback callback;
    uint32_t interval_ms;
    uint8_t priority;
    uint32_t next_run_ms;
    bool enabled;
};

void os_init();
bool os_task_add(const char *name, OsTaskCallback callback, uint32_t interval_ms, uint8_t priority);
void os_task_enable(const char *name, bool enabled);
void os_run();

bool os_timer_expired(uint32_t *last_ms, uint32_t interval_ms);
void os_sleep_ms(uint32_t delay_ms);

#endif
