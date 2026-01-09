/**
 * @file sinricpro_powersensor.c
 * @brief SinricPro Power Sensor device implementation
 */

#include "sinricpro/sinricpro_powersensor.h"
#include "sinricpro/capabilities/power_sensor.h"
#include "core/json_helpers.h"
#include "core/sinricpro_debug.h"
#include <stdio.h>
#include <string.h>

static bool powersensor_handle_request(sinricpro_device_t *device,
                                         const char *action,
                                         const cJSON *request,
                                         cJSON *response);

bool sinricpro_powersensor_init(sinricpro_powersensor_t *device, const char *device_id) {
    if (!device || !device_id) return false;

    if (!sinricpro_device_init(&device->base, device_id, SINRICPRO_DEVICE_TYPE_POWER_SENSOR)) {
        return false;
    }

    device->base.handle_request = powersensor_handle_request;

    sinricpro_power_sensor_init(&device->power_sensor);

    SINRICPRO_DEBUG_PRINTF("[PowerSensor] Initialized device: %s\n", device_id);
    return true;
}

bool sinricpro_powersensor_send_power_event(sinricpro_powersensor_t *device,
                                              float voltage,
                                              float current,
                                              float power,
                                              float apparent_power,
                                              float reactive_power,
                                              float factor) {
    if (!device) return false;
    return sinricpro_power_sensor_send_event(&device->power_sensor,
                                              device->base.device_id,
                                              voltage, current, power,
                                              apparent_power, reactive_power, factor);
}

static bool powersensor_handle_request(sinricpro_device_t *device,
                                         const char *action,
                                         const cJSON *request,
                                         cJSON *response) {
    // Power sensor is event-only, no incoming commands
    SINRICPRO_WARN_PRINTF("[PowerSensor] Unknown action: %s\n", action);
    return false;
}
