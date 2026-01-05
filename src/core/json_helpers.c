/**
 * @file json_helpers.c
 * @brief JSON utility functions implementation for SinricPro
 */

#include "json_helpers.h"
#include "sinricpro/sinricpro_config.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "pico/time.h"
#include "pico/rand.h"

// Static timestamp offset (set when NTP sync occurs)
static uint32_t timestamp_offset = 0;

cJSON *sinricpro_json_create_message(void) {
    cJSON *message = cJSON_CreateObject();
    if (!message) return NULL;

    // Create header
    cJSON *header = cJSON_CreateObject();
    if (!header) {
        cJSON_Delete(message);
        return NULL;
    }
    cJSON_AddNumberToObject(header, "payloadVersion", SINRICPRO_PAYLOAD_VERSION);
    cJSON_AddNumberToObject(header, "signatureVersion", SINRICPRO_SIGNATURE_VERSION);
    cJSON_AddItemToObject(message, "header", header);

    // Create payload
    cJSON *payload = cJSON_CreateObject();
    if (!payload) {
        cJSON_Delete(message);
        return NULL;
    }
    cJSON_AddItemToObject(message, "payload", payload);

    // Create signature placeholder
    cJSON *signature = cJSON_CreateObject();
    if (!signature) {
        cJSON_Delete(message);
        return NULL;
    }
    cJSON_AddStringToObject(signature, "HMAC", "");
    cJSON_AddItemToObject(message, "signature", signature);

    return message;
}

cJSON *sinricpro_json_create_response(const cJSON *request, bool success) {
    if (!request) return NULL;

    cJSON *response = sinricpro_json_create_message();
    if (!response) return NULL;

    cJSON *payload = cJSON_GetObjectItem(response, "payload");
    if (!payload) {
        cJSON_Delete(response);
        return NULL;
    }

    // Get values from request payload
    const cJSON *req_payload = cJSON_GetObjectItem(request, "payload");
    if (!req_payload) {
        cJSON_Delete(response);
        return NULL;
    }

    const char *action = sinricpro_json_get_string(req_payload, "action", "");
    const char *client_id = sinricpro_json_get_string(req_payload, "clientId", "");
    const char *device_id = sinricpro_json_get_string(req_payload, "deviceId", "");
    const char *reply_token = sinricpro_json_get_string(req_payload, "replyToken", "");

    // Add response fields
    cJSON_AddStringToObject(payload, "action", action);
    cJSON_AddStringToObject(payload, "clientId", client_id);
    cJSON_AddNumberToObject(payload, "createdAt", sinricpro_json_get_timestamp());
    cJSON_AddStringToObject(payload, "deviceId", device_id);

    // Generate message ID
    char message_id[40];
    sinricpro_json_generate_uuid(message_id, sizeof(message_id));
    cJSON_AddStringToObject(payload, "message", message_id);

    cJSON_AddStringToObject(payload, "replyToken", reply_token);
    cJSON_AddBoolToObject(payload, "success", success);
    cJSON_AddStringToObject(payload, "type", SINRICPRO_TYPE_RESPONSE);

    // Add empty value object
    cJSON_AddItemToObject(payload, "value", cJSON_CreateObject());

    return response;
}

cJSON *sinricpro_json_create_event(const char *device_id, const char *action) {
    if (!device_id || !action) return NULL;

    cJSON *event = sinricpro_json_create_message();
    if (!event) return NULL;

    cJSON *payload = cJSON_GetObjectItem(event, "payload");
    if (!payload) {
        cJSON_Delete(event);
        return NULL;
    }

    // Add event fields
    cJSON_AddStringToObject(payload, "action", action);

    // Add cause
    cJSON *cause = cJSON_CreateObject();
    cJSON_AddStringToObject(cause, "type", SINRICPRO_CAUSE_PHYSICAL);
    cJSON_AddItemToObject(payload, "cause", cause);

    cJSON_AddNumberToObject(payload, "createdAt", sinricpro_json_get_timestamp());
    cJSON_AddStringToObject(payload, "deviceId", device_id);

    // Generate reply token (UUID)
    char reply_token[40];
    sinricpro_json_generate_uuid(reply_token, sizeof(reply_token));
    cJSON_AddStringToObject(payload, "replyToken", reply_token);

    cJSON_AddStringToObject(payload, "type", SINRICPRO_TYPE_EVENT);

    // Add empty value object
    cJSON_AddItemToObject(payload, "value", cJSON_CreateObject());

    return event;
}

cJSON *sinricpro_json_add_value(cJSON *message) {
    if (!message) return NULL;

    cJSON *payload = cJSON_GetObjectItem(message, "payload");
    if (!payload) return NULL;

    cJSON *value = cJSON_GetObjectItem(payload, "value");
    if (!value) {
        value = cJSON_CreateObject();
        if (!value) return NULL;
        cJSON_AddItemToObject(payload, "value", value);
    }

    return value;
}

cJSON *sinricpro_json_get_value(const cJSON *message) {
    if (!message) return NULL;

    const cJSON *payload = cJSON_GetObjectItem(message, "payload");
    if (!payload) return NULL;

    return cJSON_GetObjectItem(payload, "value");
}

const char *sinricpro_json_get_string(const cJSON *object, const char *key,
                                       const char *default_val) {
    if (!object || !key) return default_val;

    const cJSON *item = cJSON_GetObjectItem(object, key);
    if (!item || !cJSON_IsString(item)) return default_val;

    return item->valuestring;
}

