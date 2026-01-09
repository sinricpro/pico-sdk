/**
 * @file sinricpro_garagedoor.h
 * @brief SinricPro Garage Door device for Raspberry Pi Pico W
 *
 * A Garage Door device supports open/close control for garage doors.
 * Use this for automatic garage door openers.
 *
 * @example
 * @code
 * sinricpro_garagedoor_t my_door;
 *
 * bool on_door_state(sinricpro_device_t *device, bool *state) {
 *     // *state: true = close, false = open
 *     if (*state) {
 *         // Close the door
 *     } else {
 *         // Open the door
 *     }
 *     return true;
 * }
 * @endcode
 */

#ifndef SINRICPRO_GARAGEDOOR_H
#define SINRICPRO_GARAGEDOOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sinricpro_device.h"
#include "sinricpro/capabilities/door_controller.h"

/**
 * @brief Garage Door device structure
 */
typedef struct {
    sinricpro_device_t base;
    sinricpro_door_controller_t door_controller;
} sinricpro_garagedoor_t;

bool sinricpro_garagedoor_init(sinricpro_garagedoor_t *device, const char *device_id);

void sinricpro_garagedoor_on_door_state(sinricpro_garagedoor_t *device,
                                         sinricpro_door_state_callback_t callback);

bool sinricpro_garagedoor_send_door_state_event(sinricpro_garagedoor_t *device, bool closed);

bool sinricpro_garagedoor_is_closed(const sinricpro_garagedoor_t *device);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_GARAGEDOOR_H
