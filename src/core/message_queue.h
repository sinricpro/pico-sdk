/**
 * @file message_queue.h
 * @brief Thread-safe message queue (ring buffer) for SinricPro
 *
 * Provides a fixed-size ring buffer for queuing incoming and outgoing
 * messages. Designed to be interrupt-safe for use with the Pico SDK.
 */

#ifndef SINRICPRO_MESSAGE_QUEUE_H
#define SINRICPRO_MESSAGE_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "sinricpro/sinricpro_config.h"

/**
 * @brief Message interface type
 */
typedef enum {
    SINRICPRO_IF_UNKNOWN   = 0,
    SINRICPRO_IF_WEBSOCKET = 1,
    SINRICPRO_IF_UDP       = 2
} sinricpro_interface_t;

/**
 * @brief Message structure
 */
typedef struct {
    sinricpro_interface_t interface;
    char message[SINRICPRO_MAX_MESSAGE_SIZE];
    size_t length;
    bool in_use;
} sinricpro_message_t;

/**
 * @brief Message queue structure (ring buffer)
 */
typedef struct {
    sinricpro_message_t messages[SINRICPRO_MESSAGE_QUEUE_SIZE];
    volatile size_t head;    // Next write position
    volatile size_t tail;    // Next read position
    volatile size_t count;   // Number of items in queue
} sinricpro_queue_t;

/**
 * @brief Initialize a message queue
 *
 * @param queue Pointer to queue structure
 */
void sinricpro_queue_init(sinricpro_queue_t *queue);

/**
 * @brief Check if queue is empty
 *
 * @param queue Pointer to queue structure
 * @return true if empty, false otherwise
 */
bool sinricpro_queue_is_empty(const sinricpro_queue_t *queue);

/**
 * @brief Check if queue is full
 *
 * @param queue Pointer to queue structure
 * @return true if full, false otherwise
 */
bool sinricpro_queue_is_full(const sinricpro_queue_t *queue);

/**
 * @brief Get number of items in queue
 *
 * @param queue Pointer to queue structure
 * @return Number of items
 */
size_t sinricpro_queue_count(const sinricpro_queue_t *queue);

/**
 * @brief Push a message onto the queue
 *
 * @param queue     Pointer to queue structure
 * @param interface Message interface type
 * @param message   Message string (will be copied)
 * @param length    Message length
 * @return true on success, false if queue is full
 */
bool sinricpro_queue_push(sinricpro_queue_t *queue,
                          sinricpro_interface_t interface,
                          const char *message,
                          size_t length);

/**
 * @brief Pop a message from the queue
 *
 * @param queue     Pointer to queue structure
 * @param interface Output: interface type of message
 * @param message   Output buffer for message
 * @param max_len   Size of output buffer
 * @param length    Output: actual message length
 * @return true on success, false if queue is empty
 */
bool sinricpro_queue_pop(sinricpro_queue_t *queue,
                         sinricpro_interface_t *interface,
                         char *message,
                         size_t max_len,
                         size_t *length);

/**
 * @brief Peek at the front message without removing it
 *
 * @param queue     Pointer to queue structure
 * @param interface Output: interface type of message
 * @param message   Output buffer for message
 * @param max_len   Size of output buffer
 * @param length    Output: actual message length
 * @return true on success, false if queue is empty
 */
bool sinricpro_queue_peek(const sinricpro_queue_t *queue,
                          sinricpro_interface_t *interface,
                          char *message,
                          size_t max_len,
                          size_t *length);

/**
 * @brief Clear all messages from the queue
 *
 * @param queue Pointer to queue structure
 */
void sinricpro_queue_clear(sinricpro_queue_t *queue);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_MESSAGE_QUEUE_H
