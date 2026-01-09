/**
 * @file range_controller.h
 * @brief Range controller capability for SinricPro devices
 */

#ifndef SINRICPRO_RANGE_CONTROLLER_H
#define SINRICPRO_RANGE_CONTROLLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "cJSON.h"
#include "sinricpro/sinricpro_device.h"
#include "sinricpro/event_limiter.h"

/**
 * @brief Range value callback function type
 *
 * @param device      The device receiving the command
 * @param range_value Pointer to range value 0-100 (can be modified)
 * @return true if successful, false on error
 */
typedef bool (*sinricpro_range_value_callback_t)(sinricpro_device_t *device, int *range_value);

/**
 * @brief Adjust range value callback function type
 *
 * @param device      The device receiving the command
 * @param range_delta Pointer to delta value (can be modified to absolute value)
 * @return true if successful, false on error
 */
typedef bool (*sinricpro_adjust_range_callback_t)(sinricpro_device_t *device, int *range_delta);

/**
 * @brief Range controller capability structure
 */
typedef struct {
    int range_value;
    sinricpro_range_value_callback_t set_callback;
    sinricpro_adjust_range_callback_t adjust_callback;
    sinricpro_event_limiter_t event_limiter;
} sinricpro_range_controller_t;

/**
 * @brief Initialize range controller
 */
void sinricpro_range_controller_init(sinricpro_range_controller_t *controller);

/**
 * @brief Set range value callback
 */
void sinricpro_range_controller_set_callback(sinricpro_range_controller_t *controller,
                                               sinricpro_range_value_callback_t callback);

/**
 * @brief Set adjust range value callback
 */
void sinricpro_range_controller_set_adjust_callback(sinricpro_range_controller_t *controller,
                                                      sinricpro_adjust_range_callback_t callback);

/**
 * @brief Handle setRangeValue request
 */
bool sinricpro_range_controller_handle_set_request(sinricpro_range_controller_t *controller,
                                                     sinricpro_device_t *device,
                                                     const cJSON *request,
                                                     cJSON *response);

/**
 * @brief Handle adjustRangeValue request
 */
bool sinricpro_range_controller_handle_adjust_request(sinricpro_range_controller_t *controller,
                                                        sinricpro_device_t *device,
                                                        const cJSON *request,
                                                        cJSON *response);

/**
 * @brief Send range value event to server
 */
bool sinricpro_range_controller_send_event(sinricpro_range_controller_t *controller,
                                             const char *device_id,
                                             int range_value);

/**
 * @brief Get current range value
 */
int sinricpro_range_controller_get_value(const sinricpro_range_controller_t *controller);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_RANGE_CONTROLLER_H
