#include "os_mutex.h"

void os_mutex_init(OsMutex *mutex) {
    if (mutex != nullptr) {
        mutex->locked = false;
    }
}

bool os_mutex_lock(OsMutex *mutex) {
    if (mutex == nullptr || mutex->locked) {
        return false;
    }

    mutex->locked = true;
    return true;
}

void os_mutex_unlock(OsMutex *mutex) {
    if (mutex != nullptr) {
        mutex->locked = false;
    }
}

bool os_mutex_is_locked(const OsMutex *mutex) {
    return mutex != nullptr && mutex->locked;
}
