/**
 * @file motion_sensor.h
 * @brief Motion sensor capability for SinricPro
 *
 * Provides motion detection events (detected/not detected).
 */

#ifndef SINRICPRO_CAPABILITY_MOTION_SENSOR_H
#define SINRICPRO_CAPABILITY_MOTION_SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "sinricpro/sinricpro_device.h"
#include "sinricpro/event_limiter.h"

/**
 * @brief Motion sensor capability structure
 */
typedef struct {
    bool motion_detected;
    sinricpro_event_limiter_t event_limiter;
} sinricpro_motion_sensor_cap_t;

/**
 * @brief Initialize motion sensor capability
 *
 * @param cap Capability structure
 */
void sinricpro_motion_sensor_cap_init(sinricpro_motion_sensor_cap_t *cap);

/**
 * @brief Send motion detection event
 *
 * @param cap Capability structure
 * @param device_id Device ID (24-char hex string)
 * @param detected true if motion detected, false otherwise
 * @return true on success, false on failure
 */
bool sinricpro_motion_sensor_cap_send_event(sinricpro_motion_sensor_cap_t *cap,
                                            const char *device_id,
                                            bool detected);

/**
 * @brief Get current motion state
 *
 * @param cap Capability structure
 * @return true if motion detected, false otherwise
 */
bool sinricpro_motion_sensor_get_state(const sinricpro_motion_sensor_cap_t *cap);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_CAPABILITY_MOTION_SENSOR_H
