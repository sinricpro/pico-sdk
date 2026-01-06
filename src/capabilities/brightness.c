/**
 * @file brightness.c
 * @brief Brightness capability implementation
 */

#include "sinricpro/capabilities/brightness.h"
#include "sinricpro/sinricpro.h"
#include "core/json_helpers.h"
#include "core/sinricpro_debug.h"
#include <stdio.h>
#include <string.h>

// Clamp value to 0-100 range
static int clamp_brightness(int value) {
    if (value < 0) return 0;
    if (value > 100) return 100;
    return value;
}

void sinricpro_brightness_init(sinricpro_brightness_t *cap) {
    if (!cap) return;

    cap->brightness_callback = NULL;
    cap->adjust_brightness_callback = NULL;
    cap->current_brightness = 0;
    sinricpro_event_limiter_init_state(&cap->event_limiter);
}

void sinricpro_brightness_set_callback(sinricpro_brightness_t *cap,
                                       sinricpro_brightness_callback_t callback) {
    if (cap) {
        cap->brightness_callback = callback;
    }
}

void sinricpro_brightness_set_adjust_callback(sinricpro_brightness_t *cap,
                                              sinricpro_adjust_brightness_callback_t callback) {
    if (cap) {
        cap->adjust_brightness_callback = callback;
    }
}

bool sinricpro_brightness_handle_set_request(sinricpro_brightness_t *cap,
                                             sinricpro_device_t *device,
                                             const cJSON *request,
                                             cJSON *response) {
    if (!cap || !device || !request || !response) {
        return false;
    }

    // Get value from request
    const cJSON *value = sinricpro_json_get_value(request);
    if (!value) {
        SINRICPRO_ERROR_PRINTF("[Brightness] No value in request\n");
        return false;
    }

    // Get brightness value
    int brightness = sinricpro_json_get_int(value, "brightness", -1);
    if (brightness < 0) {
        SINRICPRO_ERROR_PRINTF("[Brightness] No brightness in request\n");
        return false;
    }

    brightness = clamp_brightness(brightness);
    SINRICPRO_DEBUG_PRINTF("[Brightness] setBrightness: %d%%\n", brightness);

    // Call user callback
    bool success = true;
    if (cap->brightness_callback) {
        success = cap->brightness_callback(device, &brightness);
        brightness = clamp_brightness(brightness);
    }

    if (success) {
        cap->current_brightness = brightness;
    }

    // Build response value
    cJSON *resp_value = sinricpro_json_add_value(response);
    if (resp_value) {
        cJSON_AddNumberToObject(resp_value, "brightness", brightness);
    }

    return success;
}

bool sinricpro_brightness_handle_adjust_request(sinricpro_brightness_t *cap,
                                                sinricpro_device_t *device,
                                                const cJSON *request,
                                                cJSON *response) {
    if (!cap || !device || !request || !response) {
        return false;
    }

    // Get value from request
    const cJSON *value = sinricpro_json_get_value(request);
    if (!value) {
        SINRICPRO_ERROR_PRINTF("[Brightness] No value in request\n");
        return false;
    }

    // Get brightness delta
    int brightness_delta = sinricpro_json_get_int(value, "brightnessDelta", 0);
    SINRICPRO_DEBUG_PRINTF("[Brightness] adjustBrightness: %+d%%\n", brightness_delta);

    // Call user callback or apply delta ourselves
    bool success = true;
    int new_brightness;

    if (cap->adjust_brightness_callback) {
        // User handles the adjustment
        new_brightness = brightness_delta;  // Pass delta, callback returns absolute
        success = cap->adjust_brightness_callback(device, &new_brightness);
    } else {
        // Apply delta to current brightness
        new_brightness = cap->current_brightness + brightness_delta;
    }

    new_brightness = clamp_brightness(new_brightness);

    if (success) {
        cap->current_brightness = new_brightness;
    }

    // Build response value (return absolute brightness)
    cJSON *resp_value = sinricpro_json_add_value(response);
    if (resp_value) {
        cJSON_AddNumberToObject(resp_value, "brightness", new_brightness);
    }

    return success;
}

bool sinricpro_brightness_send_event(sinricpro_brightness_t *cap,
                                     const char *device_id,
                                     int brightness) {
    if (!cap || !device_id) {
        return false;
    }

    // Check rate limit
    if (sinricpro_event_limiter_check(&cap->event_limiter)) {
        SINRICPRO_DEBUG_PRINTF("[Brightness] Event rate limited\n");
        return false;
    }

    brightness = clamp_brightness(brightness);

    // Create value JSON
    cJSON *value = cJSON_CreateObject();
    if (!value) {
        return false;
    }

    cJSON_AddNumberToObject(value, "brightness", brightness);

    // Send event
    bool result = sinricpro_send_event(device_id, "setBrightness", value);

    if (result) {
        cap->current_brightness = brightness;
        SINRICPRO_DEBUG_PRINTF("[Brightness] Sent event: %d%%\n", brightness);
    }

    return result;
}

int sinricpro_brightness_get_value(const sinricpro_brightness_t *cap) {
    return cap ? cap->current_brightness : 0;
}
