#ifndef PLOTTER_OS_MUTEX_H
#define PLOTTER_OS_MUTEX_H

struct OsMutex {
    bool locked;
};

void os_mutex_init(OsMutex *mutex);
bool os_mutex_lock(OsMutex *mutex);
void os_mutex_unlock(OsMutex *mutex);
bool os_mutex_is_locked(const OsMutex *mutex);

#endif
