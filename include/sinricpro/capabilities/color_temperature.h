/**
 * @file color_temperature.h
 * @brief Color temperature capability for SinricPro (Kelvin control)
 *
 * Color temperature range: 2200K (warm white) to 7000K (cool white)
 */

#ifndef SINRICPRO_CAPABILITY_COLOR_TEMPERATURE_H
#define SINRICPRO_CAPABILITY_COLOR_TEMPERATURE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "sinricpro/sinricpro_device.h"
#include "sinricpro/event_limiter.h"
#include "cJSON.h"

/**
 * @brief Color temperature callback function type (for setColorTemperature)
 *
 * @param device Device receiving the request
 * @param color_temp Pointer to color temperature in Kelvin (input and output)
 * @return true if handled successfully, false otherwise
 */
typedef bool (*sinricpro_color_temp_callback_t)(sinricpro_device_t *device,
                                                int *color_temp);

/**
 * @brief Color temperature adjust callback function type
 * (for increase/decrease ColorTemperature)
 *
 * @param device Device receiving the request
 * @param delta Pointer to delta value (+1 for increase, -1 for decrease)
 *              Callback should return absolute temperature value
 * @return true if handled successfully, false otherwise
 */
typedef bool (*sinricpro_color_temp_adjust_callback_t)(sinricpro_device_t *device,
                                                       int *delta);

/**
 * @brief Color temperature capability structure
 */
typedef struct {
    int current_temp;  // Color temperature in Kelvin
    sinricpro_color_temp_callback_t callback;
    sinricpro_color_temp_adjust_callback_t increase_callback;
    sinricpro_color_temp_adjust_callback_t decrease_callback;
    sinricpro_event_limiter_t event_limiter;
} sinricpro_color_temp_cap_t;

/**
 * @brief Initialize color temperature capability
 *
 * @param cap Capability structure
 */
void sinricpro_color_temp_init(sinricpro_color_temp_cap_t *cap);

/**
 * @brief Set color temperature callback
 *
 * @param cap Capability structure
 * @param callback Callback function for setColorTemperature
 */
void sinricpro_color_temp_set_callback(sinricpro_color_temp_cap_t *cap,
                                       sinricpro_color_temp_callback_t callback);

/**
 * @brief Set increase color temperature callback
 *
 * @param cap Capability structure
 * @param callback Callback function for increaseColorTemperature
 */
void sinricpro_color_temp_set_increase_callback(sinricpro_color_temp_cap_t *cap,
                                                sinricpro_color_temp_adjust_callback_t callback);

/**
 * @brief Set decrease color temperature callback
 *
 * @param cap Capability structure
 * @param callback Callback function for decreaseColorTemperature
 */
void sinricpro_color_temp_set_decrease_callback(sinricpro_color_temp_cap_t *cap,
                                                sinricpro_color_temp_adjust_callback_t callback);

/**
 * @brief Handle color temperature requests
 *
 * @param cap Capability structure
 * @param device Device pointer
 * @param action Action name
 * @param request Request JSON
 * @param response Response JSON
 * @return true on success, false on failure
 */
bool sinricpro_color_temp_handle_request(sinricpro_color_temp_cap_t *cap,
                                         sinricpro_device_t *device,
                                         const char *action,
                                         const cJSON *request,
                                         cJSON *response);

/**
 * @brief Send color temperature event
 *
 * @param cap Capability structure
 * @param device_id Device ID
 * @param color_temp Color temperature in Kelvin (2200-7000)
 * @return true on success, false on failure
 */
bool sinricpro_color_temp_send_event(sinricpro_color_temp_cap_t *cap,
                                     const char *device_id,
                                     int color_temp);

/**
 * @brief Get current color temperature
 *
 * @param cap Capability structure
 * @return Current color temperature in Kelvin
 */
int sinricpro_color_temp_get_value(const sinricpro_color_temp_cap_t *cap);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_CAPABILITY_COLOR_TEMPERATURE_H
