/**
 * @file sinricpro.h
 * @brief Main SinricPro SDK header for Raspberry Pi Pico W
 *
 * This is the main include file for the SinricPro SDK. It provides
 * functions for connecting to the SinricPro service, managing devices,
 * and handling smart home commands.
 *
 * @example
 * @code
 * #include "sinricpro/sinricpro.h"
 * #include "sinricpro/sinricpro_switch.h"
 *
 * sinricpro_switch_t my_switch;
 *
 * bool on_power_state(sinricpro_device_t *device, bool *state) {
 *     printf("Power: %s\n", *state ? "ON" : "OFF");
 *     gpio_put(LED_PIN, *state);
 *     return true;
 * }
 *
 * int main() {
 *     // Connect to WiFi first
 *     cyw43_arch_init();
 *     cyw43_arch_enable_sta_mode();
 *     cyw43_arch_wifi_connect_timeout_ms("YourNetwork", "YourPassword",
 *                                         CYW43_AUTH_WPA2_AES_PSK, 30000);
 *
 *     // Initialize SinricPro
 *     sinricpro_config_t config = {
 *         .app_key = "your-app-key",
 *         .app_secret = "your-app-secret"
 *     };
 *
 *     sinricpro_init(&config);
 *     sinricpro_switch_init(&my_switch, "your-device-id");
 *     sinricpro_switch_on_power_state(&my_switch, on_power_state);
 *     sinricpro_add_device((sinricpro_device_t *)&my_switch);
 *     sinricpro_begin();
 *
 *     while (1) {
 *         sinricpro_handle();
 *     }
 * }
 * @endcode
 */

#ifndef SINRICPRO_H
#define SINRICPRO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "sinricpro_config.h"
#include "sinricpro_device.h"

/**
 * @brief Connection state
 */
typedef enum {
    SINRICPRO_STATE_DISCONNECTED = 0,
    SINRICPRO_STATE_WIFI_CONNECTING,
    SINRICPRO_STATE_WIFI_CONNECTED,
    SINRICPRO_STATE_WS_CONNECTING,
    SINRICPRO_STATE_CONNECTED,
    SINRICPRO_STATE_ERROR
} sinricpro_state_t;

/**
 * @brief SDK configuration structure
 *
 * NOTE: WiFi connection must be established before calling sinricpro_begin().
 * Use cyw43_arch_wifi_connect_timeout_ms() to connect to WiFi first.
 *
 * SSL/TLS Mode:
 * - Set use_ssl = true for secure WebSocket (wss://) on port 443 (default)
 * - Set use_ssl = false for plain WebSocket (ws://) on port 80
 * - Tip: Define SINRICPRO_NOSSL at the top of your sketch to default to non-secure mode
 */
typedef struct {
    // Credentials (required)
    const char *app_key;
    const char *app_secret;

    // Server configuration (optional, uses defaults)
    const char *server_url;      // Default: ws.sinric.pro
    uint16_t server_port;        // Default: 443 if use_ssl=true, 80 if use_ssl=false
    bool use_ssl;                // true = port 443 (secure), false = port 80 (non-secure)

    // Connection settings (optional)
    uint32_t connect_timeout_ms;     // Default: 30000
    uint32_t ping_interval_ms;       // Default: 300000 (5 min)
    uint32_t reconnect_delay_ms;     // Default: 5000

    // Debug settings (optional)
    bool enable_debug;               // Enable WebSocket message logging (default: false)
} sinricpro_config_t;

/**
 * @brief Connection state change callback
 */
typedef void (*sinricpro_state_callback_t)(sinricpro_state_t state, void *user_data);

/**
 * @brief Initialize SinricPro SDK
 *
 * Must be called before any other SinricPro functions.
 *
 * @param config Configuration structure
 * @return true on success, false on failure
 */
bool sinricpro_init(const sinricpro_config_t *config);

/**
 * @brief Start SinricPro connection
 *
 * Connects to WiFi and then to the SinricPro server.
 * This function is non-blocking; use sinricpro_handle() in your main loop.
 *
 * @return true if connection started, false on failure
 */
bool sinricpro_begin(void);

/**
 * @brief Process SinricPro events
 *
 * Must be called frequently (ideally every loop iteration) to:
 * - Maintain WiFi and WebSocket connections
 * - Process incoming messages
 * - Send queued events
 * - Handle keepalive pings
 */
void sinricpro_handle(void);

/**
 * @brief Disconnect from SinricPro
 *
 * Closes WebSocket connection. WiFi remains connected.
 */
void sinricpro_disconnect(void);

/**
 * @brief Stop SinricPro completely
 *
 * Disconnects WebSocket and WiFi.
 */
void sinricpro_stop(void);

/**
 * @brief Add a device to SinricPro
 *
 * @param device Device to add (must remain valid for SDK lifetime)
 * @return true on success, false if max devices reached
 */
bool sinricpro_add_device(sinricpro_device_t *device);

/**
 * @brief Remove a device from SinricPro
 *
 * @param device_id Device ID to remove
 * @return true if found and removed, false otherwise
 */
bool sinricpro_remove_device(const char *device_id);

/**
 * @brief Find a device by ID
 *
 * @param device_id Device ID to find
 * @return Device pointer, or NULL if not found
 */
sinricpro_device_t *sinricpro_find_device(const char *device_id);

/**
 * @brief Get number of registered devices
 *
 * @return Number of devices
 */
size_t sinricpro_device_count(void);

/**
 * @brief Get current connection state
 *
 * @return Current state
 */
sinricpro_state_t sinricpro_get_state(void);

/**
 * @brief Check if connected to SinricPro server
 *
 * @return true if connected
 */
bool sinricpro_is_connected(void);

/**
 * @brief Set state change callback
 *
 * @param callback Callback function
 * @param user_data User data passed to callback
 */
void sinricpro_on_state_change(sinricpro_state_callback_t callback, void *user_data);

/**
 * @brief Send a raw event message
 *
 * Typically use device-specific event functions instead.
 *
 * @param device_id Device ID
 * @param action Event action name
 * @param value_json JSON value object (will be added to message)
 * @return true if queued successfully
 */
bool sinricpro_send_event(const char *device_id, const char *action, cJSON *value_json);

/**
 * @brief Get SDK version string
 *
 * @return Version string (e.g., "1.0.0")
 */
const char *sinricpro_get_version(void);

/**
 * @brief Get platform identifier
 *
 * @return Platform string (e.g., "PICO_W")
 */
const char *sinricpro_get_platform(void);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_H
