/**
 * @file sinricpro_dimswitch.h
 * @brief SinricPro DimSwitch device for Raspberry Pi Pico W
 *
 * A DimSwitch device supports on/off control and brightness adjustment.
 * Use this for dimmable lights, LED strips, or any device with variable output.
 *
 * @example
 * @code
 * sinricpro_dimswitch_t my_dimmer;
 *
 * bool on_power_state(sinricpro_device_t *device, bool *state) {
 *     gpio_put(RELAY_PIN, *state);
 *     return true;
 * }
 *
 * bool on_brightness(sinricpro_device_t *device, int *brightness) {
 *     pwm_set_gpio_level(PWM_PIN, *brightness * 255 / 100);
 *     return true;
 * }
 *
 * int main() {
 *     sinricpro_dimswitch_init(&my_dimmer, "device-id-here");
 *     sinricpro_dimswitch_on_power_state(&my_dimmer, on_power_state);
 *     sinricpro_dimswitch_on_brightness(&my_dimmer, on_brightness);
 *     sinricpro_add_device((sinricpro_device_t *)&my_dimmer);
 *     // ...
 * }
 * @endcode
 */

#ifndef SINRICPRO_DIMSWITCH_H
#define SINRICPRO_DIMSWITCH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sinricpro_device.h"
#include "sinricpro/capabilities/power_state.h"
#include "sinricpro/capabilities/brightness.h"

/**
 * @brief DimSwitch device structure
 *
 * Contains base device plus PowerState and Brightness capabilities.
 */
typedef struct {
    sinricpro_device_t base;                // Must be first member
    sinricpro_power_state_t power_state;
    sinricpro_brightness_t brightness;
} sinricpro_dimswitch_t;

/**
 * @brief Initialize a DimSwitch device
 *
 * @param device    DimSwitch device to initialize
 * @param device_id Device ID from SinricPro portal (24-char hex)
 * @return true on success, false on failure
 */
bool sinricpro_dimswitch_init(sinricpro_dimswitch_t *device, const char *device_id);

/**
 * @brief Set power state callback
 *
 * Register a callback to handle setPowerState requests.
 *
 * @param device    DimSwitch device
 * @param callback  Callback function
 */
void sinricpro_dimswitch_on_power_state(sinricpro_dimswitch_t *device,
                                        sinricpro_power_state_callback_t callback);

/**
 * @brief Set brightness callback
 *
 * Register a callback to handle setBrightness requests.
 * Brightness values are 0-100%.
 *
 * @param device    DimSwitch device
 * @param callback  Callback function
 */
void sinricpro_dimswitch_on_brightness(sinricpro_dimswitch_t *device,
                                       sinricpro_brightness_callback_t callback);

/**
 * @brief Set adjust brightness callback
 *
 * Register a callback to handle adjustBrightness requests.
 * Delta values are -100 to +100, callback should return absolute brightness.
 *
 * @param device    DimSwitch device
 * @param callback  Callback function
 */
void sinricpro_dimswitch_on_adjust_brightness(sinricpro_dimswitch_t *device,
                                              sinricpro_adjust_brightness_callback_t callback);

/**
 * @brief Send power state event
 *
 * Call this when the power state changes due to physical interaction.
 *
 * @param device    DimSwitch device
 * @param state     New power state (true=ON, false=OFF)
 * @return true if event sent successfully
 */
bool sinricpro_dimswitch_send_power_state_event(sinricpro_dimswitch_t *device, bool state);

/**
 * @brief Send brightness event
 *
 * Call this when the brightness changes due to physical interaction.
 *
 * @param device     DimSwitch device
 * @param brightness New brightness value (0-100)
 * @return true if event sent successfully
 */
bool sinricpro_dimswitch_send_brightness_event(sinricpro_dimswitch_t *device, int brightness);

/**
 * @brief Get current power state
 *
 * @param device    DimSwitch device
 * @return Current power state
 */
bool sinricpro_dimswitch_get_power_state(const sinricpro_dimswitch_t *device);

/**
 * @brief Get current brightness
 *
 * @param device    DimSwitch device
 * @return Current brightness (0-100)
 */
int sinricpro_dimswitch_get_brightness(const sinricpro_dimswitch_t *device);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_DIMSWITCH_H
