/**
 * @file json_helpers.h
 * @brief JSON utility functions for SinricPro message handling
 *
 * Provides convenience functions for building and parsing SinricPro
 * protocol messages using cJSON library.
 */

#ifndef SINRICPRO_JSON_HELPERS_H
#define SINRICPRO_JSON_HELPERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "cJSON.h"

/**
 * @brief Message type enumeration
 */
typedef enum {
    SINRICPRO_MSG_REQUEST,
    SINRICPRO_MSG_RESPONSE,
    SINRICPRO_MSG_EVENT
} sinricpro_message_type_t;

/**
 * @brief Create a new SinricPro message structure
 *
 * Creates the base message structure with header, payload, and signature.
 *
 * @return cJSON object, or NULL on failure. Caller must free with cJSON_Delete.
 */
cJSON *sinricpro_json_create_message(void);

/**
 * @brief Create a response message from a request
 *
 * @param request     The original request JSON
 * @param success     Whether the request was successful
 * @return cJSON response object, or NULL on failure
 */
cJSON *sinricpro_json_create_response(const cJSON *request, bool success);

/**
 * @brief Create an event message
 *
 * @param device_id   The device ID (24-char hex string)
 * @param action      The action name (e.g., "setPowerState")
 * @return cJSON event object, or NULL on failure
 */
cJSON *sinricpro_json_create_event(const char *device_id, const char *action);

/**
 * @brief Add value object to message payload
 *
 * @param message     The message JSON object
 * @return Pointer to value object, or NULL on failure
 */
cJSON *sinricpro_json_add_value(cJSON *message);

/**
 * @brief Get value object from message payload
 *
 * @param message     The message JSON object
 * @return Pointer to value object, or NULL if not found
 */
cJSON *sinricpro_json_get_value(const cJSON *message);

/**
 * @brief Get string from JSON object
 *
 * @param object      The JSON object
 * @param key         The key to look up
 * @param default_val Default value if not found
 * @return String value or default
 */
const char *sinricpro_json_get_string(const cJSON *object, const char *key,
                                       const char *default_val);

/**
 * @brief Get integer from JSON object
 *
 * @param object      The JSON object
 * @param key         The key to look up
 * @param default_val Default value if not found
 * @return Integer value or default
 */
int sinricpro_json_get_int(const cJSON *object, const char *key, int default_val);

/**
 * @brief Get double from JSON object
 *
 * @param object      The JSON object
 * @param key         The key to look up
 * @param default_val Default value if not found
 * @return Double value or default
 */
double sinricpro_json_get_double(const cJSON *object, const char *key,
                                  double default_val);

/**
 * @brief Get boolean from JSON object
 *
 * @param object      The JSON object
 * @param key         The key to look up
 * @param default_val Default value if not found
 * @return Boolean value or default
 */
bool sinricpro_json_get_bool(const cJSON *object, const char *key, bool default_val);

/**
 * @brief Get action from message
 *
 * @param message     The message JSON object
 * @return Action string, or NULL if not found
 */
const char *sinricpro_json_get_action(const cJSON *message);

/**
 * @brief Get device ID from message
 *
 * @param message     The message JSON object
 * @return Device ID string, or NULL if not found
 */
const char *sinricpro_json_get_device_id(const cJSON *message);

/**
 * @brief Get message type from message
 *
 * @param message     The message JSON object
 * @return Message type string (request/response/event), or NULL
 */
const char *sinricpro_json_get_type(const cJSON *message);

/**
 * @brief Get reply token from message
 *
 * @param message     The message JSON object
 * @return Reply token string, or NULL if not found
 */
const char *sinricpro_json_get_reply_token(const cJSON *message);

/**
 * @brief Get signature from message
 *
 * @param message     The message JSON object
 * @return HMAC signature string, or NULL if not found
 */
const char *sinricpro_json_get_signature(const cJSON *message);

/**
 * @brief Set signature on message
 *
 * @param message     The message JSON object
 * @param signature   The HMAC signature to set
 * @return true on success, false on failure
 */
bool sinricpro_json_set_signature(cJSON *message, const char *signature);

/**
 * @brief Serialize JSON to string (no whitespace)
 *
 * @param json        The JSON object to serialize
 * @param output      Output buffer
 * @param output_len  Size of output buffer
 * @return Length of serialized string, or 0 on failure
 */
size_t sinricpro_json_serialize(const cJSON *json, char *output, size_t output_len);

/**
 * @brief Serialize payload to string for signing
 *
 * @param message     The message JSON object
 * @param output      Output buffer
 * @param output_len  Size of output buffer
 * @return Length of serialized string, or 0 on failure
 */
size_t sinricpro_json_serialize_payload(const cJSON *message, char *output,
                                        size_t output_len);

/**
 * @brief Generate a UUID v4 string
 *
 * @param output      Buffer for UUID (must be >= 37 bytes)
 * @param output_len  Size of output buffer
 * @return true on success, false on failure
 */
bool sinricpro_json_generate_uuid(char *output, size_t output_len);

/**
 * @brief Get current timestamp (Unix epoch seconds)
 *
 * @return Current timestamp, or 0 if time not available
 */
uint32_t sinricpro_json_get_timestamp(void);

/**
 * @brief Set timestamp offset from server time
 *
 * Call this when the server sends a timestamp message to sync local time.
 *
 * @param unix_time Server's Unix timestamp
 */
void sinricpro_json_set_timestamp_offset(uint32_t unix_time);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_JSON_HELPERS_H
