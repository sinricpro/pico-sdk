/**
 * @file lock_controller.h
 * @brief Lock controller capability for SinricPro devices
 */

#ifndef SINRICPRO_LOCK_CONTROLLER_H
#define SINRICPRO_LOCK_CONTROLLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "cJSON.h"
#include "sinricpro/sinricpro_device.h"
#include "sinricpro/event_limiter.h"

/**
 * @brief Lock states
 */
typedef enum {
    SINRICPRO_LOCK_STATE_LOCKED,
    SINRICPRO_LOCK_STATE_UNLOCKED,
    SINRICPRO_LOCK_STATE_JAMMED
} sinricpro_lock_state_enum_t;

/**
 * @brief Lock state callback function type
 *
 * @param device    The device receiving the command
 * @param state     Pointer to lock state (true = lock, false = unlock)
 * @return true if successful, false on error
 */
typedef bool (*sinricpro_lock_state_callback_t)(sinricpro_device_t *device, bool *state);

/**
 * @brief Lock controller capability structure
 */
typedef struct {
    bool locked;
    sinricpro_lock_state_callback_t callback;
    sinricpro_event_limiter_t event_limiter;
} sinricpro_lock_controller_t;

/**
 * @brief Initialize lock controller
 */
void sinricpro_lock_controller_init(sinricpro_lock_controller_t *controller);

/**
 * @brief Set lock state callback
 */
void sinricpro_lock_controller_set_callback(sinricpro_lock_controller_t *controller,
                                             sinricpro_lock_state_callback_t callback);

/**
 * @brief Handle lock state request
 *
 * @param controller Lock controller instance
 * @param device     Device receiving the request
 * @param request    Request JSON object
 * @param response   Response JSON object to populate
 * @return true if handled successfully
 */
bool sinricpro_lock_controller_handle_request(sinricpro_lock_controller_t *controller,
                                                sinricpro_device_t *device,
                                                const cJSON *request,
                                                cJSON *response);

/**
 * @brief Send lock state event to server
 *
 * @param controller Lock controller instance
 * @param device_id  Device ID
 * @param locked     true = locked, false = unlocked
 * @return true if event sent successfully
 */
bool sinricpro_lock_controller_send_event(sinricpro_lock_controller_t *controller,
                                           const char *device_id,
                                           bool locked);

/**
 * @brief Get current lock state
 */
bool sinricpro_lock_controller_is_locked(const sinricpro_lock_controller_t *controller);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_LOCK_CONTROLLER_H
