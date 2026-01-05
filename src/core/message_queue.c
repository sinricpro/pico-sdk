/**
 * @file message_queue.c
 * @brief Thread-safe message queue implementation for SinricPro
 */

#include "message_queue.h"
#include <string.h>
#include "pico/critical_section.h"

// Critical section for thread safety
static critical_section_t queue_cs;
static bool cs_initialized = false;

static void ensure_cs_init(void) {
    if (!cs_initialized) {
        critical_section_init(&queue_cs);
        cs_initialized = true;
    }
}

void sinricpro_queue_init(sinricpro_queue_t *queue) {
    if (!queue) return;

    ensure_cs_init();

    critical_section_enter_blocking(&queue_cs);

    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;

    for (size_t i = 0; i < SINRICPRO_MESSAGE_QUEUE_SIZE; i++) {
        queue->messages[i].in_use = false;
        queue->messages[i].length = 0;
        queue->messages[i].interface = SINRICPRO_IF_UNKNOWN;
    }

    critical_section_exit(&queue_cs);
}

bool sinricpro_queue_is_empty(const sinricpro_queue_t *queue) {
    if (!queue) return true;
    return queue->count == 0;
}

bool sinricpro_queue_is_full(const sinricpro_queue_t *queue) {
    if (!queue) return true;
    return queue->count >= SINRICPRO_MESSAGE_QUEUE_SIZE;
}

size_t sinricpro_queue_count(const sinricpro_queue_t *queue) {
    if (!queue) return 0;
    return queue->count;
}

bool sinricpro_queue_push(sinricpro_queue_t *queue,
                          sinricpro_interface_t interface,
                          const char *message,
                          size_t length) {
    if (!queue || !message || length == 0) {
        return false;
    }

    // Truncate if message is too long
    if (length >= SINRICPRO_MAX_MESSAGE_SIZE) {
        length = SINRICPRO_MAX_MESSAGE_SIZE - 1;
    }

    ensure_cs_init();
    critical_section_enter_blocking(&queue_cs);

    // Check if queue is full
    if (queue->count >= SINRICPRO_MESSAGE_QUEUE_SIZE) {
        critical_section_exit(&queue_cs);
        return false;
    }

    // Get slot at head position
    sinricpro_message_t *slot = &queue->messages[queue->head];

    // Copy message data
    memcpy(slot->message, message, length);
    slot->message[length] = '\0';
    slot->length = length;
    slot->interface = interface;
    slot->in_use = true;

    // Advance head pointer (wrap around)
    queue->head = (queue->head + 1) % SINRICPRO_MESSAGE_QUEUE_SIZE;
    queue->count++;

    critical_section_exit(&queue_cs);
    return true;
}

bool sinricpro_queue_pop(sinricpro_queue_t *queue,
                         sinricpro_interface_t *interface,
                         char *message,
                         size_t max_len,
                         size_t *length) {
    if (!queue || !message || max_len == 0) {
        return false;
    }

    ensure_cs_init();
    critical_section_enter_blocking(&queue_cs);

    // Check if queue is empty
    if (queue->count == 0) {
        critical_section_exit(&queue_cs);
        return false;
    }

    // Get slot at tail position
    sinricpro_message_t *slot = &queue->messages[queue->tail];

    if (!slot->in_use) {
        critical_section_exit(&queue_cs);
        return false;
    }

    // Calculate copy length
    size_t copy_len = slot->length;
    if (copy_len >= max_len) {
        copy_len = max_len - 1;
    }

    // Copy message data
    memcpy(message, slot->message, copy_len);
    message[copy_len] = '\0';

    if (interface) {
        *interface = slot->interface;
    }
    if (length) {
        *length = slot->length;
    }

    // Clear slot
    slot->in_use = false;
    slot->length = 0;

    // Advance tail pointer (wrap around)
    queue->tail = (queue->tail + 1) % SINRICPRO_MESSAGE_QUEUE_SIZE;
    queue->count--;

    critical_section_exit(&queue_cs);
    return true;
}

bool sinricpro_queue_peek(const sinricpro_queue_t *queue,
                          sinricpro_interface_t *interface,
                          char *message,
                          size_t max_len,
                          size_t *length) {
    if (!queue || !message || max_len == 0) {
        return false;
    }

    ensure_cs_init();
    critical_section_enter_blocking(&queue_cs);

    // Check if queue is empty
    if (queue->count == 0) {
        critical_section_exit(&queue_cs);
        return false;
    }

    // Get slot at tail position (don't remove)
    const sinricpro_message_t *slot = &queue->messages[queue->tail];

    if (!slot->in_use) {
        critical_section_exit(&queue_cs);
        return false;
    }

    // Calculate copy length
    size_t copy_len = slot->length;
    if (copy_len >= max_len) {
        copy_len = max_len - 1;
    }

    // Copy message data
    memcpy(message, slot->message, copy_len);
    message[copy_len] = '\0';

    if (interface) {
        *interface = slot->interface;
    }
    if (length) {
        *length = slot->length;
    }

    critical_section_exit(&queue_cs);
    return true;
}

void sinricpro_queue_clear(sinricpro_queue_t *queue) {
    if (!queue) return;

    ensure_cs_init();
    critical_section_enter_blocking(&queue_cs);

    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;

    for (size_t i = 0; i < SINRICPRO_MESSAGE_QUEUE_SIZE; i++) {
        queue->messages[i].in_use = false;
        queue->messages[i].length = 0;
    }

    critical_section_exit(&queue_cs);
}
