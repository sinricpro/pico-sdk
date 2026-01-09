/**
 * @file range_controller.c
 * @brief Range controller capability implementation
 */

#include "sinricpro/capabilities/range_controller.h"
#include "core/sinricpro_debug.h"
#include "core/json_helpers.h"
#include "cJSON.h"
#include <stdio.h>
#include <string.h>

extern bool sinricpro_send_event(const char *device_id, const char *action, cJSON *value_json);

void sinricpro_range_controller_init(sinricpro_range_controller_t *controller) {
    if (!controller) return;

    controller->range_value = 0;
    controller->set_callback = NULL;
    controller->adjust_callback = NULL;
    sinricpro_event_limiter_init_state(&controller->event_limiter);
}

void sinricpro_range_controller_set_callback(sinricpro_range_controller_t *controller,
                                               sinricpro_range_value_callback_t callback) {
    if (controller) {
        controller->set_callback = callback;
    }
}

void sinricpro_range_controller_set_adjust_callback(sinricpro_range_controller_t *controller,
                                                      sinricpro_adjust_range_callback_t callback) {
    if (controller) {
        controller->adjust_callback = callback;
    }
}

bool sinricpro_range_controller_handle_set_request(sinricpro_range_controller_t *controller,
                                                     sinricpro_device_t *device,
                                                     const cJSON *request,
                                                     cJSON *response) {
    if (!controller || !device || !request || !response) {
        return false;
    }

    // Get value from request
    const cJSON *value = sinricpro_json_get_value(request);
    if (!value) {
        SINRICPRO_ERROR_PRINTF("[RangeController] No value in request\n");
        return false;
    }

    // Get range value from value object
    int range_value = sinricpro_json_get_int(value, "rangeValue", -1);
    if (range_value < 0) {
        SINRICPRO_ERROR_PRINTF("[RangeController] No rangeValue in request\n");
        return false;
    }

    SINRICPRO_DEBUG_PRINTF("[RangeController] setRangeValue: %d\n", range_value);

    // Call user callback
    bool success = true;
    if (controller->set_callback) {
        success = controller->set_callback(device, &range_value);
    }

    // Clamp to 0-100
    if (range_value < 0) range_value = 0;
    if (range_value > 100) range_value = 100;

    if (success) {
        controller->range_value = range_value;
    }

    // Build response value
    cJSON *resp_value = sinricpro_json_add_value(response);
    if (resp_value) {
        cJSON_AddNumberToObject(resp_value, "rangeValue", range_value);
    }

    return success;
}

bool sinricpro_range_controller_handle_adjust_request(sinricpro_range_controller_t *controller,
                                                        sinricpro_device_t *device,
                                                        const cJSON *request,
                                                        cJSON *response) {
    if (!controller || !device || !request || !response) {
        return false;
    }

    // Get value from request
    const cJSON *value = sinricpro_json_get_value(request);
    if (!value) {
        SINRICPRO_ERROR_PRINTF("[RangeController] No value in request\n");
        return false;
    }

    // Get range value delta from value object
    int delta = sinricpro_json_get_int(value, "rangeValueDelta", 0);

    SINRICPRO_DEBUG_PRINTF("[RangeController] adjustRangeValue: delta=%d\n", delta);

    // Calculate new absolute value
    int new_value = controller->range_value + delta;

    // Call user callback
    // NOTE: Callback receives delta, but should return absolute range value
    bool success = true;
    if (controller->adjust_callback) {
        success = controller->adjust_callback(device, &delta);
        // delta now contains the absolute range value
        new_value = delta;
    }

    // Clamp to 0-100
    if (new_value < 0) new_value = 0;
    if (new_value > 100) new_value = 100;

    if (success) {
        controller->range_value = new_value;
    }

    // Build response value with absolute range value (not delta)
    cJSON *resp_value = sinricpro_json_add_value(response);
    if (resp_value) {
        cJSON_AddNumberToObject(resp_value, "rangeValue", new_value);
    }

    return success;
}

bool sinricpro_range_controller_send_event(sinricpro_range_controller_t *controller,
                                             const char *device_id,
                                             int range_value) {
    if (!controller || !device_id) {
        return false;
    }

    // Check rate limit
    if (sinricpro_event_limiter_check(&controller->event_limiter)) {
        SINRICPRO_DEBUG_PRINTF("[RangeController] Event rate limited\n");
        return false;
    }

    // Create value JSON
    cJSON *value = cJSON_CreateObject();
    if (!value) {
        return false;
    }

    cJSON_AddNumberToObject(value, "rangeValue", range_value);

    // Send event
    bool result = sinricpro_send_event(device_id, "setRangeValue", value);

    if (result) {
        controller->range_value = range_value;
        SINRICPRO_DEBUG_PRINTF("[RangeController] Sent event: %d\n", range_value);
    }

    return result;
}

int sinricpro_range_controller_get_value(const sinricpro_range_controller_t *controller) {
    return controller ? controller->range_value : 0;
}
