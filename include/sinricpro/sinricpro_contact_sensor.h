/**
 * @file sinricpro_contact_sensor.h
 * @brief SinricPro Contact Sensor device
 *
 * Contact sensor for doors, windows, etc. (open/closed detection).
 */

#ifndef SINRICPRO_CONTACT_SENSOR_H
#define SINRICPRO_CONTACT_SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sinricpro_device.h"
#include "capabilities/contact_sensor.h"

/**
 * @brief Contact sensor device structure
 */
typedef struct {
    sinricpro_device_t base;
    sinricpro_contact_sensor_cap_t contact;
} sinricpro_contact_sensor_t;

/**
 * @brief Initialize contact sensor device
 *
 * @param device Contact sensor device structure
 * @param device_id Device ID from SinricPro portal (24-char hex)
 * @return true on success, false on failure
 */
bool sinricpro_contact_sensor_init(sinricpro_contact_sensor_t *device,
                                   const char *device_id);

/**
 * @brief Send contact state event
 *
 * @param device Contact sensor device
 * @param is_open true if contact is open, false if closed
 * @return true on success, false on failure
 */
bool sinricpro_contact_sensor_send_event(sinricpro_contact_sensor_t *device,
                                         bool is_open);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_CONTACT_SENSOR_H
