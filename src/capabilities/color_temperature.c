/**
 * @file color_temperature.c
 * @brief Color temperature capability implementation
 */

#include "sinricpro/capabilities/color_temperature.h"
#include "sinricpro/sinricpro.h"
#include "core/json_helpers.h"
#include "core/sinricpro_debug.h"
#include <stdio.h>
#include <string.h>

void sinricpro_color_temp_init(sinricpro_color_temp_cap_t *cap) {
    if (!cap) return;

    cap->current_temp = 2700;  // Default: soft white
    cap->callback = NULL;
    cap->increase_callback = NULL;
    cap->decrease_callback = NULL;
    sinricpro_event_limiter_init_state(&cap->event_limiter);
}

void sinricpro_color_temp_set_callback(sinricpro_color_temp_cap_t *cap,
                                       sinricpro_color_temp_callback_t callback) {
    if (cap) {
        cap->callback = callback;
    }
}

void sinricpro_color_temp_set_increase_callback(sinricpro_color_temp_cap_t *cap,
                                                sinricpro_color_temp_adjust_callback_t callback) {
    if (cap) {
        cap->increase_callback = callback;
    }
}

void sinricpro_color_temp_set_decrease_callback(sinricpro_color_temp_cap_t *cap,
                                                sinricpro_color_temp_adjust_callback_t callback) {
    if (cap) {
        cap->decrease_callback = callback;
    }
}

bool sinricpro_color_temp_handle_request(sinricpro_color_temp_cap_t *cap,
                                         sinricpro_device_t *device,
                                         const char *action,
                                         const cJSON *request,
                                         cJSON *response) {
    if (!cap || !device || !action || !request || !response) {
        return false;
    }

    const cJSON *value = sinricpro_json_get_value(request);
    if (!value) {
        SINRICPRO_ERROR_PRINTF("[ColorTemp] No value in request\n");
        return false;
    }

    bool success = false;

    // Handle setColorTemperature
    if (strcmp(action, "setColorTemperature") == 0) {
        int color_temp = sinricpro_json_get_int(value, "colorTemperature", -1);
        if (color_temp < 0) {
            SINRICPRO_ERROR_PRINTF("[ColorTemp] No colorTemperature in request\n");
            return false;
        }

        SINRICPRO_DEBUG_PRINTF("[ColorTemp] setColorTemperature: %dK\n", color_temp);

        if (cap->callback) {
            success = cap->callback(device, &color_temp);
        }

        if (success) {
            cap->current_temp = color_temp;
        }

        // Build response
        cJSON *resp_value = sinricpro_json_add_value(response);
        if (resp_value) {
            cJSON_AddNumberToObject(resp_value, "colorTemperature", color_temp);
        }
    }
    // Handle increaseColorTemperature
    else if (strcmp(action, "increaseColorTemperature") == 0) {
        int delta = 1;  // Increase indicator

        SINRICPRO_DEBUG_PRINTF("[ColorTemp] increaseColorTemperature\n");

        if (cap->increase_callback) {
            success = cap->increase_callback(device, &delta);
            // delta now contains the absolute temperature
        }

        if (success) {
            cap->current_temp = delta;
        }

        // Build response
        cJSON *resp_value = sinricpro_json_add_value(response);
        if (resp_value) {
            cJSON_AddNumberToObject(resp_value, "colorTemperature", delta);
        }
    }
    // Handle decreaseColorTemperature
    else if (strcmp(action, "decreaseColorTemperature") == 0) {
        int delta = -1;  // Decrease indicator

        SINRICPRO_DEBUG_PRINTF("[ColorTemp] decreaseColorTemperature\n");

        if (cap->decrease_callback) {
            success = cap->decrease_callback(device, &delta);
            // delta now contains the absolute temperature
        }

        if (success) {
            cap->current_temp = delta;
        }

        // Build response
        cJSON *resp_value = sinricpro_json_add_value(response);
        if (resp_value) {
            cJSON_AddNumberToObject(resp_value, "colorTemperature", delta);
        }
    }

    return success;
}

bool sinricpro_color_temp_send_event(sinricpro_color_temp_cap_t *cap,
                                     const char *device_id,
                                     int color_temp) {
    if (!cap || !device_id) {
        return false;
    }

    // Check rate limit
    if (sinricpro_event_limiter_check(&cap->event_limiter)) {
        SINRICPRO_DEBUG_PRINTF("[ColorTemp] Event rate limited\n");
        return false;
    }

    // Create value JSON
    cJSON *value = cJSON_CreateObject();
    if (!value) {
        return false;
    }

    cJSON_AddNumberToObject(value, "colorTemperature", color_temp);

    // Send event
    bool result = sinricpro_send_event(device_id, "setColorTemperature", value);

    if (result) {
        cap->current_temp = color_temp;
        SINRICPRO_DEBUG_PRINTF("[ColorTemp] Sent event: %dK\n", color_temp);
    }

    return result;
}

int sinricpro_color_temp_get_value(const sinricpro_color_temp_cap_t *cap) {
    return cap ? cap->current_temp : 2700;
}
