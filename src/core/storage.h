#ifndef PLOTTER_STORAGE_H
#define PLOTTER_STORAGE_H

#include <stddef.h>

void storage_init();
bool storage_read_setting(const char *key, char *value, size_t value_size);
bool storage_write_setting(const char *key, const char *value);

#endif
