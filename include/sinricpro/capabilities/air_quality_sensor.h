/**
 * @file air_quality_sensor.h
 * @brief Air quality sensor capability for SinricPro devices
 */

#ifndef SINRICPRO_AIR_QUALITY_SENSOR_H
#define SINRICPRO_AIR_QUALITY_SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "sinricpro/event_limiter.h"

/**
 * @brief Air quality sensor capability structure
 */
typedef struct {
    sinricpro_event_limiter_t event_limiter;
} sinricpro_air_quality_sensor_t;

/**
 * @brief Initialize air quality sensor capability
 */
void sinricpro_air_quality_sensor_init(sinricpro_air_quality_sensor_t *sensor);

/**
 * @brief Send air quality event to server
 *
 * @param sensor    Air quality sensor instance
 * @param device_id Device ID
 * @param pm1       PM1.0 particle concentration (μg/m³)
 * @param pm2_5     PM2.5 particle concentration (μg/m³)
 * @param pm10      PM10 particle concentration (μg/m³)
 * @return true if event sent successfully
 */
bool sinricpro_air_quality_sensor_send_event(sinricpro_air_quality_sensor_t *sensor,
                                               const char *device_id,
                                               int pm1,
                                               int pm2_5,
                                               int pm10);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_AIR_QUALITY_SENSOR_H
