#include "os_queue.h"

#include <string.h>

void os_queue_init(OsQueue *queue, void *storage, size_t item_size, size_t capacity) {
    if (queue == nullptr) {
        return;
    }

    queue->buffer = static_cast<uint8_t *>(storage);
    queue->item_size = item_size;
    queue->capacity = capacity;
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
}

bool os_queue_push(OsQueue *queue, const void *item) {
    if (queue == nullptr || queue->buffer == nullptr || item == nullptr || os_queue_is_full(queue)) {
        return false;
    }

    memcpy(queue->buffer + (queue->tail * queue->item_size), item, queue->item_size);
    queue->tail = (queue->tail + 1) % queue->capacity;
    ++queue->count;
    return true;
}

bool os_queue_pop(OsQueue *queue, void *item) {
    if (queue == nullptr || queue->buffer == nullptr || item == nullptr || os_queue_is_empty(queue)) {
        return false;
    }

    memcpy(item, queue->buffer + (queue->head * queue->item_size), queue->item_size);
    queue->head = (queue->head + 1) % queue->capacity;
    --queue->count;
    return true;
}

bool os_queue_peek(const OsQueue *queue, void *item) {
    if (queue == nullptr || queue->buffer == nullptr || item == nullptr || os_queue_is_empty(queue)) {
        return false;
    }

    memcpy(item, queue->buffer + (queue->head * queue->item_size), queue->item_size);
    return true;
}

size_t os_queue_count(const OsQueue *queue) {
    return queue == nullptr ? 0 : queue->count;
}

bool os_queue_is_empty(const OsQueue *queue) {
    return queue == nullptr || queue->count == 0;
}

bool os_queue_is_full(const OsQueue *queue) {
    return queue == nullptr || queue->capacity == 0 || queue->count >= queue->capacity;
}

void os_queue_clear(OsQueue *queue) {
    if (queue == nullptr) {
        return;
    }

    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
}
