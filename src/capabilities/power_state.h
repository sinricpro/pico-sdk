/**
 * @file power_state.h
 * @brief PowerState capability for SinricPro devices
 *
 * Provides on/off power control for devices like switches, lights, etc.
 */

#ifndef SINRICPRO_CAPABILITY_POWER_STATE_H
#define SINRICPRO_CAPABILITY_POWER_STATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "sinricpro/sinricpro_device.h"
#include "core/event_limiter.h"
#include "cJSON.h"

/**
 * @brief Power state callback function type
 *
 * Called when a setPowerState request is received.
 *
 * @param device The device receiving the request
 * @param state  Pointer to the requested state (true=ON, false=OFF)
 *               Callback can modify this to report actual state
 * @return true if handled successfully, false on error
 */
typedef bool (*sinricpro_power_state_callback_t)(
    sinricpro_device_t *device,
    bool *state);

/**
 * @brief PowerState capability data
 */
typedef struct {
    sinricpro_power_state_callback_t callback;
    sinricpro_event_limiter_t event_limiter;
    bool current_state;
} sinricpro_power_state_t;

/**
 * @brief Initialize PowerState capability
 *
 * @param cap Capability structure to initialize
 */
void sinricpro_power_state_init(sinricpro_power_state_t *cap);

/**
 * @brief Set power state callback
 *
 * @param cap      Capability structure
 * @param callback Callback function
 */
void sinricpro_power_state_set_callback(sinricpro_power_state_t *cap,
                                        sinricpro_power_state_callback_t callback);

/**
 * @brief Handle setPowerState request
 *
 * @param cap       Capability structure
 * @param device    Device receiving request
 * @param request   Request JSON
 * @param response  Response JSON to populate
 * @return true if handled successfully
 */
bool sinricpro_power_state_handle_request(sinricpro_power_state_t *cap,
                                          sinricpro_device_t *device,
                                          const cJSON *request,
                                          cJSON *response);

/**
 * @brief Send power state event
 *
 * Call this when the power state changes due to physical interaction.
 *
 * @param cap       Capability structure
 * @param device_id Device ID
 * @param state     New power state
 * @return true if event sent (or queued)
 */
bool sinricpro_power_state_send_event(sinricpro_power_state_t *cap,
                                      const char *device_id,
                                      bool state);

/**
 * @brief Get current power state
 *
 * @param cap Capability structure
 * @return Current state
 */
bool sinricpro_power_state_get_state(const sinricpro_power_state_t *cap);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_CAPABILITY_POWER_STATE_H
