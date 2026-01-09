/**
 * @file air_quality_sensor.c
 * @brief Air quality sensor capability implementation
 */

#include "sinricpro/capabilities/air_quality_sensor.h"
#include "core/sinricpro_debug.h"
#include "cJSON.h"
#include <stdio.h>
#include <string.h>

// External function declaration
extern bool sinricpro_send_event(const char *device_id, const char *action, cJSON *value_json);

void sinricpro_air_quality_sensor_init(sinricpro_air_quality_sensor_t *sensor) {
    if (!sensor) return;

    sinricpro_event_limiter_init_sensor(&sensor->event_limiter);
}

bool sinricpro_air_quality_sensor_send_event(sinricpro_air_quality_sensor_t *sensor,
                                              const char *device_id,
                                              int pm1,
                                              int pm2_5,
                                              int pm10) {
    if (!sensor || !device_id) {
        SINRICPRO_DEBUG_PRINTF("[AirQualitySensor] Invalid parameters\n");
        return false;
    }

    // Check rate limit (60 seconds for sensor readings)
    if (sinricpro_event_limiter_check(&sensor->event_limiter)) {
        SINRICPRO_DEBUG_PRINTF("[AirQualitySensor] Event rate limited\n");
        return false;
    }

    // Create value JSON
    cJSON *value = cJSON_CreateObject();
    if (!value) {
        SINRICPRO_DEBUG_PRINTF("[AirQualitySensor] Failed to create JSON object\n");
        return false;
    }

    // Add fields to value object
    cJSON_AddNumberToObject(value, "pm1", pm1);
    cJSON_AddNumberToObject(value, "pm2_5", pm2_5);
    cJSON_AddNumberToObject(value, "pm10", pm10);

    // Send event
    bool result = sinricpro_send_event(device_id, "airQuality", value);

    if (result) {
        SINRICPRO_DEBUG_PRINTF("[AirQualitySensor] Sent event: PM1=%d, PM2.5=%d, PM10=%d μg/m³\n",
                               pm1, pm2_5, pm10);
    } else {
        SINRICPRO_DEBUG_PRINTF("[AirQualitySensor] Failed to send event\n");
    }

    return result;
}
