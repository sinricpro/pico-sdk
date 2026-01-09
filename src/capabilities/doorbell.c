/**
 * @file doorbell.c
 * @brief Doorbell capability implementation
 */

#include "sinricpro/capabilities/doorbell.h"
#include "core/sinricpro_debug.h"
#include "cJSON.h"
#include <string.h>

// Declared in sinricpro.c
extern bool sinricpro_send_event(const char *device_id, cJSON *event_message);

void sinricpro_doorbell_cap_init(sinricpro_doorbell_cap_t *doorbell) {
    if (!doorbell) return;
    sinricpro_event_limiter_init_state(&doorbell->event_limiter);
}

bool sinricpro_doorbell_cap_send_event(sinricpro_doorbell_cap_t *doorbell,
                                        const char *device_id) {
    if (!doorbell || !device_id) {
        return false;
    }

    // Check rate limiting (sensor state limit - allows more frequent events)
    if (!sinricpro_event_limiter_check(&doorbell->event_limiter)) {
        SINRICPRO_WARN_PRINTF("[Doorbell] Event rate limited\n");
        return false;
    }

    // Build event message
    cJSON *event = cJSON_CreateObject();
    if (!event) return false;

    cJSON_AddStringToObject(event, "action", "DoorbellPress");
    cJSON_AddStringToObject(event, "cause", "PHYSICAL_INTERACTION");

    cJSON *value = cJSON_CreateObject();
    cJSON_AddStringToObject(value, "state", "pressed");
    cJSON_AddItemToObject(event, "value", value);

    SINRICPRO_DEBUG_PRINTF("[Doorbell] Sending doorbell press event\n");

    bool result = sinricpro_send_event(device_id, event);
    cJSON_Delete(event);

    return result;
}
