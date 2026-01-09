/**
 * @file sinricpro_lock.c
 * @brief SinricPro Lock device implementation
 */

#include "sinricpro/sinricpro_lock.h"
#include "sinricpro/capabilities/lock_controller.h"
#include "core/json_helpers.h"
#include "core/sinricpro_debug.h"
#include <stdio.h>
#include <string.h>

static bool lock_handle_request(sinricpro_device_t *device,
                                 const char *action,
                                 const cJSON *request,
                                 cJSON *response);

bool sinricpro_lock_init(sinricpro_lock_t *device, const char *device_id) {
    if (!device || !device_id) return false;

    if (!sinricpro_device_init(&device->base, device_id, SINRICPRO_DEVICE_TYPE_LOCK)) {
        return false;
    }

    device->base.handle_request = lock_handle_request;

    sinricpro_lock_controller_init(&device->lock_controller);

    SINRICPRO_DEBUG_PRINTF("[Lock] Initialized device: %s\n", device_id);
    return true;
}

void sinricpro_lock_on_lock_state(sinricpro_lock_t *device,
                                   sinricpro_lock_state_callback_t callback) {
    if (device) {
        sinricpro_lock_controller_set_callback(&device->lock_controller, callback);
    }
}

bool sinricpro_lock_send_lock_state_event(sinricpro_lock_t *device, bool locked) {
    if (!device) return false;
    return sinricpro_lock_controller_send_event(&device->lock_controller,
                                                 device->base.device_id,
                                                 locked);
}

bool sinricpro_lock_is_locked(const sinricpro_lock_t *device) {
    if (!device) return false;
    return sinricpro_lock_controller_is_locked(&device->lock_controller);
}

static bool lock_handle_request(sinricpro_device_t *device,
                                 const char *action,
                                 const cJSON *request,
                                 cJSON *response) {
    sinricpro_lock_t *lock = (sinricpro_lock_t *)device;

    if (strcmp(action, "setLockState") == 0) {
        return sinricpro_lock_controller_handle_request(&lock->lock_controller,
                                                         device, request, response);
    }

    SINRICPRO_WARN_PRINTF("[Lock] Unknown action: %s\n", action);
    return false;
}
