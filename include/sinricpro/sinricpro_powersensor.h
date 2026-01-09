/**
 * @file sinricpro_powersensor.h
 * @brief SinricPro Power Sensor device for Raspberry Pi Pico W
 *
 * A Power Sensor device monitors electrical power consumption and reports
 * voltage, current, power, and energy usage.
 *
 * @example
 * @code
 * sinricpro_powersensor_t my_sensor;
 *
 * // Send power readings every 60 seconds
 * sinricpro_powersensor_send_power_event(&my_sensor, 230.0f, 0.5f, -1, -1, -1, -1);
 * @endcode
 */

#ifndef SINRICPRO_POWERSENSOR_H
#define SINRICPRO_POWERSENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sinricpro_device.h"
#include "sinricpro/capabilities/power_sensor.h"

/**
 * @brief Power Sensor device structure
 */
typedef struct {
    sinricpro_device_t base;
    sinricpro_power_sensor_t power_sensor;
} sinricpro_powersensor_t;

bool sinricpro_powersensor_init(sinricpro_powersensor_t *device, const char *device_id);

bool sinricpro_powersensor_send_power_event(sinricpro_powersensor_t *device,
                                              float voltage,
                                              float current,
                                              float power,
                                              float apparent_power,
                                              float reactive_power,
                                              float factor);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_POWERSENSOR_H
