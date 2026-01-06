/**
 * @file power_level.c
 * @brief Power level capability implementation
 */

#include "sinricpro/capabilities/power_level.h"
#include "sinricpro/sinricpro.h"
#include "core/json_helpers.h"
#include "core/sinricpro_debug.h"
#include <stdio.h>
#include <string.h>

void sinricpro_power_level_init(sinricpro_power_level_t *power_level) {
    if (!power_level) return;

    power_level->current_power_level = 0;
    power_level->callback = NULL;
    power_level->adjust_callback = NULL;
    sinricpro_event_limiter_init_state(&power_level->event_limiter);
}

void sinricpro_power_level_set_callback(sinricpro_power_level_t *power_level,
                                        sinricpro_power_level_callback_t callback) {
    if (power_level) {
        power_level->callback = callback;
    }
}

void sinricpro_power_level_set_adjust_callback(sinricpro_power_level_t *power_level,
                                               sinricpro_adjust_power_level_callback_t callback) {
    if (power_level) {
        power_level->adjust_callback = callback;
    }
}

bool sinricpro_power_level_handle_set_request(sinricpro_power_level_t *power_level,
                                               sinricpro_device_t *device,
                                               const cJSON *request,
                                               cJSON *response) {
    if (!power_level || !device || !request || !response) {
        return false;
    }

    // Get value from request
    const cJSON *value = sinricpro_json_get_value(request);
    if (!value) {
        SINRICPRO_ERROR_PRINTF("[PowerLevel] No value in request\n");
        return false;
    }

    // Get power level from value object
    int level = sinricpro_json_get_int(value, "powerLevel", -1);
    if (level < 0) {
        SINRICPRO_ERROR_PRINTF("[PowerLevel] No powerLevel in request\n");
        return false;
    }

    SINRICPRO_DEBUG_PRINTF("[PowerLevel] setPowerLevel: %d\n", level);

    // Call user callback
    bool success = true;
    if (power_level->callback) {
        success = power_level->callback(device, &level);
    }

    if (success) {
        power_level->current_power_level = level;
    }

    // Build response value
    cJSON *resp_value = sinricpro_json_add_value(response);
    if (resp_value) {
        cJSON_AddNumberToObject(resp_value, "powerLevel", level);
    }

    return success;
}

bool sinricpro_power_level_handle_adjust_request(sinricpro_power_level_t *power_level,
                                                  sinricpro_device_t *device,
                                                  const cJSON *request,
                                                  cJSON *response) {
    if (!power_level || !device || !request || !response) {
        return false;
    }

    // Get value from request
    const cJSON *value = sinricpro_json_get_value(request);
    if (!value) {
        SINRICPRO_ERROR_PRINTF("[PowerLevel] No value in request\n");
        return false;
    }

    // Get power level delta from value object
    int delta = sinricpro_json_get_int(value, "powerLevelDelta", 0);

    SINRICPRO_DEBUG_PRINTF("[PowerLevel] adjustPowerLevel: delta=%d\n", delta);

    // Call user callback
    // NOTE: Callback receives delta, but should return absolute power level
    bool success = true;
    if (power_level->adjust_callback) {
        success = power_level->adjust_callback(device, &delta);
        // delta now contains the absolute power level
    }

    if (success) {
        power_level->current_power_level = delta;
    }

    // Build response value with absolute power level
    cJSON *resp_value = sinricpro_json_add_value(response);
    if (resp_value) {
        cJSON_AddNumberToObject(resp_value, "powerLevel", delta);
    }

    return success;
}

bool sinricpro_power_level_send_event(sinricpro_power_level_t *power_level,
                                      const char *device_id,
                                      int level) {
    if (!power_level || !device_id) {
        return false;
    }

    // Check rate limit
    if (sinricpro_event_limiter_check(&power_level->event_limiter)) {
        SINRICPRO_DEBUG_PRINTF("[PowerLevel] Event rate limited\n");
        return false;
    }

    // Create value JSON
    cJSON *value = cJSON_CreateObject();
    if (!value) {
        return false;
    }

    cJSON_AddNumberToObject(value, "powerLevel", level);

    // Send event
    bool result = sinricpro_send_event(device_id, "setPowerLevel", value);

    if (result) {
        power_level->current_power_level = level;
        SINRICPRO_DEBUG_PRINTF("[PowerLevel] Sent event: %d\n", level);
    }

    return result;
}

int sinricpro_power_level_get_value(const sinricpro_power_level_t *power_level) {
    return power_level ? power_level->current_power_level : 0;
}
