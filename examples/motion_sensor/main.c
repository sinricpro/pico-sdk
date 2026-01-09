/**
 * @file main.c
 * @brief SinricPro Motion Sensor Example for Raspberry Pi Pico W
 *
 * This example demonstrates how to create a motion sensor device
 * that sends motion detection events to SinricPro, triggering
 * Alexa routines and notifications.
 *
 * Hardware:
 * - Raspberry Pi Pico W
 * - PIR motion sensor connected to GPIO 15
 *
 * Alexa Setup:
 * 1. Create a routine in Alexa app
 * 2. Set trigger: "When [motion sensor name] detects motion"
 * 3. Add action: Turn on lights, send notification, etc.
 *
 * Connection Mode:
 * - Default: Secure mode (WSS on port 443) with TLS encryption
 * - Low Memory: Uncomment the line below to use non-secure mode (WS on port 80)
 */

// Uncomment this line to use non-secure WebSocket (port 80) for low memory devices
#define SINRICPRO_NOSSL

// Uncomment the following line to enable/disable sdk debug output
// #define ENABLE_DEBUG

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/gpio.h"

#include "sinricpro/sinricpro.h"
#include "sinricpro/sinricpro_motion_sensor.h"

// =============================================================================
// Configuration - UPDATE THESE VALUES
// =============================================================================

#define WIFI_SSID       "YOUR_WIFI_SSID"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"

// Get these from https://sinric.pro
#define APP_KEY         "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
#define APP_SECRET      "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx-xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
#define DEVICE_ID       "xxxxxxxxxxxxxxxxxxxxxxxx"  // 24-character device ID

// =============================================================================
// Hardware Configuration
// =============================================================================

#define PIR_PIN         15  // GPIO for PIR sensor input
#define DEBOUNCE_MS     2000  // Minimum time between motion events (2 seconds)

// =============================================================================
// Global Variables
// =============================================================================

static sinricpro_motion_sensor_t my_motion_sensor;
static uint32_t last_motion_time = 0;

// =============================================================================
// Hardware Functions
// =============================================================================

/**
 * @brief Initialize GPIO pins
 */
void init_hardware(void) {
    // Initialize PIR sensor input with pull-down
    gpio_init(PIR_PIN);
    gpio_set_dir(PIR_PIN, GPIO_IN);
    gpio_pull_down(PIR_PIN);
}

/**
 * @brief Check for motion with debouncing
 *
 * @return true if motion detected (with debounce)
 */
bool check_motion(void) {
    bool pir_state = gpio_get(PIR_PIN);
    uint32_t now = to_ms_since_boot(get_absolute_time());

    // Motion detected (PIR output high) and debounce period passed
    if (pir_state && (now - last_motion_time > DEBOUNCE_MS)) {
        last_motion_time = now;
        return true;
    }

    return false;
}

// =============================================================================
// Callbacks
// =============================================================================

/**
 * @brief Connection state change callback
 */
void on_state_change(sinricpro_state_t state, void *user_data) {
    const char *state_str = "";
    switch (state) {
        case SINRICPRO_STATE_DISCONNECTED: state_str = "DISCONNECTED"; break;
        case SINRICPRO_STATE_WIFI_CONNECTING: state_str = "WIFI_CONNECTING"; break;
        case SINRICPRO_STATE_WIFI_CONNECTED: state_str = "WIFI_CONNECTED"; break;
        case SINRICPRO_STATE_WS_CONNECTING: state_str = "WS_CONNECTING"; break;
        case SINRICPRO_STATE_CONNECTED: state_str = "CONNECTED"; break;
        case SINRICPRO_STATE_ERROR: state_str = "ERROR"; break;
        default: state_str = "UNKNOWN"; break;
    }

    printf("[SinricPro] State: %s\n", state_str);
}

// =============================================================================
// Main Program
// =============================================================================

int main() {
    // Initialize stdio for USB serial output
    stdio_init_all();

    // Wait a moment for USB serial to be ready
    sleep_ms(2000);

    printf("\n");
    printf("================================================\n");
    printf("SinricPro Motion Sensor Example\n");
    printf("================================================\n\n");

    // =============================================================================
    // Step 1: Initialize WiFi
    // =============================================================================

    printf("[1/4] Initializing WiFi...\n");
    if (cyw43_arch_init()) {
        printf("ERROR: Failed to initialize WiFi\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();

    printf("[2/4] Connecting to WiFi SSID: %s\n", WIFI_SSID);
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD,
                                            CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("ERROR: Failed to connect to WiFi\n");
        return 1;
    }

    printf("WiFi connected! IP: %s\n",
           ip4addr_ntoa(netif_ip4_addr(netif_list)));

    // =============================================================================
    // Step 2: Initialize SinricPro and Device
    // =============================================================================

    printf("[3/4] Initializing SinricPro...\n");

    // Configure SinricPro
    sinricpro_config_t config = {
        .app_key = APP_KEY,
        .app_secret = APP_SECRET,
#ifdef SINRICPRO_NOSSL
        .use_ssl = false,  // Non-secure mode (port 80)
#else
        .use_ssl = true,   // Secure mode (port 443) - default
#endif

#ifdef ENABLE_DEBUG
        .enable_debug = true
#else
        .enable_debug = false
#endif
    };

    if (!sinricpro_init(&config)) {
        printf("ERROR: Failed to initialize SinricPro\n");
        return 1;
    }

    // Register state change callback
    sinricpro_on_state_change(on_state_change, NULL);

    // Initialize motion sensor device
    if (!sinricpro_motion_sensor_init(&my_motion_sensor, DEVICE_ID)) {
        printf("ERROR: Failed to initialize motion sensor device\n");
        return 1;
    }

    // Note: Motion sensors are event-only devices, no callbacks needed

    // Add device to SinricPro
    if (!sinricpro_add_device((sinricpro_device_t *)&my_motion_sensor)) {
        printf("ERROR: Failed to add device\n");
        return 1;
    }

    // =============================================================================
    // Step 3: Connect to SinricPro Server
    // =============================================================================

    printf("[4/4] Connecting to SinricPro...\n");
    if (!sinricpro_begin()) {
        printf("ERROR: Failed to connect to SinricPro\n");
        return 1;
    }

    printf("\n");
    printf("================================================\n");
    printf("Ready! Motion sensor is monitoring.\n");
    printf("\n");
    printf("When motion is detected:\n");
    printf("  - Event sent to SinricPro server\n");
    printf("  - Can trigger Alexa routines\n");
    printf("  - Visible in SinricPro app\n");
    printf("================================================\n\n");

    // Initialize hardware
    init_hardware();

    // Main loop
    while (1) {
        // Process SinricPro events
        sinricpro_handle();

        // Check for motion
        if (check_motion()) {
            printf("[Motion] DETECTED! Sending event...\n");

            // Send motion detected event
            if (sinricpro_motion_sensor_send_event(&my_motion_sensor, true)) {
                printf("[Motion] Event sent successfully\n");
            } else {
                printf("[Motion] Failed to send event (rate limited or not connected)\n");
            }
        }

        // Blink onboard LED when connected
        static uint32_t last_blink = 0;
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - last_blink > 1000) {
            last_blink = now;
            if (sinricpro_is_connected()) {
                cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
                sleep_ms(50);
                cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
            }
        }

        // Small delay to prevent CPU hogging
        sleep_ms(10);
    }

    return 0;
}
