/**
 * @file sinricpro_motion_sensor.c
 * @brief SinricPro Motion Sensor device implementation
 */

#include "sinricpro/sinricpro_motion_sensor.h"
#include "sinricpro/capabilities/motion_sensor.h"
#include "core/json_helpers.h"
#include "core/sinricpro_debug.h"
#include <stdio.h>
#include <string.h>

// Forward declaration
static bool motion_sensor_handle_request(sinricpro_device_t *device,
                                         const char *action,
                                         const cJSON *request,
                                         cJSON *response);

bool sinricpro_motion_sensor_init(sinricpro_motion_sensor_t *device,
                                  const char *device_id) {
    if (!device || !device_id) {
        return false;
    }

    // Initialize base device
    if (!sinricpro_device_init(&device->base, device_id,
                               SINRICPRO_DEVICE_TYPE_MOTION_SENSOR)) {
        return false;
    }

    // Set request handler
    device->base.handle_request = motion_sensor_handle_request;

    // Initialize capabilities
    sinricpro_motion_sensor_cap_init(&device->motion);

    SINRICPRO_DEBUG_PRINTF("[MotionSensor] Initialized device: %s\n", device_id);
    return true;
}

bool sinricpro_motion_sensor_send_event(sinricpro_motion_sensor_t *device,
                                        bool detected) {
    if (!device) {
        return false;
    }

    return sinricpro_motion_sensor_cap_send_event(&device->motion,
                                                  device->base.device_id,
                                                  detected);
}

// Handle incoming requests (sensors typically don't receive many commands)
static bool motion_sensor_handle_request(sinricpro_device_t *device,
                                         const char *action,
                                         const cJSON *request,
                                         cJSON *response) {
    // Motion sensors are event-only devices
    // Could implement state query if needed in future

    SINRICPRO_WARN_PRINTF("[MotionSensor] Unknown action: %s\n", action);
    return false;
}
