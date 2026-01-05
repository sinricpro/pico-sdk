/**
 * @file sinricpro.c
 * @brief Main SinricPro SDK implementation
 */

#include "sinricpro/sinricpro.h"
#include "sinricpro/sinricpro_config.h"
#include "core/websocket_client.h"
#include "core/message_queue.h"
#include "core/signature.h"
#include "core/json_helpers.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "cJSON.h"

// SDK state
typedef struct {
    sinricpro_config_t config;
    sinricpro_state_t state;

    // Device registry
    sinricpro_device_t *devices[SINRICPRO_MAX_DEVICES];
    size_t device_count;

    // Message queue
    sinricpro_queue_t rx_queue;
    sinricpro_queue_t tx_queue;

    // Callbacks
    sinricpro_state_callback_t state_callback;
    void *state_callback_data;

    // Connection state
    bool wifi_connected;
    uint32_t last_connect_attempt;

    // Device ID list for WebSocket headers
    char device_ids_header[SINRICPRO_MAX_DEVICES * (SINRICPRO_DEVICE_ID_LENGTH + 1) + 1];

} sinricpro_ctx_t;

static sinricpro_ctx_t ctx;
static bool sdk_initialized = false;

// Forward declarations
static void on_ws_message(const char *message, size_t length, void *user_data);
static void on_ws_state(sinricpro_ws_state_t state, void *user_data);
static void process_incoming_message(const char *message, size_t length);
static void process_request(cJSON *message);
static bool send_message(cJSON *message);
static void update_device_ids_header(void);
static void set_state(sinricpro_state_t new_state);

bool sinricpro_init(const sinricpro_config_t *config) {
    if (!config || !config->app_key || !config->app_secret ||
        !config->wifi_ssid || !config->wifi_password) {
        printf("[SinricPro] Invalid configuration\n");
        return false;
    }

    // Initialize Pico stdio
    stdio_init_all();

    // Initialize cyw43 (WiFi driver)
    if (cyw43_arch_init()) {
        printf("[SinricPro] Failed to initialize cyw43\n");
        return false;
    }

    // Enable station mode
    cyw43_arch_enable_sta_mode();

    // Store configuration
    memset(&ctx, 0, sizeof(ctx));
    memcpy(&ctx.config, config, sizeof(sinricpro_config_t));

    // Apply defaults
    if (!ctx.config.server_url) {
        ctx.config.server_url = SINRICPRO_SERVER_URL;
    }
    if (ctx.config.server_port == 0) {
        ctx.config.server_port = SINRICPRO_SERVER_PORT;
    } 

    ctx.config.use_ssl = config->use_ssl;

    if (ctx.config.connect_timeout_ms == 0) {
        ctx.config.connect_timeout_ms = 30000;
    }
    if (ctx.config.ping_interval_ms == 0) {
        ctx.config.ping_interval_ms = SINRICPRO_WEBSOCKET_PING_INTERVAL_MS;
    }
    if (ctx.config.reconnect_delay_ms == 0) {
        ctx.config.reconnect_delay_ms = SINRICPRO_WEBSOCKET_RECONNECT_DELAY_MS;
    }

    // Initialize queues
    sinricpro_queue_init(&ctx.rx_queue);
    sinricpro_queue_init(&ctx.tx_queue);

    // Initialize WebSocket client
    sinricpro_ws_init();

    ctx.state = SINRICPRO_STATE_DISCONNECTED;
    sdk_initialized = true;

    printf("[SinricPro] SDK v%s initialized\n", SINRICPRO_SDK_VERSION);
    return true;
}

bool sinricpro_begin(void) {
    if (!sdk_initialized) {
        printf("[SinricPro] SDK not initialized\n");
        return false;
    }

    if (ctx.device_count == 0) {
        printf("[SinricPro] No devices registered\n");
        return false;
    }

    // Update device IDs header
    update_device_ids_header();

    // Connect to WiFi
    set_state(SINRICPRO_STATE_WIFI_CONNECTING);
    printf("[SinricPro] Connecting to WiFi: %s\n", ctx.config.wifi_ssid);

    int result = cyw43_arch_wifi_connect_timeout_ms(
        ctx.config.wifi_ssid,
        ctx.config.wifi_password,
        CYW43_AUTH_WPA2_AES_PSK,
        ctx.config.connect_timeout_ms);

    if (result != 0) {
        printf("[SinricPro] WiFi connection failed: %d\n", result);
        set_state(SINRICPRO_STATE_ERROR);
        return false;
    }

    ctx.wifi_connected = true;
    set_state(SINRICPRO_STATE_WIFI_CONNECTED);
    printf("[SinricPro] WiFi connected\n");

    // Connect WebSocket
    set_state(SINRICPRO_STATE_WS_CONNECTING);

    sinricpro_ws_config_t ws_config = {
        .host = ctx.config.server_url,
        .port = ctx.config.server_port,
        .path = "/",
        .use_ssl = ctx.config.use_ssl,
        .app_key = ctx.config.app_key,
        .device_ids = ctx.device_ids_header,
        .platform = SINRICPRO_PLATFORM,
        .sdk_version = SINRICPRO_SDK_VERSION,
        .on_message = on_ws_message,
        .on_state_change = on_ws_state,
        .user_data = NULL,
        .connect_timeout_ms = ctx.config.connect_timeout_ms,
        .ping_interval_ms = ctx.config.ping_interval_ms,
        .ping_timeout_ms = SINRICPRO_WEBSOCKET_PING_TIMEOUT_MS
    };

    return sinricpro_ws_connect(&ws_config);
}

