/**
 * @file sinricpro_blinds.h
 * @brief SinricPro Blinds device for Raspberry Pi Pico W
 *
 * A Blinds device supports position control (0-100%) for motorized blinds,
 * shades, or curtains. Use this for window treatments with position control.
 *
 * @example
 * @code
 * sinricpro_blinds_t my_blinds;
 *
 * bool on_range_value(sinricpro_device_t *device, int *position) {
 *     // Set blinds position (0=fully open, 100=fully closed)
 *     set_motor_position(*position);
 *     return true;
 * }
 *
 * bool on_adjust_range(sinricpro_device_t *device, int *delta) {
 *     // Adjust position relatively
 *     int new_pos = get_current_position() + *delta;
 *     *delta = new_pos;  // Return absolute position
 *     set_motor_position(new_pos);
 *     return true;
 * }
 * @endcode
 */

#ifndef SINRICPRO_BLINDS_H
#define SINRICPRO_BLINDS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sinricpro_device.h"
#include "sinricpro/capabilities/power_state.h"
#include "sinricpro/capabilities/range_controller.h"

/**
 * @brief Blinds device structure
 */
typedef struct {
    sinricpro_device_t base;
    sinricpro_power_state_t power_state;
    sinricpro_range_controller_t range_controller;
} sinricpro_blinds_t;

bool sinricpro_blinds_init(sinricpro_blinds_t *device, const char *device_id);

void sinricpro_blinds_on_power_state(sinricpro_blinds_t *device,
                                      sinricpro_power_state_callback_t callback);

void sinricpro_blinds_on_range_value(sinricpro_blinds_t *device,
                                      sinricpro_range_value_callback_t callback);

void sinricpro_blinds_on_adjust_range(sinricpro_blinds_t *device,
                                       sinricpro_adjust_range_callback_t callback);

bool sinricpro_blinds_send_power_state_event(sinricpro_blinds_t *device, bool state);
bool sinricpro_blinds_send_range_value_event(sinricpro_blinds_t *device, int position);

bool sinricpro_blinds_get_power_state(const sinricpro_blinds_t *device);
int sinricpro_blinds_get_position(const sinricpro_blinds_t *device);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_BLINDS_H
