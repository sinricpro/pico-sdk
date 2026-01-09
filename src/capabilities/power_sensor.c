/**
 * @file power_sensor.c
 * @brief Power sensor capability implementation
 */

#include "sinricpro/capabilities/power_sensor.h"
#include "core/sinricpro_debug.h"
#include "cJSON.h"
#include "pico/time.h"
#include <stdio.h>
#include <string.h>

// External function declaration
extern bool sinricpro_send_event(const char *device_id, const char *action, cJSON *value_json);

void sinricpro_power_sensor_init(sinricpro_power_sensor_t *sensor) {
    if (!sensor) return;

    sinricpro_event_limiter_init_sensor(&sensor->event_limiter);
    sensor->start_time = 0;
    sensor->last_power = 0.0f;
}

bool sinricpro_power_sensor_send_event(sinricpro_power_sensor_t *sensor,
                                        const char *device_id,
                                        float voltage,
                                        float current,
                                        float power,
                                        float apparent_power,
                                        float reactive_power,
                                        float factor) {
    if (!sensor || !device_id) {
        SINRICPRO_DEBUG_PRINTF("[PowerSensor] Invalid parameters\n");
        return false;
    }

    // Check rate limit (60 seconds for sensor readings)
    if (sinricpro_event_limiter_check(&sensor->event_limiter)) {
        SINRICPRO_DEBUG_PRINTF("[PowerSensor] Event rate limited\n");
        return false;
    }

    // Calculate power if not provided
    if (power == -1.0f) {
        power = voltage * current;
    }

    // Calculate power factor if not provided and apparent power is available
    if (factor == -1.0f && apparent_power != -1.0f && apparent_power > 0.0f) {
        factor = power / apparent_power;
    }

    // Get current timestamp in seconds
    uint32_t current_timestamp = to_ms_since_boot(get_absolute_time()) / 1000;

    // Calculate watt hours
    float watt_hours = 0.0f;
    if (sensor->start_time > 0) {
        uint32_t elapsed_seconds = current_timestamp - sensor->start_time;
        watt_hours = (elapsed_seconds * sensor->last_power) / 3600.0f;
    }

    // Create value JSON
    cJSON *value = cJSON_CreateObject();
    if (!value) {
        SINRICPRO_DEBUG_PRINTF("[PowerSensor] Failed to create JSON object\n");
        return false;
    }

    // Add all fields to value object
    cJSON_AddNumberToObject(value, "startTime", (double)current_timestamp);
    cJSON_AddNumberToObject(value, "voltage", (double)voltage);
    cJSON_AddNumberToObject(value, "current", (double)current);
    cJSON_AddNumberToObject(value, "power", (double)power);

    if (apparent_power != -1.0f) {
        cJSON_AddNumberToObject(value, "apparentPower", (double)apparent_power);
    }

    if (reactive_power != -1.0f) {
        cJSON_AddNumberToObject(value, "reactivePower", (double)reactive_power);
    }

    if (factor != -1.0f) {
        cJSON_AddNumberToObject(value, "factor", (double)factor);
    }

    cJSON_AddNumberToObject(value, "wattHours", (double)watt_hours);

    // Send event
    bool result = sinricpro_send_event(device_id, "powerUsage", value);

    if (result) {
        // Update tracking values
        if (sensor->start_time == 0) {
            sensor->start_time = current_timestamp;
        }
        sensor->last_power = power;

        SINRICPRO_DEBUG_PRINTF("[PowerSensor] Sent event: %.2fV, %.2fA, %.2fW, %.2fWh\n",
                               voltage, current, power, watt_hours);
    } else {
        SINRICPRO_DEBUG_PRINTF("[PowerSensor] Failed to send event\n");
    }

    return result;
}
