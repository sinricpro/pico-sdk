/**
 * @file sinricpro_temperature_sensor.c
 * @brief SinricPro Temperature Sensor device implementation
 */

#include "sinricpro/sinricpro_temperature_sensor.h"
#include "sinricpro/capabilities/temperature_sensor.h"
#include "core/json_helpers.h"
#include "core/sinricpro_debug.h"
#include <stdio.h>
#include <string.h>

// Forward declaration
static bool temp_sensor_handle_request(sinricpro_device_t *device,
                                       const char *action,
                                       const cJSON *request,
                                       cJSON *response);

bool sinricpro_temperature_sensor_init(sinricpro_temperature_sensor_t *device,
                                       const char *device_id) {
    if (!device || !device_id) {
        return false;
    }

    // Initialize base device
    if (!sinricpro_device_init(&device->base, device_id,
                               SINRICPRO_DEVICE_TYPE_TEMPERATURE_SENSOR)) {
        return false;
    }

    // Set request handler
    device->base.handle_request = temp_sensor_handle_request;

    // Initialize capabilities
    sinricpro_temperature_sensor_cap_init(&device->temp_humidity);

    SINRICPRO_DEBUG_PRINTF("[TempSensor] Initialized device: %s\n", device_id);
    return true;
}

bool sinricpro_temperature_sensor_send_event(sinricpro_temperature_sensor_t *device,
                                             float temperature,
                                             float humidity) {
    if (!device) {
        return false;
    }

    return sinricpro_temperature_sensor_cap_send_event(&device->temp_humidity,
                                                       device->base.device_id,
                                                       temperature,
                                                       humidity);
}

// Handle incoming requests (sensors typically don't receive many commands)
static bool temp_sensor_handle_request(sinricpro_device_t *device,
                                       const char *action,
                                       const cJSON *request,
                                       cJSON *response) {
    // Temperature sensors are event-only devices
    // Could implement state query if needed in future

    SINRICPRO_WARN_PRINTF("[TempSensor] Unknown action: %s\n", action);
    return false;
}