void sinricpro_handle(void) {
    if (!sdk_initialized) return;

    // Handle WebSocket
    sinricpro_ws_handle();

    // Process received messages
    char message[SINRICPRO_MAX_MESSAGE_SIZE];
    size_t length;
    sinricpro_interface_t interface;

    while (sinricpro_queue_pop(&ctx.rx_queue, &interface, message,
                               sizeof(message), &length)) {
        process_incoming_message(message, length);
    }

    // Send queued messages
    if (sinricpro_ws_is_connected()) {
        while (sinricpro_queue_pop(&ctx.tx_queue, &interface, message,
                                   sizeof(message), &length)) {
            sinricpro_ws_send(message, length);
        }
    }
}

void sinricpro_disconnect(void) {
    sinricpro_ws_disconnect();
    set_state(SINRICPRO_STATE_WIFI_CONNECTED);
}

void sinricpro_stop(void) {
    sinricpro_disconnect();
    cyw43_arch_deinit();
    ctx.wifi_connected = false;
    set_state(SINRICPRO_STATE_DISCONNECTED);
}

bool sinricpro_add_device(sinricpro_device_t *device) {
    if (!device || ctx.device_count >= SINRICPRO_MAX_DEVICES) {
        return false;
    }

    // Check for duplicate
    for (size_t i = 0; i < ctx.device_count; i++) {
        if (strcmp(ctx.devices[i]->device_id, device->device_id) == 0) {
            printf("[SinricPro] Device %s already registered\n", device->device_id);
            return false;
        }
    }

    ctx.devices[ctx.device_count++] = device;
    printf("[SinricPro] Added device: %s\n", device->device_id);

    return true;
}

bool sinricpro_remove_device(const char *device_id) {
    if (!device_id) return false;

    for (size_t i = 0; i < ctx.device_count; i++) {
        if (strcmp(ctx.devices[i]->device_id, device_id) == 0) {
            // Shift remaining devices
            for (size_t j = i; j < ctx.device_count - 1; j++) {
                ctx.devices[j] = ctx.devices[j + 1];
            }
            ctx.device_count--;
            return true;
        }
    }

    return false;
}

sinricpro_device_t *sinricpro_find_device(const char *device_id) {
    if (!device_id) return NULL;

    for (size_t i = 0; i < ctx.device_count; i++) {
        if (strcmp(ctx.devices[i]->device_id, device_id) == 0) {
            return ctx.devices[i];
        }
    }

    return NULL;
}

size_t sinricpro_device_count(void) {
    return ctx.device_count;
}

sinricpro_state_t sinricpro_get_state(void) {
    return ctx.state;
}

bool sinricpro_is_connected(void) {
    return ctx.state == SINRICPRO_STATE_CONNECTED;
}

void sinricpro_on_state_change(sinricpro_state_callback_t callback, void *user_data) {
    ctx.state_callback = callback;
    ctx.state_callback_data = user_data;
}

bool sinricpro_send_event(const char *device_id, const char *action, cJSON *value_json) {
    if (!device_id || !action) return false;

    // Create event message
    cJSON *event = sinricpro_json_create_event(device_id, action);
    if (!event) return false;

    // Add value
    if (value_json) {
        cJSON *payload = cJSON_GetObjectItem(event, "payload");
        if (payload) {
            cJSON_DeleteItemFromObject(payload, "value");
            cJSON_AddItemToObject(payload, "value", value_json);
        }
    }

    bool result = send_message(event);
    cJSON_Delete(event);

    return result;
}

const char *sinricpro_get_version(void) {
    return SINRICPRO_SDK_VERSION;
}

const char *sinricpro_get_platform(void) {
    return SINRICPRO_PLATFORM;
}

// ============================================================================
// Internal Functions
// ============================================================================

static void set_state(sinricpro_state_t new_state) {
    if (ctx.state != new_state) {
        ctx.state = new_state;

        if (ctx.state_callback) {
            ctx.state_callback(new_state, ctx.state_callback_data);
        }
    }
}

static void update_device_ids_header(void) {
    ctx.device_ids_header[0] = '\0';

    for (size_t i = 0; i < ctx.device_count; i++) {
        if (i > 0) {
            strcat(ctx.device_ids_header, ";");
        }
        strcat(ctx.device_ids_header, ctx.devices[i]->device_id);
    }
}

static void on_ws_message(const char *message, size_t length, void *user_data) {
    // Queue message for processing
    sinricpro_queue_push(&ctx.rx_queue, SINRICPRO_IF_WEBSOCKET, message, length);
}

