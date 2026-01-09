/**
 * @file sinricpro_garagedoor.c
 * @brief SinricPro Garage Door device implementation
 */

#include "sinricpro/sinricpro_garagedoor.h"
#include "sinricpro/capabilities/door_controller.h"
#include "core/json_helpers.h"
#include "core/sinricpro_debug.h"
#include <stdio.h>
#include <string.h>

static bool garagedoor_handle_request(sinricpro_device_t *device,
                                        const char *action,
                                        const cJSON *request,
                                        cJSON *response);

bool sinricpro_garagedoor_init(sinricpro_garagedoor_t *device, const char *device_id) {
    if (!device || !device_id) return false;

    if (!sinricpro_device_init(&device->base, device_id, SINRICPRO_DEVICE_TYPE_GARAGE_DOOR)) {
        return false;
    }

    device->base.handle_request = garagedoor_handle_request;

    sinricpro_door_controller_init(&device->door_controller);

    SINRICPRO_DEBUG_PRINTF("[GarageDoor] Initialized device: %s\n", device_id);
    return true;
}

void sinricpro_garagedoor_on_door_state(sinricpro_garagedoor_t *device,
                                         sinricpro_door_state_callback_t callback) {
    if (device) {
        sinricpro_door_controller_set_callback(&device->door_controller, callback);
    }
}

bool sinricpro_garagedoor_send_door_state_event(sinricpro_garagedoor_t *device, bool closed) {
    if (!device) return false;
    return sinricpro_door_controller_send_event(&device->door_controller,
                                                 device->base.device_id,
                                                 closed);
}

bool sinricpro_garagedoor_is_closed(const sinricpro_garagedoor_t *device) {
    if (!device) return false;
    return sinricpro_door_controller_is_closed(&device->door_controller);
}

static bool garagedoor_handle_request(sinricpro_device_t *device,
                                        const char *action,
                                        const cJSON *request,
                                        cJSON *response) {
    sinricpro_garagedoor_t *door = (sinricpro_garagedoor_t *)device;

    if (strcmp(action, "setMode") == 0) {
        return sinricpro_door_controller_handle_request(&door->door_controller,
                                                         device, request, response);
    }

    SINRICPRO_WARN_PRINTF("[GarageDoor] Unknown action: %s\n", action);
    return false;
}
