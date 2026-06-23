#include "storage.h"

void storage_init() {
}

bool storage_read_setting(const char *key, char *value, size_t value_size) {
    (void)key;
    if (value != nullptr && value_size > 0) {
        value[0] = '\0';
    }
    return false;
}

bool storage_write_setting(const char *key, const char *value) {
    (void)key;
    (void)value;
    return false;
}