static void on_ws_state(sinricpro_ws_state_t ws_state, void *user_data) {
    switch (ws_state) {
        case WS_STATE_CONNECTED:
            set_state(SINRICPRO_STATE_CONNECTED);
            printf("[SinricPro] Connected to server\n");
            break;

        case WS_STATE_DISCONNECTED:
        case WS_STATE_ERROR:
            if (ctx.wifi_connected) {
                set_state(SINRICPRO_STATE_WIFI_CONNECTED);
            } else {
                set_state(SINRICPRO_STATE_DISCONNECTED);
            }
            break;

        default:
            break;
    }
}

static void process_incoming_message(const char *message, size_t length) {
    // Parse JSON
    cJSON *json = cJSON_ParseWithLength(message, length);
    if (!json) {
        printf("[SinricPro] Failed to parse message\n");
        return;
    }

    // Verify signature
    const char *signature = sinricpro_json_get_signature(json);
    if (!signature || !sinricpro_verify_signature(ctx.config.app_secret,
                                                   message, signature)) {
        printf("[SinricPro] Invalid signature\n");
        cJSON_Delete(json);
        return;
    }

    // Get message type
    const char *type = sinricpro_json_get_type(json);
    if (!type) {
        cJSON_Delete(json);
        return;
    }

    if (strcmp(type, SINRICPRO_TYPE_REQUEST) == 0) {
        process_request(json);
    }
    // Response and event types are typically not received from server

    cJSON_Delete(json);
}

static void process_request(cJSON *message) {
    const char *device_id = sinricpro_json_get_device_id(message);
    const char *action = sinricpro_json_get_action(message);

    if (!device_id || !action) {
        printf("[SinricPro] Invalid request: missing deviceId or action\n");
        return;
    }

    printf("[SinricPro] Request: %s -> %s\n", device_id, action);

    // Find device
    sinricpro_device_t *device = sinricpro_find_device(device_id);
    if (!device) {
        printf("[SinricPro] Device not found: %s\n", device_id);
        return;
    }

    // Create response
    cJSON *response = sinricpro_json_create_response(message, false);
    if (!response) {
        printf("[SinricPro] Failed to create response\n");
        return;
    }

    // Handle request via device's request handler
    bool success = false;
    if (device->handle_request) {
        success = device->handle_request(device, action, message, response);
    }

    // Update success flag in response
    cJSON *payload = cJSON_GetObjectItem(response, "payload");
    if (payload) {
        cJSON *success_item = cJSON_GetObjectItem(payload, "success");
        if (success_item) {
            cJSON_SetBoolValue(success_item, success);
        }
    }

    // Send response
    send_message(response);
    cJSON_Delete(response);
}

static bool send_message(cJSON *message) {
    if (!message) return false;

    // Serialize payload for signing
    char payload_str[SINRICPRO_MAX_MESSAGE_SIZE];
    size_t payload_len = sinricpro_json_serialize_payload(message, payload_str,
                                                          sizeof(payload_str));
    if (payload_len == 0) {
        printf("[SinricPro] Failed to serialize payload\n");
        return false;
    }

    // Calculate signature
    char signature[SINRICPRO_SIGNATURE_MAX_LEN];
    if (!sinricpro_calculate_signature(ctx.config.app_secret, payload_str,
                                       signature, sizeof(signature))) {
        printf("[SinricPro] Failed to calculate signature\n");
        return false;
    }

    // Set signature
    sinricpro_json_set_signature(message, signature);

    // Serialize complete message
    char message_str[SINRICPRO_MAX_MESSAGE_SIZE];
    size_t message_len = sinricpro_json_serialize(message, message_str,
                                                   sizeof(message_str));
    if (message_len == 0) {
        printf("[SinricPro] Failed to serialize message\n");
        return false;
    }

    // Queue for sending
    return sinricpro_queue_push(&ctx.tx_queue, SINRICPRO_IF_WEBSOCKET,
                                message_str, message_len);
}

// Device base implementation
bool sinricpro_device_init(sinricpro_device_t *device,
                           const char *device_id,
                           sinricpro_device_type_t type) {
    if (!device || !device_id) return false;

    if (strlen(device_id) != SINRICPRO_DEVICE_ID_LENGTH) {
        printf("[SinricPro] Invalid device ID length: %s\n", device_id);
        return false;
    }

    memset(device, 0, sizeof(sinricpro_device_t));
    strncpy(device->device_id, device_id, SINRICPRO_DEVICE_ID_LENGTH);
    device->device_id[SINRICPRO_DEVICE_ID_LENGTH] = '\0';
    device->type = type;

    return true;
}

const char *sinricpro_device_get_id(const sinricpro_device_t *device) {
    return device ? device->device_id : NULL;
}

sinricpro_device_type_t sinricpro_device_get_type(const sinricpro_device_t *device) {
    return device ? device->type : SINRICPRO_DEVICE_TYPE_UNKNOWN;
}

void sinricpro_device_set_user_data(sinricpro_device_t *device, void *user_data) {
    if (device) device->user_data = user_data;
}

void *sinricpro_device_get_user_data(const sinricpro_device_t *device) {
    return device ? device->user_data : NULL;
}
