/**
 * @file sinricpro_fan.c
 * @brief SinricPro Fan device implementation
 */

#include "sinricpro/sinricpro_fan.h"
#include "sinricpro/capabilities/power_state.h"
#include "sinricpro/capabilities/power_level.h"
#include "core/json_helpers.h"
#include "core/sinricpro_debug.h"
#include <stdio.h>
#include <string.h>

static bool fan_handle_request(sinricpro_device_t *device,
                                const char *action,
                                const cJSON *request,
                                cJSON *response);

bool sinricpro_fan_init(sinricpro_fan_t *device, const char *device_id) {
    if (!device || !device_id) return false;

    if (!sinricpro_device_init(&device->base, device_id, SINRICPRO_DEVICE_TYPE_FAN)) {
        return false;
    }

    device->base.handle_request = fan_handle_request;

    sinricpro_power_state_init(&device->power_state);
    sinricpro_power_level_init(&device->power_level);

    SINRICPRO_DEBUG_PRINTF("[Fan] Initialized device: %s\n", device_id);
    return true;
}

void sinricpro_fan_on_power_state(sinricpro_fan_t *device,
                                  sinricpro_power_state_callback_t callback) {
    if (device) {
        sinricpro_power_state_set_callback(&device->power_state, callback);
    }
}

void sinricpro_fan_on_power_level(sinricpro_fan_t *device,
                                  sinricpro_power_level_callback_t callback) {
    if (device) {
        sinricpro_power_level_set_callback(&device->power_level, callback);
    }
}

void sinricpro_fan_on_adjust_power_level(sinricpro_fan_t *device,
                                         sinricpro_adjust_power_level_callback_t callback) {
    if (device) {
        sinricpro_power_level_set_adjust_callback(&device->power_level, callback);
    }
}

bool sinricpro_fan_send_power_state_event(sinricpro_fan_t *device, bool state) {
    if (!device) return false;
    return sinricpro_power_state_send_event(&device->power_state,
                                            device->base.device_id,
                                            state);
}

bool sinricpro_fan_send_power_level_event(sinricpro_fan_t *device, int power_level) {
    if (!device) return false;
    return sinricpro_power_level_send_event(&device->power_level,
                                            device->base.device_id,
                                            power_level);
}

bool sinricpro_fan_get_power_state(const sinricpro_fan_t *device) {
    if (!device) return false;
    return sinricpro_power_state_get_state(&device->power_state);
}

int sinricpro_fan_get_power_level(const sinricpro_fan_t *device) {
    if (!device) return 0;
    return sinricpro_power_level_get_value(&device->power_level);
}

static bool fan_handle_request(sinricpro_device_t *device,
                                const char *action,
                                const cJSON *request,
                                cJSON *response) {
    sinricpro_fan_t *fan = (sinricpro_fan_t *)device;

    if (strcmp(action, "setPowerState") == 0) {
        return sinricpro_power_state_handle_request(&fan->power_state,
                                                    device, request, response);
    }

    if (strcmp(action, "setPowerLevel") == 0) {
        return sinricpro_power_level_handle_set_request(&fan->power_level,
                                                        device, request, response);
    }

    if (strcmp(action, "adjustPowerLevel") == 0) {
        return sinricpro_power_level_handle_adjust_request(&fan->power_level,
                                                           device, request, response);
    }

    SINRICPRO_WARN_PRINTF("[Fan] Unknown action: %s\n", action);
    return false;
}
