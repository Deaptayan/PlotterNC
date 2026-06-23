#ifndef PLOTTER_PROTOCOL_H
#define PLOTTER_PROTOCOL_H

#include <stddef.h>

void protocol_init();
void protocol_task();
void protocol_report_status();
bool protocol_execute_command(const char *input, char *response, size_t response_size);

#endif
