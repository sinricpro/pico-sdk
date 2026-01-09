/**
 * @file sinricpro_doorbell.c
 * @brief SinricPro Doorbell device implementation
 */

#include "sinricpro/sinricpro_doorbell.h"
#include "sinricpro/capabilities/power_state.h"
#include "sinricpro/capabilities/doorbell.h"
#include "core/json_helpers.h"
#include "core/sinricpro_debug.h"
#include <stdio.h>
#include <string.h>

static bool doorbell_handle_request(sinricpro_device_t *device,
                                      const char *action,
                                      const cJSON *request,
                                      cJSON *response);

bool sinricpro_doorbell_init(sinricpro_doorbell_t *device, const char *device_id) {
    if (!device || !device_id) return false;

    if (!sinricpro_device_init(&device->base, device_id, SINRICPRO_DEVICE_TYPE_DOORBELL)) {
        return false;
    }

    device->base.handle_request = doorbell_handle_request;

    sinricpro_power_state_init(&device->power_state);
    sinricpro_doorbell_cap_init(&device->doorbell);

    SINRICPRO_DEBUG_PRINTF("[Doorbell] Initialized device: %s\n", device_id);
    return true;
}

void sinricpro_doorbell_on_power_state(sinricpro_doorbell_t *device,
                                        sinricpro_power_state_callback_t callback) {
    if (device) {
        sinricpro_power_state_set_callback(&device->power_state, callback);
    }
}

bool sinricpro_doorbell_send_press_event(sinricpro_doorbell_t *device) {
    if (!device) return false;
    return sinricpro_doorbell_cap_send_event(&device->doorbell,
                                              device->base.device_id);
}

bool sinricpro_doorbell_send_power_state_event(sinricpro_doorbell_t *device, bool state) {
    if (!device) return false;
    return sinricpro_power_state_send_event(&device->power_state,
                                             device->base.device_id,
                                             state);
}

static bool doorbell_handle_request(sinricpro_device_t *device,
                                      const char *action,
                                      const cJSON *request,
                                      cJSON *response) {
    sinricpro_doorbell_t *doorbell = (sinricpro_doorbell_t *)device;

    if (strcmp(action, "setPowerState") == 0) {
        return sinricpro_power_state_handle_request(&doorbell->power_state,
                                                     device, request, response);
    }

    SINRICPRO_WARN_PRINTF("[Doorbell] Unknown action: %s\n", action);
    return false;
}
