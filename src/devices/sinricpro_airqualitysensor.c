/**
 * @file sinricpro_airqualitysensor.c
 * @brief SinricPro Air Quality Sensor device implementation
 */

#include "sinricpro/sinricpro_airqualitysensor.h"
#include "sinricpro/capabilities/air_quality_sensor.h"
#include "core/json_helpers.h"
#include "core/sinricpro_debug.h"
#include <stdio.h>
#include <string.h>

static bool airqualitysensor_handle_request(sinricpro_device_t *device,
                                              const char *action,
                                              const cJSON *request,
                                              cJSON *response);

bool sinricpro_airqualitysensor_init(sinricpro_airqualitysensor_t *device, const char *device_id) {
    if (!device || !device_id) return false;

    if (!sinricpro_device_init(&device->base, device_id, SINRICPRO_DEVICE_TYPE_AIR_QUALITY_SENSOR)) {
        return false;
    }

    device->base.handle_request = airqualitysensor_handle_request;

    sinricpro_air_quality_sensor_init(&device->air_quality_sensor);

    SINRICPRO_DEBUG_PRINTF("[AirQualitySensor] Initialized device: %s\n", device_id);
    return true;
}

bool sinricpro_airqualitysensor_send_event(sinricpro_airqualitysensor_t *device,
                                             int pm1,
                                             int pm2_5,
                                             int pm10) {
    if (!device) return false;
    return sinricpro_air_quality_sensor_send_event(&device->air_quality_sensor,
                                                     device->base.device_id,
                                                     pm1, pm2_5, pm10);
}

static bool airqualitysensor_handle_request(sinricpro_device_t *device,
                                              const char *action,
                                              const cJSON *request,
                                              cJSON *response) {
    // Air quality sensor is event-only, no incoming commands
    SINRICPRO_WARN_PRINTF("[AirQualitySensor] Unknown action: %s\n", action);
    return false;
}
