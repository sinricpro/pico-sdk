/**
 * @file doorbell.h
 * @brief Doorbell capability for SinricPro devices
 */

#ifndef SINRICPRO_DOORBELL_CAP_H
#define SINRICPRO_DOORBELL_CAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "sinricpro/event_limiter.h"

/**
 * @brief Doorbell capability structure
 */
typedef struct {
    sinricpro_event_limiter_t event_limiter;
} sinricpro_doorbell_cap_t;

/**
 * @brief Initialize doorbell capability
 */
void sinricpro_doorbell_cap_init(sinricpro_doorbell_cap_t *doorbell);

/**
 * @brief Send doorbell press event to server
 *
 * @param doorbell   Doorbell capability instance
 * @param device_id  Device ID
 * @return true if event sent successfully
 */
bool sinricpro_doorbell_cap_send_event(sinricpro_doorbell_cap_t *doorbell,
                                        const char *device_id);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_DOORBELL_CAP_H
