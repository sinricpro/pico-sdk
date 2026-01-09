/**
 * @file sinricpro_airqualitysensor.h
 * @brief SinricPro Air Quality Sensor device for Raspberry Pi Pico W
 *
 * An Air Quality Sensor device monitors particulate matter (PM) levels
 * and reports PM1.0, PM2.5, and PM10 concentrations.
 *
 * @example
 * @code
 * sinricpro_airqualitysensor_t my_sensor;
 *
 * // Send air quality readings
 * sinricpro_airqualitysensor_send_event(&my_sensor, 10, 25, 50);
 * @endcode
 */

#ifndef SINRICPRO_AIRQUALITYSENSOR_H
#define SINRICPRO_AIRQUALITYSENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sinricpro_device.h"
#include "sinricpro/capabilities/air_quality_sensor.h"

/**
 * @brief Air Quality Sensor device structure
 */
typedef struct {
    sinricpro_device_t base;
    sinricpro_air_quality_sensor_t air_quality_sensor;
} sinricpro_airqualitysensor_t;

bool sinricpro_airqualitysensor_init(sinricpro_airqualitysensor_t *device, const char *device_id);

bool sinricpro_airqualitysensor_send_event(sinricpro_airqualitysensor_t *device,
                                             int pm1,
                                             int pm2_5,
                                             int pm10);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_AIRQUALITYSENSOR_H