int sinricpro_json_get_int(const cJSON *object, const char *key, int default_val) {
    if (!object || !key) return default_val;

    const cJSON *item = cJSON_GetObjectItem(object, key);
    if (!item || !cJSON_IsNumber(item)) return default_val;

    return item->valueint;
}

double sinricpro_json_get_double(const cJSON *object, const char *key,
                                  double default_val) {
    if (!object || !key) return default_val;

    const cJSON *item = cJSON_GetObjectItem(object, key);
    if (!item || !cJSON_IsNumber(item)) return default_val;

    return item->valuedouble;
}

bool sinricpro_json_get_bool(const cJSON *object, const char *key, bool default_val) {
    if (!object || !key) return default_val;

    const cJSON *item = cJSON_GetObjectItem(object, key);
    if (!item) return default_val;

    if (cJSON_IsBool(item)) {
        return cJSON_IsTrue(item);
    }

    // Also handle string "On"/"Off" for power state
    if (cJSON_IsString(item)) {
        const char *str = item->valuestring;
        if (strcasecmp(str, "On") == 0 || strcasecmp(str, "true") == 0) {
            return true;
        }
        if (strcasecmp(str, "Off") == 0 || strcasecmp(str, "false") == 0) {
            return false;
        }
    }

    return default_val;
}

const char *sinricpro_json_get_action(const cJSON *message) {
    const cJSON *payload = cJSON_GetObjectItem(message, "payload");
    return sinricpro_json_get_string(payload, "action", NULL);
}

const char *sinricpro_json_get_device_id(const cJSON *message) {
    const cJSON *payload = cJSON_GetObjectItem(message, "payload");
    return sinricpro_json_get_string(payload, "deviceId", NULL);
}

const char *sinricpro_json_get_type(const cJSON *message) {
    const cJSON *payload = cJSON_GetObjectItem(message, "payload");
    return sinricpro_json_get_string(payload, "type", NULL);
}

const char *sinricpro_json_get_reply_token(const cJSON *message) {
    const cJSON *payload = cJSON_GetObjectItem(message, "payload");
    return sinricpro_json_get_string(payload, "replyToken", NULL);
}

const char *sinricpro_json_get_signature(const cJSON *message) {
    const cJSON *signature = cJSON_GetObjectItem(message, "signature");
    return sinricpro_json_get_string(signature, "HMAC", NULL);
}

bool sinricpro_json_set_signature(cJSON *message, const char *signature) {
    if (!message || !signature) return false;

    cJSON *sig_obj = cJSON_GetObjectItem(message, "signature");
    if (!sig_obj) {
        sig_obj = cJSON_CreateObject();
        if (!sig_obj) return false;
        cJSON_AddItemToObject(message, "signature", sig_obj);
    }

    cJSON *hmac = cJSON_GetObjectItem(sig_obj, "HMAC");
    if (hmac) {
        cJSON_SetValuestring(hmac, signature);
    } else {
        cJSON_AddStringToObject(sig_obj, "HMAC", signature);
    }

    return true;
}

size_t sinricpro_json_serialize(const cJSON *json, char *output, size_t output_len) {
    if (!json || !output || output_len == 0) return 0;

    // Use unformatted print (no whitespace)
    char *str = cJSON_PrintUnformatted(json);
    if (!str) return 0;

    size_t len = strlen(str);
    if (len >= output_len) {
        free(str);
        return 0;
    }

    strcpy(output, str);
    free(str);

    return len;
}

size_t sinricpro_json_serialize_payload(const cJSON *message, char *output,
                                        size_t output_len) {
    if (!message || !output || output_len == 0) return 0;

    const cJSON *payload = cJSON_GetObjectItem(message, "payload");
    if (!payload) return 0;

    return sinricpro_json_serialize(payload, output, output_len);
}

bool sinricpro_json_generate_uuid(char *output, size_t output_len) {
    if (!output || output_len < 37) return false;

    // Generate random bytes using Pico SDK random
    uint8_t bytes[16];
    for (int i = 0; i < 16; i++) {
        bytes[i] = (uint8_t)(get_rand_32() & 0xFF);
    }

    // Set version to 4 (random)
    bytes[6] = (bytes[6] & 0x0F) | 0x40;
    // Set variant to RFC4122
    bytes[8] = (bytes[8] & 0x3F) | 0x80;

    // Format as UUID string
    snprintf(output, output_len,
             "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
             bytes[0], bytes[1], bytes[2], bytes[3],
             bytes[4], bytes[5],
             bytes[6], bytes[7],
             bytes[8], bytes[9],
             bytes[10], bytes[11], bytes[12], bytes[13], bytes[14], bytes[15]);

    return true;
}

uint32_t sinricpro_json_get_timestamp(void) {
    // Get milliseconds since boot and convert to seconds
    uint32_t seconds_since_boot = to_ms_since_boot(get_absolute_time()) / 1000;

    // Add offset from NTP (if set)
    return timestamp_offset + seconds_since_boot;
}

// Function to set timestamp offset (called when NTP sync occurs)
void sinricpro_json_set_timestamp_offset(uint32_t unix_time) {
    uint32_t seconds_since_boot = to_ms_since_boot(get_absolute_time()) / 1000;
    timestamp_offset = unix_time - seconds_since_boot;
}
