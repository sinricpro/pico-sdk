/**
 * @file door_controller.h
 * @brief Door controller capability for SinricPro devices
 */

#ifndef SINRICPRO_DOOR_CONTROLLER_H
#define SINRICPRO_DOOR_CONTROLLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "cJSON.h"
#include "sinricpro/sinricpro_device.h"
#include "sinricpro/event_limiter.h"

/**
 * @brief Door state callback function type
 *
 * @param device    The device receiving the command
 * @param state     Pointer to door state (true = close, false = open)
 * @return true if successful, false on error
 */
typedef bool (*sinricpro_door_state_callback_t)(sinricpro_device_t *device, bool *state);

/**
 * @brief Door controller capability structure
 */
typedef struct {
    bool closed;
    sinricpro_door_state_callback_t callback;
    sinricpro_event_limiter_t event_limiter;
} sinricpro_door_controller_t;

/**
 * @brief Initialize door controller
 */
void sinricpro_door_controller_init(sinricpro_door_controller_t *controller);

/**
 * @brief Set door state callback
 */
void sinricpro_door_controller_set_callback(sinricpro_door_controller_t *controller,
                                              sinricpro_door_state_callback_t callback);

/**
 * @brief Handle door state request
 *
 * @param controller Door controller instance
 * @param device     Device receiving the request
 * @param request    Request JSON object
 * @param response   Response JSON object to populate
 * @return true if handled successfully
 */
bool sinricpro_door_controller_handle_request(sinricpro_door_controller_t *controller,
                                                sinricpro_device_t *device,
                                                const cJSON *request,
                                                cJSON *response);

/**
 * @brief Send door state event to server
 *
 * @param controller Door controller instance
 * @param device_id  Device ID
 * @param closed     true = closed, false = open
 * @return true if event sent successfully
 */
bool sinricpro_door_controller_send_event(sinricpro_door_controller_t *controller,
                                            const char *device_id,
                                            bool closed);

/**
 * @brief Get current door state
 */
bool sinricpro_door_controller_is_closed(const sinricpro_door_controller_t *controller);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_DOOR_CONTROLLER_H
