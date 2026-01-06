/**
 * @file power_state.c
 * @brief PowerState capability implementation
 */

#include "sinricpro/capabilities/power_state.h"
#include "sinricpro/sinricpro.h"
#include "core/json_helpers.h"
#include "core/sinricpro_debug.h"
#include <stdio.h>
#include <string.h>

void sinricpro_power_state_init(sinricpro_power_state_t *cap) {
    if (!cap) return;

    cap->callback = NULL;
    cap->current_state = false;
    sinricpro_event_limiter_init_state(&cap->event_limiter);
}

void sinricpro_power_state_set_callback(sinricpro_power_state_t *cap,
                                        sinricpro_power_state_callback_t callback) {
    if (cap) {
        cap->callback = callback;
    }
}

bool sinricpro_power_state_handle_request(sinricpro_power_state_t *cap,
                                          sinricpro_device_t *device,
                                          const cJSON *request,
                                          cJSON *response) {
    if (!cap || !device || !request || !response) {
        return false;
    }

    // Get value from request
    const cJSON *value = sinricpro_json_get_value(request);
    if (!value) {
        SINRICPRO_ERROR_PRINTF("[PowerState] No value in request\n");
        return false;
    }

    // Get state string (SinricPro uses "On"/"Off")
    const char *state_str = sinricpro_json_get_string(value, "state", NULL);
    if (!state_str) {
        SINRICPRO_ERROR_PRINTF("[PowerState] No state in request\n");
        return false;
    }

    // Parse state
    bool new_state = (strcasecmp(state_str, "On") == 0);

    SINRICPRO_DEBUG_PRINTF("[PowerState] setPowerState: %s\n", new_state ? "ON" : "OFF");

    // Call user callback
    bool success = true;
    if (cap->callback) {
        success = cap->callback(device, &new_state);
    }

    if (success) {
        cap->current_state = new_state;
    }

    // Build response value
    cJSON *resp_value = sinricpro_json_add_value(response);
    if (resp_value) {
        cJSON_AddStringToObject(resp_value, "state", new_state ? "On" : "Off");
    }

    return success;
}

bool sinricpro_power_state_send_event(sinricpro_power_state_t *cap,
                                      const char *device_id,
                                      bool state) {
    if (!cap || !device_id) {
        return false;
    }

    // Check rate limit
    if (sinricpro_event_limiter_check(&cap->event_limiter)) {
        SINRICPRO_DEBUG_PRINTF("[PowerState] Event rate limited\n");
        return false;
    }

    // Create value JSON
    cJSON *value = cJSON_CreateObject();
    if (!value) {
        return false;
    }

    cJSON_AddStringToObject(value, "state", state ? "On" : "Off");

    // Send event
    bool result = sinricpro_send_event(device_id, "setPowerState", value);

    if (result) {
        cap->current_state = state;
        SINRICPRO_DEBUG_PRINTF("[PowerState] Sent event: %s\n", state ? "ON" : "OFF");
    }

    return result;
}

bool sinricpro_power_state_get_state(const sinricpro_power_state_t *cap) {
    return cap ? cap->current_state : false;
}
