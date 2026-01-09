/**
 * @file door_controller.c
 * @brief Door controller capability implementation
 */

#include "sinricpro/capabilities/door_controller.h"
#include "core/sinricpro_debug.h"
#include "core/json_helpers.h"
#include <string.h>
#include <stdio.h>

// Declared in sinricpro.c
extern bool sinricpro_send_event(const char *device_id, cJSON *event_message);

void sinricpro_door_controller_init(sinricpro_door_controller_t *controller) {
    if (!controller) return;

    controller->closed = false;
    controller->callback = NULL;
    sinricpro_event_limiter_init_state(&controller->event_limiter);
}

void sinricpro_door_controller_set_callback(sinricpro_door_controller_t *controller,
                                              sinricpro_door_state_callback_t callback) {
    if (controller) {
        controller->callback = callback;
    }
}

bool sinricpro_door_controller_handle_request(sinricpro_door_controller_t *controller,
                                                sinricpro_device_t *device,
                                                const cJSON *request,
                                                cJSON *response) {
    if (!controller || !device || !request || !response) {
        return false;
    }

    if (!controller->callback) {
        SINRICPRO_WARN_PRINTF("[DoorController] No callback set\n");
        return false;
    }

    // Parse request: { "mode": "Open" } or { "mode": "Close" }
    cJSON *value = cJSON_GetObjectItem(request, "value");
    if (!value) {
        SINRICPRO_WARN_PRINTF("[DoorController] Missing 'value' in request\n");
        return false;
    }

    cJSON *mode_json = cJSON_GetObjectItem(value, "mode");
    if (!mode_json || !cJSON_IsString(mode_json)) {
        SINRICPRO_WARN_PRINTF("[DoorController] Missing or invalid 'mode' in request\n");
        return false;
    }

    const char *mode_str = mode_json->valuestring;
    bool close_requested = (strcmp(mode_str, "Close") == 0);

    SINRICPRO_DEBUG_PRINTF("[DoorController] Request: %s\n",
                            close_requested ? "CLOSE" : "OPEN");

    // Call user callback
    bool door_state = close_requested;
    bool success = controller->callback(device, &door_state);

    // Build response: { "mode": "Open" } or { "mode": "Close" }
    cJSON *response_value = cJSON_GetObjectItem(response, "value");
    if (!response_value) {
        response_value = cJSON_CreateObject();
        cJSON_AddItemToObject(response, "value", response_value);
    }

    controller->closed = door_state;
    cJSON_AddStringToObject(response_value, "mode",
                             door_state ? "Close" : "Open");

    SINRICPRO_DEBUG_PRINTF("[DoorController] Success: %s\n",
                            door_state ? "CLOSED" : "OPEN");

    return success;
}

bool sinricpro_door_controller_send_event(sinricpro_door_controller_t *controller,
                                            const char *device_id,
                                            bool closed) {
    if (!controller || !device_id) {
        return false;
    }

    // Check rate limiting
    if (!sinricpro_event_limiter_check(&controller->event_limiter)) {
        SINRICPRO_WARN_PRINTF("[DoorController] Event rate limited\n");
        return false;
    }

    // Update internal state
    controller->closed = closed;

    // Build event message
    cJSON *event = cJSON_CreateObject();
    if (!event) return false;

    cJSON_AddStringToObject(event, "action", "setMode");
    cJSON_AddStringToObject(event, "cause", "PHYSICAL_INTERACTION");

    cJSON *value = cJSON_CreateObject();
    cJSON_AddStringToObject(value, "mode", closed ? "Close" : "Open");
    cJSON_AddItemToObject(event, "value", value);

    SINRICPRO_DEBUG_PRINTF("[DoorController] Sending event: %s\n",
                            closed ? "CLOSED" : "OPEN");

    bool result = sinricpro_send_event(device_id, event);
    cJSON_Delete(event);

    return result;
}

bool sinricpro_door_controller_is_closed(const sinricpro_door_controller_t *controller) {
    return controller ? controller->closed : false;
}
