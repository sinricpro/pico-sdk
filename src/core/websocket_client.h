/**
 * @file websocket_client.h
 * @brief WebSocket client for SinricPro with TLS support
 *
 * Implements RFC 6455 WebSocket protocol over TLS using lwIP and mbedTLS.
 * Designed for the Raspberry Pi Pico W.
 */

#ifndef SINRICPRO_WEBSOCKET_CLIENT_H
#define SINRICPRO_WEBSOCKET_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief WebSocket connection state
 */
typedef enum {
    WS_STATE_DISCONNECTED = 0,
    WS_STATE_DNS_LOOKUP,
    WS_STATE_TCP_CONNECTING,
    WS_STATE_TLS_HANDSHAKE,
    WS_STATE_WS_HANDSHAKE,
    WS_STATE_CONNECTED,
    WS_STATE_CLOSING,
    WS_STATE_ERROR
} sinricpro_ws_state_t;

/**
 * @brief WebSocket opcode types (RFC 6455)
 */
typedef enum {
    WS_OPCODE_CONTINUATION = 0x0,
    WS_OPCODE_TEXT         = 0x1,
    WS_OPCODE_BINARY       = 0x2,
    WS_OPCODE_CLOSE        = 0x8,
    WS_OPCODE_PING         = 0x9,
    WS_OPCODE_PONG         = 0xA
} sinricpro_ws_opcode_t;

/**
 * @brief Callback for receiving text messages
 *
 * @param message   The received message (null-terminated)
 * @param length    Message length
 * @param user_data User data pointer
 */
typedef void (*sinricpro_ws_message_callback_t)(const char *message,
                                                 size_t length,
                                                 void *user_data);

/**
 * @brief Callback for connection state changes
 *
 * @param state     New connection state
 * @param user_data User data pointer
 */
typedef void (*sinricpro_ws_state_callback_t)(sinricpro_ws_state_t state,
                                               void *user_data);

/**
 * @brief WebSocket client configuration
 */
typedef struct {
    const char *host;                   // Server hostname
    uint16_t port;                      // Server port (443 for WSS)
    const char *path;                   // Path (e.g., "/")
    bool use_ssl;                       // Use TLS/SSL

    // Custom headers (for SinricPro authentication)
    const char *app_key;                // SinricPro app key
    const char *device_ids;             // Semicolon-separated device IDs
    const char *platform;               // Platform identifier
    const char *sdk_version;            // SDK version string

    // Callbacks
    sinricpro_ws_message_callback_t on_message;
    sinricpro_ws_state_callback_t on_state_change;
    void *user_data;

    // Timeouts (in milliseconds)
    uint32_t connect_timeout_ms;
    uint32_t ping_interval_ms;
    uint32_t ping_timeout_ms;
} sinricpro_ws_config_t;

/**
 * @brief Initialize WebSocket client subsystem
 *
 * Must be called once before using any WebSocket functions.
 *
 * @return true on success, false on failure
 */
bool sinricpro_ws_init(void);

/**
 * @brief Connect to WebSocket server
 *
 * @param config    Connection configuration
 * @return true if connection initiated, false on immediate failure
 */
bool sinricpro_ws_connect(const sinricpro_ws_config_t *config);

/**
 * @brief Disconnect from WebSocket server
 *
 * Sends close frame and terminates connection gracefully.
 */
void sinricpro_ws_disconnect(void);

/**
 * @brief Process WebSocket events (must be called regularly)
 *
 * This function handles:
 * - Network I/O
 * - Message reception
 * - Ping/Pong keepalive
 * - Reconnection
 *
 * Call this function frequently in your main loop.
 */
void sinricpro_ws_handle(void);

/**
 * @brief Send a text message over WebSocket
 *
 * @param message   Message to send (null-terminated)
 * @param length    Message length (0 to auto-detect)
 * @return true on success, false on failure
 */
bool sinricpro_ws_send(const char *message, size_t length);

/**
 * @brief Send a ping frame
 *
 * @return true on success, false on failure
 */
bool sinricpro_ws_send_ping(void);

/**
 * @brief Get current connection state
 *
 * @return Current WebSocket state
 */
sinricpro_ws_state_t sinricpro_ws_get_state(void);

/**
 * @brief Check if WebSocket is connected
 *
 * @return true if connected and ready to send/receive
 */
bool sinricpro_ws_is_connected(void);

/**
 * @brief Get time since last successful ping/pong
 *
 * @return Milliseconds since last pong received
 */
uint32_t sinricpro_ws_get_last_pong_age(void);

/**
 * @brief Set reconnect behavior
 *
 * @param enabled   Enable automatic reconnection
 * @param delay_ms  Delay between reconnection attempts
 */
void sinricpro_ws_set_reconnect(bool enabled, uint32_t delay_ms);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_WEBSOCKET_CLIENT_H
