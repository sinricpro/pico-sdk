/**
 * @file sinricpro_fan.h
 * @brief SinricPro Fan device for Raspberry Pi Pico W
 *
 * A Fan device supports on/off control and speed adjustment (0-100%).
 * Use this for ceiling fans, desk fans, or any fan with variable speed.
 *
 * @example
 * @code
 * sinricpro_fan_t my_fan;
 *
 * bool on_power_state(sinricpro_device_t *device, bool *state) {
 *     gpio_put(RELAY_PIN, *state);
 *     return true;
 * }
 *
 * bool on_power_level(sinricpro_device_t *device, int *level) {
 *     // Set fan speed (0-100%)
 *     pwm_set_gpio_level(PWM_PIN, *level * 255 / 100);
 *     return true;
 * }
 * @endcode
 */

#ifndef SINRICPRO_FAN_H
#define SINRICPRO_FAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sinricpro_device.h"
#include "sinricpro/capabilities/power_state.h"
#include "sinricpro/capabilities/power_level.h"

/**
 * @brief Fan device structure
 */
typedef struct {
    sinricpro_device_t base;
    sinricpro_power_state_t power_state;
    sinricpro_power_level_t power_level;
} sinricpro_fan_t;

bool sinricpro_fan_init(sinricpro_fan_t *device, const char *device_id);

void sinricpro_fan_on_power_state(sinricpro_fan_t *device,
                                  sinricpro_power_state_callback_t callback);

void sinricpro_fan_on_power_level(sinricpro_fan_t *device,
                                  sinricpro_power_level_callback_t callback);

void sinricpro_fan_on_adjust_power_level(sinricpro_fan_t *device,
                                         sinricpro_adjust_power_level_callback_t callback);

bool sinricpro_fan_send_power_state_event(sinricpro_fan_t *device, bool state);
bool sinricpro_fan_send_power_level_event(sinricpro_fan_t *device, int power_level);

bool sinricpro_fan_get_power_state(const sinricpro_fan_t *device);
int sinricpro_fan_get_power_level(const sinricpro_fan_t *device);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_FAN_H
