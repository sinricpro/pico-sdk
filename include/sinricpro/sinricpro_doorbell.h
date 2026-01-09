/**
 * @file sinricpro_doorbell.h
 * @brief SinricPro Doorbell device for Raspberry Pi Pico W
 *
 * A Doorbell device sends press events when someone rings the doorbell.
 * This is an event-only device (no incoming commands).
 *
 * @example
 * @code
 * sinricpro_doorbell_t my_doorbell;
 *
 * // When button is pressed:
 * sinricpro_doorbell_send_press_event(&my_doorbell);
 * @endcode
 */

#ifndef SINRICPRO_DOORBELL_H
#define SINRICPRO_DOORBELL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sinricpro_device.h"
#include "sinricpro/capabilities/power_state.h"
#include "sinricpro/capabilities/doorbell.h"

/**
 * @brief Doorbell device structure
 */
typedef struct {
    sinricpro_device_t base;
    sinricpro_power_state_t power_state;
    sinricpro_doorbell_cap_t doorbell;
} sinricpro_doorbell_t;

bool sinricpro_doorbell_init(sinricpro_doorbell_t *device, const char *device_id);

void sinricpro_doorbell_on_power_state(sinricpro_doorbell_t *device,
                                        sinricpro_power_state_callback_t callback);

bool sinricpro_doorbell_send_press_event(sinricpro_doorbell_t *device);

bool sinricpro_doorbell_send_power_state_event(sinricpro_doorbell_t *device, bool state);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_DOORBELL_H
