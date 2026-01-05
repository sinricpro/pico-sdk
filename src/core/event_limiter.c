/**
 * @file event_limiter.c
 * @brief Event rate limiting implementation for SinricPro
 */

#include "event_limiter.h"
#include <stdio.h>
#include "pico/time.h"

// Get current time in milliseconds
static uint32_t get_millis(void) {
    return to_ms_since_boot(get_absolute_time());
}

void sinricpro_event_limiter_init(sinricpro_event_limiter_t *limiter,
                                  uint32_t min_distance_ms) {
    if (!limiter) return;

    limiter->minimum_distance_ms = min_distance_ms;
    limiter->next_event_time = 0;
    limiter->extra_distance_ms = 0;
    limiter->fail_counter = 0;
}

void sinricpro_event_limiter_init_state(sinricpro_event_limiter_t *limiter) {
    sinricpro_event_limiter_init(limiter, SINRICPRO_EVENT_LIMIT_STATE_MS);
}

void sinricpro_event_limiter_init_sensor(sinricpro_event_limiter_t *limiter) {
    sinricpro_event_limiter_init(limiter, SINRICPRO_EVENT_LIMIT_SENSOR_MS);
}

bool sinricpro_event_limiter_check(sinricpro_event_limiter_t *limiter) {
    if (!limiter) return true;  // Block if invalid

    uint32_t current_millis = get_millis();
    uint32_t fail_threshold = limiter->minimum_distance_ms / 4;

    // Check if enough time has passed
    if (current_millis >= limiter->next_event_time) {
        // Event is allowed

        // Check if we need to adjust backoff
        if (limiter->fail_counter > fail_threshold) {
            // Too many violations - increase backoff
            limiter->extra_distance_ms += limiter->minimum_distance_ms;
            limiter->fail_counter = 0;
        } else {
            // Good behavior - clear backoff
            limiter->extra_distance_ms = 0;
        }

        // Set next allowed event time
        limiter->next_event_time = current_millis +
                                   limiter->minimum_distance_ms +
                                   limiter->extra_distance_ms;

        return false;  // Event ALLOWED
    }

    // Event came too soon - increment failure counter
    limiter->fail_counter++;

    // Warn user if hitting threshold
    if (limiter->fail_counter == fail_threshold) {
        printf("WARNING: Excessive events detected! Adding %lu second delay.\n",
               (unsigned long)(limiter->extra_distance_ms / 1000));
    }

    return true;  // Event BLOCKED
}

uint32_t sinricpro_event_limiter_time_remaining(const sinricpro_event_limiter_t *limiter) {
    if (!limiter) return 0;

    uint32_t current_millis = get_millis();

    if (current_millis >= limiter->next_event_time) {
        return 0;  // Can send now
    }

    return limiter->next_event_time - current_millis;
}

void sinricpro_event_limiter_reset(sinricpro_event_limiter_t *limiter) {
    if (!limiter) return;

    limiter->next_event_time = 0;
    limiter->extra_distance_ms = 0;
    limiter->fail_counter = 0;
}

uint32_t sinricpro_event_limiter_get_backoff(const sinricpro_event_limiter_t *limiter) {
    if (!limiter) return 0;
    return limiter->extra_distance_ms;
}
