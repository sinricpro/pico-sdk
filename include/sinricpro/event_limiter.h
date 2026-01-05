/**
 * @file event_limiter.h
 * @brief Event rate limiting with adaptive backoff for SinricPro
 *
 * Prevents excessive event sending by enforcing minimum intervals
 * between events. Implements adaptive backoff when limits are violated.
 */

#ifndef SINRICPRO_EVENT_LIMITER_H
#define SINRICPRO_EVENT_LIMITER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "sinricpro/sinricpro_config.h"

/**
 * @brief Event limiter structure
 */
typedef struct {
    uint32_t minimum_distance_ms;   // Minimum time between events
    uint32_t next_event_time;       // Timestamp when next event is allowed
    uint32_t extra_distance_ms;     // Additional delay from backoff
    uint32_t fail_counter;          // Count of rate limit violations
} sinricpro_event_limiter_t;

/**
 * @brief Initialize an event limiter
 *
 * @param limiter           Pointer to event limiter structure
 * @param min_distance_ms   Minimum time between events in milliseconds
 */
void sinricpro_event_limiter_init(sinricpro_event_limiter_t *limiter,
                                  uint32_t min_distance_ms);

/**
 * @brief Create a state event limiter (1 second default)
 *
 * @param limiter Pointer to event limiter structure
 */
void sinricpro_event_limiter_init_state(sinricpro_event_limiter_t *limiter);

/**
 * @brief Create a sensor event limiter (60 seconds default)
 *
 * @param limiter Pointer to event limiter structure
 */
void sinricpro_event_limiter_init_sensor(sinricpro_event_limiter_t *limiter);

/**
 * @brief Check if an event is allowed (rate limited)
 *
 * Returns true if the event should be blocked (rate limited).
 * Returns false if the event is allowed to proceed.
 *
 * This function also implements adaptive backoff: if events are
 * consistently rate-limited, the delay increases.
 *
 * @param limiter Pointer to event limiter structure
 * @return true if event should be BLOCKED, false if allowed
 */
bool sinricpro_event_limiter_check(sinricpro_event_limiter_t *limiter);

/**
 * @brief Get time until next event is allowed
 *
 * @param limiter Pointer to event limiter structure
 * @return Milliseconds until next event, or 0 if allowed now
 */
uint32_t sinricpro_event_limiter_time_remaining(const sinricpro_event_limiter_t *limiter);

/**
 * @brief Reset the event limiter
 *
 * Clears backoff and failure counters, allowing immediate events.
 *
 * @param limiter Pointer to event limiter structure
 */
void sinricpro_event_limiter_reset(sinricpro_event_limiter_t *limiter);

/**
 * @brief Get current backoff delay
 *
 * @param limiter Pointer to event limiter structure
 * @return Current extra delay in milliseconds due to backoff
 */
uint32_t sinricpro_event_limiter_get_backoff(const sinricpro_event_limiter_t *limiter);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_EVENT_LIMITER_H
