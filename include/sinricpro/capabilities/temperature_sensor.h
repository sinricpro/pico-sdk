/**
 * @file temperature_sensor.h
 * @brief Temperature sensor capability for SinricPro
 *
 * Provides temperature and humidity reporting.
 */

#ifndef SINRICPRO_CAPABILITY_TEMPERATURE_SENSOR_H
#define SINRICPRO_CAPABILITY_TEMPERATURE_SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "sinricpro/sinricpro_device.h"
#include "sinricpro/event_limiter.h"

/**
 * @brief Temperature sensor capability structure
 */
typedef struct {
    float temperature;
    float humidity;
    sinricpro_event_limiter_t event_limiter;
} sinricpro_temperature_sensor_cap_t;

/**
 * @brief Initialize temperature sensor capability
 *
 * @param cap Capability structure
 */
void sinricpro_temperature_sensor_cap_init(sinricpro_temperature_sensor_cap_t *cap);

/**
 * @brief Send temperature and humidity event
 *
 * Rate limited to 1 event per 60 seconds (sensor reading limit).
 *
 * @param cap Capability structure
 * @param device_id Device ID (24-char hex string)
 * @param temperature Temperature in Celsius
 * @param humidity Relative humidity (0-100%)
 * @return true on success, false on failure
 */
bool sinricpro_temperature_sensor_cap_send_event(sinricpro_temperature_sensor_cap_t *cap,
                                                 const char *device_id,
                                                 float temperature,
                                                 float humidity);

/**
 * @brief Get current temperature reading
 *
 * @param cap Capability structure
 * @return Temperature in Celsius
 */
float sinricpro_temperature_sensor_get_temperature(const sinricpro_temperature_sensor_cap_t *cap);

/**
 * @brief Get current humidity reading
 *
 * @param cap Capability structure
 * @return Relative humidity (0-100%)
 */
float sinricpro_temperature_sensor_get_humidity(const sinricpro_temperature_sensor_cap_t *cap);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_CAPABILITY_TEMPERATURE_SENSOR_H
