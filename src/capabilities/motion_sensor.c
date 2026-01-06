/**
 * @file motion_sensor.c
 * @brief Motion sensor capability implementation
 */

#include "sinricpro/capabilities/motion_sensor.h"
#include "sinricpro/sinricpro.h"
#include "core/json_helpers.h"
#include "core/sinricpro_debug.h"
#include <stdio.h>
#include <string.h>

void sinricpro_motion_sensor_cap_init(sinricpro_motion_sensor_cap_t *cap) {
    if (!cap) return;

    cap->motion_detected = false;
    sinricpro_event_limiter_init_state(&cap->event_limiter);
}

bool sinricpro_motion_sensor_cap_send_event(sinricpro_motion_sensor_cap_t *cap,
                                            const char *device_id,
                                            bool detected) {
    if (!cap || !device_id) {
        return false;
    }

    // Check rate limit
    if (sinricpro_event_limiter_check(&cap->event_limiter)) {
        SINRICPRO_DEBUG_PRINTF("[MotionSensor] Event rate limited\n");
        return false;
    }

    // Create value JSON
    cJSON *value = cJSON_CreateObject();
    if (!value) {
        return false;
    }

    cJSON_AddStringToObject(value, "state", detected ? "detected" : "notDetected");

    // Send event
    bool result = sinricpro_send_event(device_id, "setMotionDetection", value);

    if (result) {
        cap->motion_detected = detected;
        SINRICPRO_DEBUG_PRINTF("[MotionSensor] Sent event: %s\n",
                               detected ? "DETECTED" : "NOT DETECTED");
    }

    return result;
}

bool sinricpro_motion_sensor_get_state(const sinricpro_motion_sensor_cap_t *cap) {
    return cap ? cap->motion_detected : false;
}
