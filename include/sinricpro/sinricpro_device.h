/**
 * @file sinricpro_device.h
 * @brief Base device structure and common device interface for SinricPro
 */

#ifndef SINRICPRO_DEVICE_H
#define SINRICPRO_DEVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "sinricpro_config.h"
#include "cJSON.h"

// Forward declaration
typedef struct sinricpro_device sinricpro_device_t;

/**
 * @brief Device type identifiers
 */
typedef enum {
    SINRICPRO_DEVICE_TYPE_UNKNOWN = 0,
    SINRICPRO_DEVICE_TYPE_SWITCH,
    SINRICPRO_DEVICE_TYPE_DIMSWITCH,
    SINRICPRO_DEVICE_TYPE_LIGHT,
    SINRICPRO_DEVICE_TYPE_TEMPERATURE_SENSOR,
    SINRICPRO_DEVICE_TYPE_CONTACT_SENSOR,
    SINRICPRO_DEVICE_TYPE_MOTION_SENSOR,
    SINRICPRO_DEVICE_TYPE_BLINDS,
    SINRICPRO_DEVICE_TYPE_GARAGE_DOOR,
    SINRICPRO_DEVICE_TYPE_LOCK,
    SINRICPRO_DEVICE_TYPE_THERMOSTAT,
    SINRICPRO_DEVICE_TYPE_FAN,
    SINRICPRO_DEVICE_TYPE_TV,
    SINRICPRO_DEVICE_TYPE_SPEAKER,
    SINRICPRO_DEVICE_TYPE_DOORBELL,
    SINRICPRO_DEVICE_TYPE_WINDOW_AC,
    SINRICPRO_DEVICE_TYPE_POWER_SENSOR,
    SINRICPRO_DEVICE_TYPE_AIR_QUALITY_SENSOR,
    SINRICPRO_DEVICE_TYPE_CAMERA
} sinricpro_device_type_t;

/**
 * @brief Request handler function type
 *
 * @param device    The device receiving the request
 * @param action    The action name
 * @param request   The full request JSON
 * @param response  Response JSON to populate
 * @return true if handled successfully, false otherwise
 */
typedef bool (*sinricpro_request_handler_t)(
    sinricpro_device_t *device,
    const char *action,
    const cJSON *request,
    cJSON *response);

/**
 * @brief Base device structure
 *
 * All device types embed this structure as their first member.
 */
struct sinricpro_device {
    char device_id[SINRICPRO_DEVICE_ID_LENGTH + 1];
    sinricpro_device_type_t type;

    // Request handler (implemented by device type)
    sinricpro_request_handler_t handle_request;

    // User data
    void *user_data;
};

/**
 * @brief Initialize base device structure
 *
 * @param device    Device to initialize
 * @param device_id Device ID (24-character hex string)
 * @param type      Device type
 * @return true on success, false on failure
 */
bool sinricpro_device_init(sinricpro_device_t *device,
                           const char *device_id,
                           sinricpro_device_type_t type);

/**
 * @brief Get device ID
 *
 * @param device Device pointer
 * @return Device ID string
 */
const char *sinricpro_device_get_id(const sinricpro_device_t *device);

/**
 * @brief Get device type
 *
 * @param device Device pointer
 * @return Device type
 */
sinricpro_device_type_t sinricpro_device_get_type(const sinricpro_device_t *device);

/**
 * @brief Set user data on device
 *
 * @param device    Device pointer
 * @param user_data User data pointer
 */
void sinricpro_device_set_user_data(sinricpro_device_t *device, void *user_data);

/**
 * @brief Get user data from device
 *
 * @param device Device pointer
 * @return User data pointer
 */
void *sinricpro_device_get_user_data(const sinricpro_device_t *device);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_DEVICE_H
