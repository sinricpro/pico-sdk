/**
 * @file sinricpro_temperature_sensor.h
 * @brief SinricPro Temperature Sensor device
 *
 * Temperature and humidity sensor for environmental monitoring.
 */

#ifndef SINRICPRO_TEMPERATURE_SENSOR_H
#define SINRICPRO_TEMPERATURE_SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sinricpro_device.h"
#include "capabilities/temperature_sensor.h"

/**
 * @brief Temperature sensor device structure
 */
typedef struct {
    sinricpro_device_t base;
    sinricpro_temperature_sensor_cap_t temp_humidity;
} sinricpro_temperature_sensor_t;

/**
 * @brief Initialize temperature sensor device
 *
 * @param device Temperature sensor device structure
 * @param device_id Device ID from SinricPro portal (24-char hex)
 * @return true on success, false on failure
 */
bool sinricpro_temperature_sensor_init(sinricpro_temperature_sensor_t *device,
                                       const char *device_id);

/**
 * @brief Send temperature and humidity event
 *
 * Rate limited to 1 event per 60 seconds.
 *
 * @param device Temperature sensor device
 * @param temperature Temperature in Celsius
 * @param humidity Relative humidity (0-100%)
 * @return true on success, false on failure
 */
bool sinricpro_temperature_sensor_send_event(sinricpro_temperature_sensor_t *device,
                                             float temperature,
                                             float humidity);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_TEMPERATURE_SENSOR_H
