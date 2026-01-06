/**
 * @file sinricpro_switch.c
 * @brief SinricPro Switch device implementation
 */

#include "sinricpro/sinricpro_switch.h"
#include "sinricpro/capabilities/power_state.h"
#include "core/json_helpers.h"
#include "core/sinricpro_debug.h"
#include <stdio.h>
#include <string.h>

// Forward declaration
static bool switch_handle_request(sinricpro_device_t *device,
                                  const char *action,
                                  const cJSON *request,
                                  cJSON *response);

bool sinricpro_switch_init(sinricpro_switch_t *device, const char *device_id) {
    if (!device || !device_id) {
        return false;
    }

    // Initialize base device
    if (!sinricpro_device_init(&device->base, device_id,
                               SINRICPRO_DEVICE_TYPE_SWITCH)) {
        return false;
    }

    // Set request handler
    device->base.handle_request = switch_handle_request;

    // Initialize capabilities
    sinricpro_power_state_init(&device->power_state);

    SINRICPRO_DEBUG_PRINTF("[Switch] Initialized device: %s\n", device_id);
    return true;
}

void sinricpro_switch_on_power_state(sinricpro_switch_t *device,
                                     sinricpro_power_state_callback_t callback) {
    if (device) {
        sinricpro_power_state_set_callback(&device->power_state, callback);
    }
}

bool sinricpro_switch_send_power_state_event(sinricpro_switch_t *device, bool state) {
    if (!device) {
        return false;
    }

    return sinricpro_power_state_send_event(&device->power_state,
                                            device->base.device_id,
                                            state);
}

bool sinricpro_switch_get_power_state(const sinricpro_switch_t *device) {
    if (!device) {
        return false;
    }

    return sinricpro_power_state_get_state(&device->power_state);
}

// ============================================================================
// Internal Functions
// ============================================================================

static bool switch_handle_request(sinricpro_device_t *device,
                                  const char *action,
                                  const cJSON *request,
                                  cJSON *response) {
    // Cast to switch device
    sinricpro_switch_t *sw = (sinricpro_switch_t *)device;

    if (strcmp(action, "setPowerState") == 0) {
        return sinricpro_power_state_handle_request(&sw->power_state,
                                                    device, request, response);
    }

    SINRICPRO_WARN_PRINTF("[Switch] Unknown action: %s\n", action);
    return false;
}
