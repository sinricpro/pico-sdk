/**
 * @file lock_controller.c
 * @brief Lock controller capability implementation
 */

#include "sinricpro/capabilities/lock_controller.h"
#include "core/sinricpro_debug.h"
#include "core/json_helpers.h"
#include <string.h>
#include <stdio.h>

// Declared in sinricpro.c
extern bool sinricpro_send_event(const char *device_id, cJSON *event_message);

void sinricpro_lock_controller_init(sinricpro_lock_controller_t *controller) {
    if (!controller) return;

    controller->locked = false;
    controller->callback = NULL;
    sinricpro_event_limiter_init_state(&controller->event_limiter);
}

void sinricpro_lock_controller_set_callback(sinricpro_lock_controller_t *controller,
                                             sinricpro_lock_state_callback_t callback) {
    if (controller) {
        controller->callback = callback;
    }
}

bool sinricpro_lock_controller_handle_request(sinricpro_lock_controller_t *controller,
                                                sinricpro_device_t *device,
                                                const cJSON *request,
                                                cJSON *response) {
    if (!controller || !device || !request || !response) {
        return false;
    }

    if (!controller->callback) {
        SINRICPRO_WARN_PRINTF("[LockController] No callback set\n");
        return false;
    }

    // Parse request: { "state": "lock" } or { "state": "unlock" }
    cJSON *value = cJSON_GetObjectItem(request, "value");
    if (!value) {
        SINRICPRO_WARN_PRINTF("[LockController] Missing 'value' in request\n");
        return false;
    }

    cJSON *state_json = cJSON_GetObjectItem(value, "state");
    if (!state_json || !cJSON_IsString(state_json)) {
        SINRICPRO_WARN_PRINTF("[LockController] Missing or invalid 'state' in request\n");
        return false;
    }

    const char *state_str = state_json->valuestring;
    bool lock_requested = (strcmp(state_str, "lock") == 0);

    SINRICPRO_DEBUG_PRINTF("[LockController] Request: %s\n",
                            lock_requested ? "LOCK" : "UNLOCK");

    // Call user callback
    bool lock_state = lock_requested;
    bool success = controller->callback(device, &lock_state);

    // Build response: { "state": "LOCKED" } or { "state": "UNLOCKED" } or { "state": "JAMMED" }
    cJSON *response_value = cJSON_GetObjectItem(response, "value");
    if (!response_value) {
        response_value = cJSON_CreateObject();
        cJSON_AddItemToObject(response, "value", response_value);
    }

    if (success) {
        controller->locked = lock_state;
        cJSON_AddStringToObject(response_value, "state",
                                 lock_state ? "LOCKED" : "UNLOCKED");
        SINRICPRO_DEBUG_PRINTF("[LockController] Success: %s\n",
                                lock_state ? "LOCKED" : "UNLOCKED");
    } else {
        cJSON_AddStringToObject(response_value, "state", "JAMMED");
        SINRICPRO_WARN_PRINTF("[LockController] Failed: JAMMED\n");
    }

    return success;
}

bool sinricpro_lock_controller_send_event(sinricpro_lock_controller_t *controller,
                                           const char *device_id,
                                           bool locked) {
    if (!controller || !device_id) {
        return false;
    }

    // Check rate limiting
    if (!sinricpro_event_limiter_check(&controller->event_limiter)) {
        SINRICPRO_WARN_PRINTF("[LockController] Event rate limited\n");
        return false;
    }

    // Update internal state
    controller->locked = locked;

    // Build event message
    cJSON *event = cJSON_CreateObject();
    if (!event) return false;

    cJSON_AddStringToObject(event, "action", "setLockState");
    cJSON_AddStringToObject(event, "cause", "PHYSICAL_INTERACTION");

    cJSON *value = cJSON_CreateObject();
    cJSON_AddStringToObject(value, "state", locked ? "LOCKED" : "UNLOCKED");
    cJSON_AddItemToObject(event, "value", value);

    SINRICPRO_DEBUG_PRINTF("[LockController] Sending event: %s\n",
                            locked ? "LOCKED" : "UNLOCKED");

    bool result = sinricpro_send_event(device_id, event);
    cJSON_Delete(event);

    return result;
}

bool sinricpro_lock_controller_is_locked(const sinricpro_lock_controller_t *controller) {
    return controller ? controller->locked : false;
}
