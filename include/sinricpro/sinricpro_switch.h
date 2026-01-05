/**
 * @file sinricpro_switch.h
 * @brief SinricPro Switch device for Raspberry Pi Pico W
 *
 * A Switch device supports on/off control via the PowerState capability.
 * Use this for relays, smart plugs, or any binary on/off device.
 *
 * @example
 * @code
 * sinricpro_switch_t my_switch;
 *
 * bool on_power_state(sinricpro_device_t *device, bool *state) {
 *     gpio_put(RELAY_PIN, *state);
 *     return true;
 * }
 *
 * int main() {
 *     sinricpro_switch_init(&my_switch, "device-id-here");
 *     sinricpro_switch_on_power_state(&my_switch, on_power_state);
 *     sinricpro_add_device((sinricpro_device_t *)&my_switch);
 *     // ...
 * }
 * @endcode
 */

#ifndef SINRICPRO_SWITCH_H
#define SINRICPRO_SWITCH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sinricpro_device.h"
#include "sinricpro/capabilities/power_state.h"

/**
 * @brief Switch device structure
 *
 * Contains base device plus PowerState capability.
 */
typedef struct {
    sinricpro_device_t base;           // Must be first member
    sinricpro_power_state_t power_state;
} sinricpro_switch_t;

/**
 * @brief Initialize a Switch device
 *
 * @param device    Switch device to initialize
 * @param device_id Device ID from SinricPro portal (24-char hex)
 * @return true on success, false on failure
 */
bool sinricpro_switch_init(sinricpro_switch_t *device, const char *device_id);

/**
 * @brief Set power state callback
 *
 * Register a callback to handle setPowerState requests from Alexa/Google/App.
 *
 * @param device    Switch device
 * @param callback  Callback function
 */
void sinricpro_switch_on_power_state(sinricpro_switch_t *device,
                                     sinricpro_power_state_callback_t callback);

/**
 * @brief Send power state event
 *
 * Call this when the switch state changes due to physical button press
 * or other local trigger. This notifies the cloud of the new state.
 *
 * @param device    Switch device
 * @param state     New power state (true=ON, false=OFF)
 * @return true if event sent successfully
 */
bool sinricpro_switch_send_power_state_event(sinricpro_switch_t *device, bool state);

/**
 * @brief Get current power state
 *
 * @param device    Switch device
 * @return Current power state
 */
bool sinricpro_switch_get_power_state(const sinricpro_switch_t *device);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_SWITCH_H
