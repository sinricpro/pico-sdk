/**
 * @file power_level.h
 * @brief Power level capability for SinricPro
 *
 * Controls the power level (0-100) of a device.
 * Used by DimSwitch devices for dimming control.
 */

#ifndef SINRICPRO_CAPABILITY_POWER_LEVEL_H
#define SINRICPRO_CAPABILITY_POWER_LEVEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "sinricpro/sinricpro_device.h"
#include "sinricpro/event_limiter.h"
#include "cJSON.h"

/**
 * @brief Power level callback function type (for setPowerLevel)
 *
 * @param device Device receiving the request
 * @param power_level Pointer to power level value (0-100, input and output)
 * @return true if handled successfully, false otherwise
 */
typedef bool (*sinricpro_power_level_callback_t)(sinricpro_device_t *device,
                                                  int *power_level);

/**
 * @brief Adjust power level callback function type (for adjustPowerLevel)
 *
 * @param device Device receiving the request
 * @param delta Pointer to delta value (-100 to +100, input).
 *              Callback should modify this to return absolute power level (0-100, output)
 * @return true if handled successfully, false otherwise
 */
typedef bool (*sinricpro_adjust_power_level_callback_t)(sinricpro_device_t *device,
                                                         int *delta);

/**
 * @brief Power level capability structure
 */
typedef struct {
    int current_power_level;  // Current power level (0-100)
    sinricpro_power_level_callback_t callback;
    sinricpro_adjust_power_level_callback_t adjust_callback;
    sinricpro_event_limiter_t event_limiter;
} sinricpro_power_level_t;

/**
 * @brief Initialize power level capability
 *
 * @param power_level Capability structure
 */
void sinricpro_power_level_init(sinricpro_power_level_t *power_level);

/**
 * @brief Set power level callback
 *
 * @param power_level Capability structure
 * @param callback Callback function for setPowerLevel
 */
void sinricpro_power_level_set_callback(sinricpro_power_level_t *power_level,
                                        sinricpro_power_level_callback_t callback);

/**
 * @brief Set adjust power level callback
 *
 * @param power_level Capability structure
 * @param callback Callback function for adjustPowerLevel
 */
void sinricpro_power_level_set_adjust_callback(sinricpro_power_level_t *power_level,
                                               sinricpro_adjust_power_level_callback_t callback);

/**
 * @brief Handle setPowerLevel request
 *
 * @param power_level Capability structure
 * @param device Device pointer
 * @param request Request JSON
 * @param response Response JSON
 * @return true on success, false on failure
 */
bool sinricpro_power_level_handle_set_request(sinricpro_power_level_t *power_level,
                                               sinricpro_device_t *device,
                                               const cJSON *request,
                                               cJSON *response);

/**
 * @brief Handle adjustPowerLevel request
 *
 * @param power_level Capability structure
 * @param device Device pointer
 * @param request Request JSON
 * @param response Response JSON
 * @return true on success, false on failure
 */
bool sinricpro_power_level_handle_adjust_request(sinricpro_power_level_t *power_level,
                                                  sinricpro_device_t *device,
                                                  const cJSON *request,
                                                  cJSON *response);

/**
 * @brief Send power level event
 *
 * @param power_level Capability structure
 * @param device_id Device ID
 * @param level Power level (0-100)
 * @return true on success, false on failure
 */
bool sinricpro_power_level_send_event(sinricpro_power_level_t *power_level,
                                      const char *device_id,
                                      int level);

/**
 * @brief Get current power level value
 *
 * @param power_level Capability structure
 * @return Current power level (0-100)
 */
int sinricpro_power_level_get_value(const sinricpro_power_level_t *power_level);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_CAPABILITY_POWER_LEVEL_H
