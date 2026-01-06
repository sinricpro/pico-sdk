/**
 * @file contact_sensor.c
 * @brief Contact sensor capability implementation
 */

#include "sinricpro/capabilities/contact_sensor.h"
#include "sinricpro/sinricpro.h"
#include "core/json_helpers.h"
#include "core/sinricpro_debug.h"
#include <stdio.h>
#include <string.h>

void sinricpro_contact_sensor_cap_init(sinricpro_contact_sensor_cap_t *cap) {
    if (!cap) return;

    cap->contact_open = false;
    sinricpro_event_limiter_init_state(&cap->event_limiter);
}

bool sinricpro_contact_sensor_cap_send_event(sinricpro_contact_sensor_cap_t *cap,
                                             const char *device_id,
                                             bool is_open) {
    if (!cap || !device_id) {
        return false;
    }

    // Check rate limit
    if (sinricpro_event_limiter_check(&cap->event_limiter)) {
        SINRICPRO_DEBUG_PRINTF("[ContactSensor] Event rate limited\n");
        return false;
    }

    // Create value JSON
    cJSON *value = cJSON_CreateObject();
    if (!value) {
        return false;
    }

    cJSON_AddStringToObject(value, "state", is_open ? "open" : "closed");

    // Send event
    bool result = sinricpro_send_event(device_id, "setContactState", value);

    if (result) {
        cap->contact_open = is_open;
        SINRICPRO_DEBUG_PRINTF("[ContactSensor] Sent event: %s\n",
                               is_open ? "OPEN" : "CLOSED");
    }

    return result;
}

bool sinricpro_contact_sensor_get_state(const sinricpro_contact_sensor_cap_t *cap) {
    return cap ? cap->contact_open : false;
}
