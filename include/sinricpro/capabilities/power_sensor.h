/**
 * @file power_sensor.h
 * @brief Power sensor capability for SinricPro devices
 */

#ifndef SINRICPRO_POWER_SENSOR_H
#define SINRICPRO_POWER_SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "sinricpro/event_limiter.h"

/**
 * @brief Power sensor capability structure
 */
typedef struct {
    sinricpro_event_limiter_t event_limiter;
    uint32_t start_time;
    float last_power;
} sinricpro_power_sensor_t;

/**
 * @brief Initialize power sensor capability
 */
void sinricpro_power_sensor_init(sinricpro_power_sensor_t *sensor);

/**
 * @brief Send power sensor event to server
 *
 * @param sensor          Power sensor instance
 * @param device_id       Device ID
 * @param voltage         Voltage in volts
 * @param current         Current in amps
 * @param power           Power in watts (-1 for auto-calculate)
 * @param apparent_power  Apparent power in VA (-1 if not available)
 * @param reactive_power  Reactive power in VAR (-1 if not available)
 * @param factor          Power factor (-1 for auto-calculate)
 * @return true if event sent successfully
 */
bool sinricpro_power_sensor_send_event(sinricpro_power_sensor_t *sensor,
                                         const char *device_id,
                                         float voltage,
                                         float current,
                                         float power,
                                         float apparent_power,
                                         float reactive_power,
                                         float factor);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_POWER_SENSOR_H
