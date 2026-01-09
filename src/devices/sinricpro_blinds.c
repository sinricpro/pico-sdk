/**
 * @file sinricpro_blinds.c
 * @brief SinricPro Blinds device implementation
 */

#include "sinricpro/sinricpro_blinds.h"
#include "sinricpro/capabilities/power_state.h"
#include "sinricpro/capabilities/range_controller.h"
#include "core/json_helpers.h"
#include "core/sinricpro_debug.h"
#include <stdio.h>
#include <string.h>

static bool blinds_handle_request(sinricpro_device_t *device,
                                    const char *action,
                                    const cJSON *request,
                                    cJSON *response);

bool sinricpro_blinds_init(sinricpro_blinds_t *device, const char *device_id) {
    if (!device || !device_id) return false;

    if (!sinricpro_device_init(&device->base, device_id, SINRICPRO_DEVICE_TYPE_BLINDS)) {
        return false;
    }

    device->base.handle_request = blinds_handle_request;

    sinricpro_power_state_init(&device->power_state);
    sinricpro_range_controller_init(&device->range_controller);

    SINRICPRO_DEBUG_PRINTF("[Blinds] Initialized device: %s\n", device_id);
    return true;
}

void sinricpro_blinds_on_power_state(sinricpro_blinds_t *device,
                                      sinricpro_power_state_callback_t callback) {
    if (device) {
        sinricpro_power_state_set_callback(&device->power_state, callback);
    }
}

void sinricpro_blinds_on_range_value(sinricpro_blinds_t *device,
                                      sinricpro_range_value_callback_t callback) {
    if (device) {
        sinricpro_range_controller_set_callback(&device->range_controller, callback);
    }
}

void sinricpro_blinds_on_adjust_range(sinricpro_blinds_t *device,
                                       sinricpro_adjust_range_callback_t callback) {
    if (device) {
        sinricpro_range_controller_set_adjust_callback(&device->range_controller, callback);
    }
}

bool sinricpro_blinds_send_power_state_event(sinricpro_blinds_t *device, bool state) {
    if (!device) return false;
    return sinricpro_power_state_send_event(&device->power_state,
                                             device->base.device_id,
                                             state);
}

bool sinricpro_blinds_send_range_value_event(sinricpro_blinds_t *device, int position) {
    if (!device) return false;
    return sinricpro_range_controller_send_event(&device->range_controller,
                                                  device->base.device_id,
                                                  position);
}

bool sinricpro_blinds_get_power_state(const sinricpro_blinds_t *device) {
    if (!device) return false;
    return sinricpro_power_state_get_state(&device->power_state);
}

int sinricpro_blinds_get_position(const sinricpro_blinds_t *device) {
    if (!device) return 0;
    return sinricpro_range_controller_get_value(&device->range_controller);
}

static bool blinds_handle_request(sinricpro_device_t *device,
                                    const char *action,
                                    const cJSON *request,
                                    cJSON *response) {
    sinricpro_blinds_t *blinds = (sinricpro_blinds_t *)device;

    if (strcmp(action, "setPowerState") == 0) {
        return sinricpro_power_state_handle_request(&blinds->power_state,
                                                     device, request, response);
    }

    if (strcmp(action, "setRangeValue") == 0) {
        return sinricpro_range_controller_handle_set_request(&blinds->range_controller,
                                                              device, request, response);
    }

    if (strcmp(action, "adjustRangeValue") == 0) {
        return sinricpro_range_controller_handle_adjust_request(&blinds->range_controller,
                                                                 device, request, response);
    }

    SINRICPRO_WARN_PRINTF("[Blinds] Unknown action: %s\n", action);
    return false;
}
