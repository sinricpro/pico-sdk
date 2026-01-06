/**
 * @file sinricpro_light.c
 * @brief SinricPro Light device implementation
 */

#include "sinricpro/sinricpro_light.h"
#include "sinricpro/capabilities/power_state.h"
#include "sinricpro/capabilities/brightness.h"
#include "sinricpro/capabilities/color.h"
#include "sinricpro/capabilities/color_temperature.h"
#include "core/json_helpers.h"
#include "core/sinricpro_debug.h"
#include <stdio.h>
#include <string.h>

// Forward declaration
static bool light_handle_request(sinricpro_device_t *device,
                                  const char *action,
                                  const cJSON *request,
                                  cJSON *response);

bool sinricpro_light_init(sinricpro_light_t *device, const char *device_id) {
    if (!device || !device_id) {
        return false;
    }

    // Initialize base device
    if (!sinricpro_device_init(&device->base, device_id,
                               SINRICPRO_DEVICE_TYPE_LIGHT)) {
        return false;
    }

    // Set request handler
    device->base.handle_request = light_handle_request;

    // Initialize capabilities
    sinricpro_power_state_init(&device->power_state);
    sinricpro_brightness_init(&device->brightness);
    sinricpro_color_init(&device->color);
    sinricpro_color_temp_init(&device->color_temp);

    SINRICPRO_DEBUG_PRINTF("[Light] Initialized device: %s\n", device_id);
    return true;
}

void sinricpro_light_on_power_state(sinricpro_light_t *device,
                                    sinricpro_power_state_callback_t callback) {
    if (device) {
        sinricpro_power_state_set_callback(&device->power_state, callback);
    }
}

void sinricpro_light_on_brightness(sinricpro_light_t *device,
                                   sinricpro_brightness_callback_t callback) {
    if (device) {
        sinricpro_brightness_set_callback(&device->brightness, callback);
    }
}

void sinricpro_light_on_adjust_brightness(sinricpro_light_t *device,
                                          sinricpro_adjust_brightness_callback_t callback) {
    if (device) {
        sinricpro_brightness_set_adjust_callback(&device->brightness, callback);
    }
}

void sinricpro_light_on_color(sinricpro_light_t *device,
                              sinricpro_color_callback_t callback) {
    if (device) {
        sinricpro_color_set_callback(&device->color, callback);
    }
}

void sinricpro_light_on_color_temperature(sinricpro_light_t *device,
                                          sinricpro_color_temp_callback_t callback) {
    if (device) {
        sinricpro_color_temp_set_callback(&device->color_temp, callback);
    }
}

void sinricpro_light_on_increase_color_temperature(sinricpro_light_t *device,
                                                   sinricpro_color_temp_adjust_callback_t callback) {
    if (device) {
        sinricpro_color_temp_set_increase_callback(&device->color_temp, callback);
    }
}

void sinricpro_light_on_decrease_color_temperature(sinricpro_light_t *device,
                                                   sinricpro_color_temp_adjust_callback_t callback) {
    if (device) {
        sinricpro_color_temp_set_decrease_callback(&device->color_temp, callback);
    }
}

bool sinricpro_light_send_power_state_event(sinricpro_light_t *device, bool state) {
    if (!device) {
        return false;
    }

    return sinricpro_power_state_send_event(&device->power_state,
                                            device->base.device_id,
                                            state);
}

bool sinricpro_light_send_brightness_event(sinricpro_light_t *device, int brightness) {
    if (!device) {
        return false;
    }

    return sinricpro_brightness_send_event(&device->brightness,
                                           device->base.device_id,
                                           brightness);
}

bool sinricpro_light_send_color_event(sinricpro_light_t *device, sinricpro_color_t color) {
    if (!device) {
        return false;
    }

    return sinricpro_color_send_event(&device->color,
                                      device->base.device_id,
                                      color);
}

bool sinricpro_light_send_color_temp_event(sinricpro_light_t *device, int color_temp) {
    if (!device) {
        return false;
    }

    return sinricpro_color_temp_send_event(&device->color_temp,
                                           device->base.device_id,
                                           color_temp);
}

bool sinricpro_light_get_power_state(const sinricpro_light_t *device) {
    if (!device) {
        return false;
    }

    return sinricpro_power_state_get_state(&device->power_state);
}

int sinricpro_light_get_brightness(const sinricpro_light_t *device) {
    if (!device) {
        return 0;
    }

    return sinricpro_brightness_get_value(&device->brightness);
}

sinricpro_color_t sinricpro_light_get_color(const sinricpro_light_t *device) {
    sinricpro_color_t black = {0, 0, 0};
    if (!device) {
        return black;
    }

    return sinricpro_color_get_value(&device->color);
}

int sinricpro_light_get_color_temp(const sinricpro_light_t *device) {
    if (!device) {
        return 2700;
    }

    return sinricpro_color_temp_get_value(&device->color_temp);
}

// ============================================================================
// Internal Functions
// ============================================================================

static bool light_handle_request(sinricpro_device_t *device,
                                  const char *action,
                                  const cJSON *request,
                                  cJSON *response) {
    // Cast to light device
    sinricpro_light_t *light = (sinricpro_light_t *)device;

    // PowerState
    if (strcmp(action, "setPowerState") == 0) {
        return sinricpro_power_state_handle_request(&light->power_state,
                                                    device, request, response);
    }

    // Brightness
    if (strcmp(action, "setBrightness") == 0) {
        return sinricpro_brightness_handle_set_request(&light->brightness,
                                                       device, request, response);
    }

    if (strcmp(action, "adjustBrightness") == 0) {
        return sinricpro_brightness_handle_adjust_request(&light->brightness,
                                                          device, request, response);
    }

    // Color
    if (strcmp(action, "setColor") == 0) {
        return sinricpro_color_handle_request(&light->color,
                                              device, request, response);
    }

    // Color Temperature
    if (strcmp(action, "setColorTemperature") == 0 ||
        strcmp(action, "increaseColorTemperature") == 0 ||
        strcmp(action, "decreaseColorTemperature") == 0) {
        return sinricpro_color_temp_handle_request(&light->color_temp,
                                                   device, action, request, response);
    }

    SINRICPRO_WARN_PRINTF("[Light] Unknown action: %s\n", action);
    return false;
}
