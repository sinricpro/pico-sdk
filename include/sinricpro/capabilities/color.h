/**
 * @file color.h
 * @brief Color capability for SinricPro (RGB control)
 */

#ifndef SINRICPRO_CAPABILITY_COLOR_H
#define SINRICPRO_CAPABILITY_COLOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "sinricpro/sinricpro_device.h"
#include "sinricpro/event_limiter.h"
#include "cJSON.h"

/**
 * @brief RGB color structure
 */
typedef struct {
    uint8_t r;  // Red (0-255)
    uint8_t g;  // Green (0-255)
    uint8_t b;  // Blue (0-255)
} sinricpro_color_t;

/**
 * @brief Color callback function type
 *
 * @param device Device receiving the request
 * @param color Pointer to color structure (input and output)
 * @return true if handled successfully, false otherwise
 */
typedef bool (*sinricpro_color_callback_t)(sinricpro_device_t *device,
                                           sinricpro_color_t *color);

/**
 * @brief Color capability structure
 */
typedef struct {
    sinricpro_color_t current_color;
    sinricpro_color_callback_t callback;
    sinricpro_event_limiter_t event_limiter;
} sinricpro_color_cap_t;

/**
 * @brief Initialize color capability
 *
 * @param cap Capability structure
 */
void sinricpro_color_init(sinricpro_color_cap_t *cap);

/**
 * @brief Set color callback
 *
 * @param cap Capability structure
 * @param callback Callback function
 */
void sinricpro_color_set_callback(sinricpro_color_cap_t *cap,
                                  sinricpro_color_callback_t callback);

/**
 * @brief Handle setColor request
 *
 * @param cap Capability structure
 * @param device Device pointer
 * @param request Request JSON
 * @param response Response JSON
 * @return true on success, false on failure
 */
bool sinricpro_color_handle_request(sinricpro_color_cap_t *cap,
                                    sinricpro_device_t *device,
                                    const cJSON *request,
                                    cJSON *response);

/**
 * @brief Send color event
 *
 * @param cap Capability structure
 * @param device_id Device ID
 * @param color RGB color
 * @return true on success, false on failure
 */
bool sinricpro_color_send_event(sinricpro_color_cap_t *cap,
                                const char *device_id,
                                sinricpro_color_t color);

/**
 * @brief Get current color
 *
 * @param cap Capability structure
 * @return Current RGB color
 */
sinricpro_color_t sinricpro_color_get_value(const sinricpro_color_cap_t *cap);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_CAPABILITY_COLOR_H
