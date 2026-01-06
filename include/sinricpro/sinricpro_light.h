/**
 * @file sinricpro_light.h
 * @brief SinricPro Light device for Raspberry Pi Pico W
 *
 * A Light device supports:
 * - On/Off control (PowerState)
 * - Brightness control (Brightness 0-100%)
 * - RGB color control (Color)
 * - Color temperature control (ColorTemperature in Kelvin)
 *
 * Use this for smart RGB lights, LED strips, or any RGB+CCT lighting.
 *
 * @example
 * @code
 * sinricpro_light_t my_light;
 *
 * bool on_power_state(sinricpro_device_t *device, bool *state) {
 *     // Control your light hardware
 *     return true;
 * }
 *
 * bool on_brightness(sinricpro_device_t *device, int *brightness) {
 *     // Set brightness
 *     return true;
 * }
 *
 * bool on_color(sinricpro_device_t *device, sinricpro_color_t *color) {
 *     // Set RGB color
 *     return true;
 * }
 *
 * bool on_color_temp(sinricpro_device_t *device, int *color_temp) {
 *     // Set color temperature
 *     return true;
 * }
 * @endcode
 */

#ifndef SINRICPRO_LIGHT_H
#define SINRICPRO_LIGHT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sinricpro_device.h"
#include "sinricpro/capabilities/power_state.h"
#include "sinricpro/capabilities/brightness.h"
#include "sinricpro/capabilities/color.h"
#include "sinricpro/capabilities/color_temperature.h"

/**
 * @brief Light device structure
 *
 * Contains base device plus PowerState, Brightness, Color, and ColorTemperature capabilities.
 */
typedef struct {
    sinricpro_device_t base;                    // Must be first member
    sinricpro_power_state_t power_state;
    sinricpro_brightness_t brightness;
    sinricpro_color_cap_t color;
    sinricpro_color_temp_cap_t color_temp;
} sinricpro_light_t;

/**
 * @brief Initialize a Light device
 *
 * @param device    Light device to initialize
 * @param device_id Device ID from SinricPro portal (24-char hex)
 * @return true on success, false on failure
 */
bool sinricpro_light_init(sinricpro_light_t *device, const char *device_id);

/**
 * @brief Set power state callback
 *
 * @param device    Light device
 * @param callback  Callback function
 */
void sinricpro_light_on_power_state(sinricpro_light_t *device,
                                    sinricpro_power_state_callback_t callback);

/**
 * @brief Set brightness callback
 *
 * @param device    Light device
 * @param callback  Callback function
 */
void sinricpro_light_on_brightness(sinricpro_light_t *device,
                                   sinricpro_brightness_callback_t callback);

/**
 * @brief Set adjust brightness callback
 *
 * @param device    Light device
 * @param callback  Callback function
 */
void sinricpro_light_on_adjust_brightness(sinricpro_light_t *device,
                                          sinricpro_adjust_brightness_callback_t callback);

/**
 * @brief Set color callback
 *
 * @param device    Light device
 * @param callback  Callback function
 */
void sinricpro_light_on_color(sinricpro_light_t *device,
                              sinricpro_color_callback_t callback);

/**
 * @brief Set color temperature callback
 *
 * @param device    Light device
 * @param callback  Callback function
 */
void sinricpro_light_on_color_temperature(sinricpro_light_t *device,
                                          sinricpro_color_temp_callback_t callback);

/**
 * @brief Set increase color temperature callback
 *
 * @param device    Light device
 * @param callback  Callback function
 */
void sinricpro_light_on_increase_color_temperature(sinricpro_light_t *device,
                                                   sinricpro_color_temp_adjust_callback_t callback);

/**
 * @brief Set decrease color temperature callback
 *
 * @param device    Light device
 * @param callback  Callback function
 */
void sinricpro_light_on_decrease_color_temperature(sinricpro_light_t *device,
                                                   sinricpro_color_temp_adjust_callback_t callback);

/**
 * @brief Send power state event
 *
 * @param device    Light device
 * @param state     Power state (true=ON, false=OFF)
 * @return true if event sent successfully
 */
bool sinricpro_light_send_power_state_event(sinricpro_light_t *device, bool state);

/**
 * @brief Send brightness event
 *
 * @param device     Light device
 * @param brightness Brightness value (0-100)
 * @return true if event sent successfully
 */
bool sinricpro_light_send_brightness_event(sinricpro_light_t *device, int brightness);

/**
 * @brief Send color event
 *
 * @param device Light device
 * @param color  RGB color
 * @return true if event sent successfully
 */
bool sinricpro_light_send_color_event(sinricpro_light_t *device, sinricpro_color_t color);

/**
 * @brief Send color temperature event
 *
 * @param device     Light device
 * @param color_temp Color temperature in Kelvin (2200-7000)
 * @return true if event sent successfully
 */
bool sinricpro_light_send_color_temp_event(sinricpro_light_t *device, int color_temp);

/**
 * @brief Get current power state
 *
 * @param device Light device
 * @return Current power state
 */
bool sinricpro_light_get_power_state(const sinricpro_light_t *device);

/**
 * @brief Get current brightness
 *
 * @param device Light device
 * @return Current brightness (0-100)
 */
int sinricpro_light_get_brightness(const sinricpro_light_t *device);

/**
 * @brief Get current color
 *
 * @param device Light device
 * @return Current RGB color
 */
sinricpro_color_t sinricpro_light_get_color(const sinricpro_light_t *device);

/**
 * @brief Get current color temperature
 *
 * @param device Light device
 * @return Current color temperature in Kelvin
 */
int sinricpro_light_get_color_temp(const sinricpro_light_t *device);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_LIGHT_H
