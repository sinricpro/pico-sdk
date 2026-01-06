/**
 * @file sinricpro_dimswitch.c
 * @brief SinricPro DimSwitch device implementation
 */

#include "sinricpro/sinricpro_dimswitch.h"
#include "sinricpro/capabilities/power_state.h"
#include "sinricpro/capabilities/brightness.h"
#include "core/json_helpers.h"
#include "core/sinricpro_debug.h"
#include <stdio.h>
#include <string.h>

// Forward declaration
static bool dimswitch_handle_request(sinricpro_device_t *device,
                                     const char *action,
                                     const cJSON *request,
                                     cJSON *response);

bool sinricpro_dimswitch_init(sinricpro_dimswitch_t *device, const char *device_id) {
    if (!device || !device_id) {
        return false;
    }

    // Initialize base device
    if (!sinricpro_device_init(&device->base, device_id,
                               SINRICPRO_DEVICE_TYPE_DIMSWITCH)) {
        return false;
    }

    // Set request handler
    device->base.handle_request = dimswitch_handle_request;

    // Initialize capabilities
    sinricpro_power_state_init(&device->power_state);
    sinricpro_brightness_init(&device->brightness);

    SINRICPRO_DEBUG_PRINTF("[DimSwitch] Initialized device: %s\n", device_id);
    return true;
}

void sinricpro_dimswitch_on_power_state(sinricpro_dimswitch_t *device,
                                        sinricpro_power_state_callback_t callback) {
    if (device) {
        sinricpro_power_state_set_callback(&device->power_state, callback);
    }
}

void sinricpro_dimswitch_on_brightness(sinricpro_dimswitch_t *device,
                                       sinricpro_brightness_callback_t callback) {
    if (device) {
        sinricpro_brightness_set_callback(&device->brightness, callback);
    }
}

void sinricpro_dimswitch_on_adjust_brightness(sinricpro_dimswitch_t *device,
                                              sinricpro_adjust_brightness_callback_t callback) {
    if (device) {
        sinricpro_brightness_set_adjust_callback(&device->brightness, callback);
    }
}

bool sinricpro_dimswitch_send_power_state_event(sinricpro_dimswitch_t *device, bool state) {
    if (!device) {
        return false;
    }

    return sinricpro_power_state_send_event(&device->power_state,
                                            device->base.device_id,
                                            state);
}

bool sinricpro_dimswitch_send_brightness_event(sinricpro_dimswitch_t *device, int brightness) {
    if (!device) {
        return false;
    }

    return sinricpro_brightness_send_event(&device->brightness,
                                           device->base.device_id,
                                           brightness);
}

bool sinricpro_dimswitch_get_power_state(const sinricpro_dimswitch_t *device) {
    if (!device) {
        return false;
    }

    return sinricpro_power_state_get_state(&device->power_state);
}

int sinricpro_dimswitch_get_brightness(const sinricpro_dimswitch_t *device) {
    if (!device) {
        return 0;
    }

    return sinricpro_brightness_get_value(&device->brightness);
}

// ============================================================================
// Internal Functions
// ============================================================================

static bool dimswitch_handle_request(sinricpro_device_t *device,
                                     const char *action,
                                     const cJSON *request,
                                     cJSON *response) {
    // Cast to dimswitch device
    sinricpro_dimswitch_t *dimswitch = (sinricpro_dimswitch_t *)device;

    if (strcmp(action, "setPowerState") == 0) {
        return sinricpro_power_state_handle_request(&dimswitch->power_state,
                                                    device, request, response);
    }

    if (strcmp(action, "setBrightness") == 0) {
        return sinricpro_brightness_handle_set_request(&dimswitch->brightness,
                                                       device, request, response);
    }

    if (strcmp(action, "adjustBrightness") == 0) {
        return sinricpro_brightness_handle_adjust_request(&dimswitch->brightness,
                                                          device, request, response);
    }

    SINRICPRO_WARN_PRINTF("[DimSwitch] Unknown action: %s\n", action);
    return false;
}
