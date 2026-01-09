/**
 * @file sinricpro_lock.h
 * @brief SinricPro Lock device for Raspberry Pi Pico W
 *
 * A Lock device supports lock/unlock control for smart locks.
 * Use this for door locks, cabinet locks, or any electronic locking mechanism.
 *
 * @example
 * @code
 * sinricpro_lock_t my_lock;
 *
 * bool on_lock_state(sinricpro_device_t *device, bool *state) {
 *     // *state: true = lock, false = unlock
 *     gpio_put(LOCK_PIN, *state);
 *     return true; // Return false if lock is jammed
 * }
 * @endcode
 */

#ifndef SINRICPRO_LOCK_H
#define SINRICPRO_LOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sinricpro_device.h"
#include "sinricpro/capabilities/lock_controller.h"

/**
 * @brief Lock device structure
 */
typedef struct {
    sinricpro_device_t base;
    sinricpro_lock_controller_t lock_controller;
} sinricpro_lock_t;

bool sinricpro_lock_init(sinricpro_lock_t *device, const char *device_id);

void sinricpro_lock_on_lock_state(sinricpro_lock_t *device,
                                   sinricpro_lock_state_callback_t callback);

bool sinricpro_lock_send_lock_state_event(sinricpro_lock_t *device, bool locked);

bool sinricpro_lock_is_locked(const sinricpro_lock_t *device);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_LOCK_H
