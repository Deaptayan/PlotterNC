#ifndef PLOTTER_OS_QUEUE_H
#define PLOTTER_OS_QUEUE_H

#include <stddef.h>
#include <stdint.h>

struct OsQueue {
    uint8_t *buffer;
    size_t item_size;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t count;
};

void os_queue_init(OsQueue *queue, void *storage, size_t item_size, size_t capacity);
bool os_queue_push(OsQueue *queue, const void *item);
bool os_queue_pop(OsQueue *queue, void *item);
bool os_queue_peek(const OsQueue *queue, void *item);
size_t os_queue_count(const OsQueue *queue);
bool os_queue_is_empty(const OsQueue *queue);
bool os_queue_is_full(const OsQueue *queue);
void os_queue_clear(OsQueue *queue);

#endif
