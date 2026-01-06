/**
 * @file color.c
 * @brief Color capability implementation
 */

#include "sinricpro/capabilities/color.h"
#include "sinricpro/sinricpro.h"
#include "core/json_helpers.h"
#include "core/sinricpro_debug.h"
#include <stdio.h>
#include <string.h>

void sinricpro_color_init(sinricpro_color_cap_t *cap) {
    if (!cap) return;

    cap->current_color.r = 0;
    cap->current_color.g = 0;
    cap->current_color.b = 0;
    cap->callback = NULL;
    sinricpro_event_limiter_init_state(&cap->event_limiter);
}

void sinricpro_color_set_callback(sinricpro_color_cap_t *cap,
                                  sinricpro_color_callback_t callback) {
    if (cap) {
        cap->callback = callback;
    }
}

bool sinricpro_color_handle_request(sinricpro_color_cap_t *cap,
                                    sinricpro_device_t *device,
                                    const cJSON *request,
                                    cJSON *response) {
    if (!cap || !device || !request || !response) {
        return false;
    }

    // Get value from request
    const cJSON *value = sinricpro_json_get_value(request);
    if (!value) {
        SINRICPRO_ERROR_PRINTF("[Color] No value in request\n");
        return false;
    }

    // Get color object
    const cJSON *color_obj = cJSON_GetObjectItem(value, "color");
    if (!color_obj) {
        SINRICPRO_ERROR_PRINTF("[Color] No color in request\n");
        return false;
    }

    // Extract RGB values
    sinricpro_color_t new_color;
    new_color.r = (uint8_t)sinricpro_json_get_int(color_obj, "r", 0);
    new_color.g = (uint8_t)sinricpro_json_get_int(color_obj, "g", 0);
    new_color.b = (uint8_t)sinricpro_json_get_int(color_obj, "b", 0);

    SINRICPRO_DEBUG_PRINTF("[Color] setColor: RGB(%d, %d, %d)\n",
                           new_color.r, new_color.g, new_color.b);

    // Call user callback
    bool success = true;
    if (cap->callback) {
        success = cap->callback(device, &new_color);
    }

    if (success) {
        cap->current_color = new_color;
    }

    // Build response value
    cJSON *resp_value = sinricpro_json_add_value(response);
    if (resp_value) {
        cJSON *resp_color = cJSON_AddObjectToObject(resp_value, "color");
        if (resp_color) {
            cJSON_AddNumberToObject(resp_color, "r", new_color.r);
            cJSON_AddNumberToObject(resp_color, "g", new_color.g);
            cJSON_AddNumberToObject(resp_color, "b", new_color.b);
        }
    }

    return success;
}

bool sinricpro_color_send_event(sinricpro_color_cap_t *cap,
                                const char *device_id,
                                sinricpro_color_t color) {
    if (!cap || !device_id) {
        return false;
    }

    // Check rate limit
    if (sinricpro_event_limiter_check(&cap->event_limiter)) {
        SINRICPRO_DEBUG_PRINTF("[Color] Event rate limited\n");
        return false;
    }

    // Create value JSON
    cJSON *value = cJSON_CreateObject();
    if (!value) {
        return false;
    }

    cJSON *color_obj = cJSON_AddObjectToObject(value, "color");
    if (!color_obj) {
        cJSON_Delete(value);
        return false;
    }

    cJSON_AddNumberToObject(color_obj, "r", color.r);
    cJSON_AddNumberToObject(color_obj, "g", color.g);
    cJSON_AddNumberToObject(color_obj, "b", color.b);

    // Send event
    bool result = sinricpro_send_event(device_id, "setColor", value);

    if (result) {
        cap->current_color = color;
        SINRICPRO_DEBUG_PRINTF("[Color] Sent event: RGB(%d, %d, %d)\n",
                               color.r, color.g, color.b);
    }

    return result;
}

sinricpro_color_t sinricpro_color_get_value(const sinricpro_color_cap_t *cap) {
    sinricpro_color_t black = {0, 0, 0};
    return cap ? cap->current_color : black;
}
