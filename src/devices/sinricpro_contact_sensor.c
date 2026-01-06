/**
 * @file sinricpro_contact_sensor.c
 * @brief SinricPro Contact Sensor device implementation
 */

#include "sinricpro/sinricpro_contact_sensor.h"
#include "sinricpro/capabilities/contact_sensor.h"
#include "core/json_helpers.h"
#include "core/sinricpro_debug.h"
#include <stdio.h>
#include <string.h>

// Forward declaration
static bool contact_sensor_handle_request(sinricpro_device_t *device,
                                          const char *action,
                                          const cJSON *request,
                                          cJSON *response);

bool sinricpro_contact_sensor_init(sinricpro_contact_sensor_t *device,
                                   const char *device_id) {
    if (!device || !device_id) {
        return false;
    }

    // Initialize base device
    if (!sinricpro_device_init(&device->base, device_id,
                               SINRICPRO_DEVICE_TYPE_CONTACT_SENSOR)) {
        return false;
    }

    // Set request handler
    device->base.handle_request = contact_sensor_handle_request;

    // Initialize capabilities
    sinricpro_contact_sensor_cap_init(&device->contact);

    SINRICPRO_DEBUG_PRINTF("[ContactSensor] Initialized device: %s\n", device_id);
    return true;
}

bool sinricpro_contact_sensor_send_event(sinricpro_contact_sensor_t *device,
                                         bool is_open) {
    if (!device) {
        return false;
    }

    return sinricpro_contact_sensor_cap_send_event(&device->contact,
                                                   device->base.device_id,
                                                   is_open);
}

// Handle incoming requests (sensors typically don't receive many commands)
static bool contact_sensor_handle_request(sinricpro_device_t *device,
                                          const char *action,
                                          const cJSON *request,
                                          cJSON *response) {
    // Contact sensors are event-only devices
    // Could implement state query if needed in future

    SINRICPRO_WARN_PRINTF("[ContactSensor] Unknown action: %s\n", action);
    return false;
}
