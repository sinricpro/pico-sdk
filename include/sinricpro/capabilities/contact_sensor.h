/**
 * @file contact_sensor.h
 * @brief Contact sensor capability for SinricPro
 *
 * Provides contact state events (open/closed) for doors, windows, etc.
 */

#ifndef SINRICPRO_CAPABILITY_CONTACT_SENSOR_H
#define SINRICPRO_CAPABILITY_CONTACT_SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "sinricpro/sinricpro_device.h"
#include "sinricpro/event_limiter.h"

/**
 * @brief Contact sensor capability structure
 */
typedef struct {
    bool contact_open;
    sinricpro_event_limiter_t event_limiter;
} sinricpro_contact_sensor_cap_t;

/**
 * @brief Initialize contact sensor capability
 *
 * @param cap Capability structure
 */
void sinricpro_contact_sensor_cap_init(sinricpro_contact_sensor_cap_t *cap);

/**
 * @brief Send contact state event
 *
 * @param cap Capability structure
 * @param device_id Device ID (24-char hex string)
 * @param is_open true if contact is open, false if closed
 * @return true on success, false on failure
 */
bool sinricpro_contact_sensor_cap_send_event(sinricpro_contact_sensor_cap_t *cap,
                                             const char *device_id,
                                             bool is_open);

/**
 * @brief Get current contact state
 *
 * @param cap Capability structure
 * @return true if contact is open, false if closed
 */
bool sinricpro_contact_sensor_get_state(const sinricpro_contact_sensor_cap_t *cap);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_CAPABILITY_CONTACT_SENSOR_H
