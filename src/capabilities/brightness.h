/**
 * @file brightness.h
 * @brief Brightness capability for SinricPro devices
 *
 * Provides brightness control (0-100%) for dimmable devices like
 * dim switches and lights.
 */

#ifndef SINRICPRO_CAPABILITY_BRIGHTNESS_H
#define SINRICPRO_CAPABILITY_BRIGHTNESS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "sinricpro/sinricpro_device.h"
#include "core/event_limiter.h"
#include "cJSON.h"

/**
 * @brief Brightness callback function type
 *
 * Called when a setBrightness request is received.
 *
 * @param device     The device receiving the request
 * @param brightness Pointer to brightness value (0-100)
 *                   Callback can modify to report actual brightness
 * @return true if handled successfully, false on error
 */
typedef bool (*sinricpro_brightness_callback_t)(
    sinricpro_device_t *device,
    int *brightness);

/**
 * @brief Adjust brightness callback function type
 *
 * Called when an adjustBrightness request is received.
 *
 * @param device          The device receiving the request
 * @param brightness_delta Pointer to brightness delta (-100 to +100)
 *                         Callback should modify to return new absolute brightness
 * @return true if handled successfully, false on error
 */
typedef bool (*sinricpro_adjust_brightness_callback_t)(
    sinricpro_device_t *device,
    int *brightness_delta);

/**
 * @brief Brightness capability data
 */
typedef struct {
    sinricpro_brightness_callback_t brightness_callback;
    sinricpro_adjust_brightness_callback_t adjust_brightness_callback;
    sinricpro_event_limiter_t event_limiter;
    int current_brightness;  // 0-100
} sinricpro_brightness_t;

/**
 * @brief Initialize Brightness capability
 *
 * @param cap Capability structure to initialize
 */
void sinricpro_brightness_init(sinricpro_brightness_t *cap);

/**
 * @brief Set brightness callback
 *
 * @param cap      Capability structure
 * @param callback Callback function for setBrightness
 */
void sinricpro_brightness_set_callback(sinricpro_brightness_t *cap,
                                       sinricpro_brightness_callback_t callback);

/**
 * @brief Set adjust brightness callback
 *
 * @param cap      Capability structure
 * @param callback Callback function for adjustBrightness
 */
void sinricpro_brightness_set_adjust_callback(sinricpro_brightness_t *cap,
                                              sinricpro_adjust_brightness_callback_t callback);

/**
 * @brief Handle setBrightness request
 *
 * @param cap       Capability structure
 * @param device    Device receiving request
 * @param request   Request JSON
 * @param response  Response JSON to populate
 * @return true if handled successfully
 */
bool sinricpro_brightness_handle_set_request(sinricpro_brightness_t *cap,
                                             sinricpro_device_t *device,
                                             const cJSON *request,
                                             cJSON *response);

/**
 * @brief Handle adjustBrightness request
 *
 * @param cap       Capability structure
 * @param device    Device receiving request
 * @param request   Request JSON
 * @param response  Response JSON to populate
 * @return true if handled successfully
 */
bool sinricpro_brightness_handle_adjust_request(sinricpro_brightness_t *cap,
                                                sinricpro_device_t *device,
                                                const cJSON *request,
                                                cJSON *response);

/**
 * @brief Send brightness event
 *
 * Call this when brightness changes due to physical interaction.
 *
 * @param cap        Capability structure
 * @param device_id  Device ID
 * @param brightness New brightness value (0-100)
 * @return true if event sent (or queued)
 */
bool sinricpro_brightness_send_event(sinricpro_brightness_t *cap,
                                     const char *device_id,
                                     int brightness);

/**
 * @brief Get current brightness
 *
 * @param cap Capability structure
 * @return Current brightness (0-100)
 */
int sinricpro_brightness_get_value(const sinricpro_brightness_t *cap);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_CAPABILITY_BRIGHTNESS_H
