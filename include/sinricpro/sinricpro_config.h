/**
 * @file sinricpro_config.h
 * @brief SinricPro SDK configuration constants
 */

#ifndef SINRICPRO_CONFIG_H
#define SINRICPRO_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// Version Information
// =============================================================================
#define SINRICPRO_SDK_VERSION_MAJOR     1
#define SINRICPRO_SDK_VERSION_MINOR     0
#define SINRICPRO_SDK_VERSION_PATCH     0
#define SINRICPRO_SDK_VERSION           "1.0.0"
#define SINRICPRO_PLATFORM              "PICO_W"

// =============================================================================
// Server Configuration
// =============================================================================
#define SINRICPRO_SERVER_URL            "ws.sinric.pro"

// Check if user defined SINRICPRO_NOSSL in their sketch
#ifdef SINRICPRO_NOSSL
    // Non-secure mode (HTTP/WS) - For testing or low memory devices
    #define SINRICPRO_SERVER_PORT       80
    #define SINRICPRO_SERVER_USE_SSL    0
#else
    // Secure mode (HTTPS/WSS) - Default for production
    #define SINRICPRO_SERVER_PORT       443
    #define SINRICPRO_SERVER_USE_SSL    1
#endif

// =============================================================================
// WebSocket Configuration
// =============================================================================
#define SINRICPRO_WEBSOCKET_PING_INTERVAL_MS    300000  // 5 minutes
#define SINRICPRO_WEBSOCKET_PING_TIMEOUT_MS     10000   // 10 seconds
#define SINRICPRO_WEBSOCKET_RECONNECT_DELAY_MS  5000    // 5 seconds
#define SINRICPRO_WEBSOCKET_BUFFER_SIZE         2048

// =============================================================================
// Message Queue Configuration
// =============================================================================
#define SINRICPRO_MESSAGE_QUEUE_SIZE    8
#define SINRICPRO_MAX_MESSAGE_SIZE      2048

// =============================================================================
// Device Configuration
// =============================================================================
#define SINRICPRO_MAX_DEVICES           8
#define SINRICPRO_DEVICE_ID_LENGTH      24

// =============================================================================
// Event Limiter Configuration
// =============================================================================
#define SINRICPRO_EVENT_LIMIT_STATE_MS          1000    // 1 second for state events
#define SINRICPRO_EVENT_LIMIT_SENSOR_MS         60000   // 60 seconds for sensor values

// =============================================================================
// Signature Configuration
// =============================================================================
#define SINRICPRO_SIGNATURE_VERSION     1
#define SINRICPRO_PAYLOAD_VERSION       2

// =============================================================================
// Cause Types
// =============================================================================
#define SINRICPRO_CAUSE_PHYSICAL        "PHYSICAL_INTERACTION"
#define SINRICPRO_CAUSE_PERIODIC        "PERIODIC_POLL"
#define SINRICPRO_CAUSE_ALERT           "ALERT"

// =============================================================================
// Message Types
// =============================================================================
#define SINRICPRO_TYPE_REQUEST          "request"
#define SINRICPRO_TYPE_RESPONSE         "response"
#define SINRICPRO_TYPE_EVENT            "event"

// =============================================================================
// Scope Types
// =============================================================================
#define SINRICPRO_SCOPE_DEVICE          "device"
#define SINRICPRO_SCOPE_MODULE          "module"

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_CONFIG_H
