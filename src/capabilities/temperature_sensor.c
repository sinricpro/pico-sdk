/**
 * @file temperature_sensor.c
 * @brief Temperature sensor capability implementation
 */

#include "sinricpro/capabilities/temperature_sensor.h"
#include "sinricpro/sinricpro.h"
#include "core/json_helpers.h"
#include "core/sinricpro_debug.h"
#include <stdio.h>
#include <string.h>

void sinricpro_temperature_sensor_cap_init(sinricpro_temperature_sensor_cap_t *cap) {
    if (!cap) return;

    cap->temperature = 0.0f;
    cap->humidity = 0.0f;
    sinricpro_event_limiter_init_sensor(&cap->event_limiter);  // 60-second limit
}

bool sinricpro_temperature_sensor_cap_send_event(sinricpro_temperature_sensor_cap_t *cap,
                                                 const char *device_id,
                                                 float temperature,
                                                 float humidity) {
    if (!cap || !device_id) {
        return false;
    }

    // Check rate limit (60 seconds for sensor readings)
    if (sinricpro_event_limiter_check(&cap->event_limiter)) {
        SINRICPRO_DEBUG_PRINTF("[TempSensor] Event rate limited\n");
        return false;
    }

    // Create value JSON
    cJSON *value = cJSON_CreateObject();
    if (!value) {
        return false;
    }

    cJSON_AddNumberToObject(value, "temperature", (double)temperature);
    cJSON_AddNumberToObject(value, "humidity", (double)humidity);

    // Send event
    bool result = sinricpro_send_event(device_id, "currentTemperature", value);

    if (result) {
        cap->temperature = temperature;
        cap->humidity = humidity;
        SINRICPRO_DEBUG_PRINTF("[TempSensor] Sent event: %.1f°C, %.1f%% RH\n",
                               temperature, humidity);
    }

    return result;
}

float sinricpro_temperature_sensor_get_temperature(const sinricpro_temperature_sensor_cap_t *cap) {
    return cap ? cap->temperature : 0.0f;
}

float sinricpro_temperature_sensor_get_humidity(const sinricpro_temperature_sensor_cap_t *cap) {
    return cap ? cap->humidity : 0.0f;
}
