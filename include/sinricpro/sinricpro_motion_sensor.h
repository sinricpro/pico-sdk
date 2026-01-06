/**
 * @file sinricpro_motion_sensor.h
 * @brief SinricPro Motion Sensor device
 *
 * Motion sensor for detecting movement.
 */

#ifndef SINRICPRO_MOTION_SENSOR_H
#define SINRICPRO_MOTION_SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sinricpro_device.h"
#include "capabilities/motion_sensor.h"

/**
 * @brief Motion sensor device structure
 */
typedef struct {
    sinricpro_device_t base;
    sinricpro_motion_sensor_cap_t motion;
} sinricpro_motion_sensor_t;

/**
 * @brief Initialize motion sensor device
 *
 * @param device Motion sensor device structure
 * @param device_id Device ID from SinricPro portal (24-char hex)
 * @return true on success, false on failure
 */
bool sinricpro_motion_sensor_init(sinricpro_motion_sensor_t *device,
                                  const char *device_id);

/**
 * @brief Send motion detection event
 *
 * @param device Motion sensor device
 * @param detected true if motion detected, false if no motion
 * @return true on success, false on failure
 */
bool sinricpro_motion_sensor_send_event(sinricpro_motion_sensor_t *device,
                                        bool detected);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_MOTION_SENSOR_H
