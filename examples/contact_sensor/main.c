/**
 * @file main.c
 * @brief SinricPro Contact Sensor Example for Raspberry Pi Pico W
 *
 * This example demonstrates how to create a door/window contact sensor
 * that detects open/closed states and sends events to SinricPro.
 *
 * Hardware:
 * - Raspberry Pi Pico W
 * - Magnetic reed switch or door sensor connected to GPIO 15
 *   (Normally closed when door/window is closed)
 *
 * Alexa Usage:
 * - "Alexa, is the [door/window name] open?"
 * - Create routines: "When [door/window] opens, turn on lights"
 * - Get notifications when doors/windows open
 *
 * Connection Mode:
 * - Default: Secure mode (WSS on port 443) with TLS encryption
 * - Low Memory: Uncomment the line below to use non-secure mode (WS on port 80)
 */

// Uncomment this line to use non-secure WebSocket (port 80) for low memory devices
// #define SINRICPRO_NOSSL

// Uncomment the following line to enable/disable sdk debug output
// #define ENABLE_DEBUG

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/gpio.h"

#include "sinricpro/sinricpro.h"
#include "sinricpro/sinricpro_contact_sensor.h"

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

#define CONTACT_PIN     15  // GPIO for contact sensor (reed switch)
#define DEBOUNCE_MS     100  // Debounce time

// Set to true if sensor is normally open (open = high), false if normally closed
#define NORMALLY_OPEN   false

// =============================================================================
// Global Variables
// =============================================================================

static sinricpro_contact_sensor_t my_contact_sensor;
static bool last_contact_state = false;  // Track state changes
static uint32_t last_change_time = 0;

// =============================================================================
// Hardware Functions
// =============================================================================

/**
 * @brief Initialize GPIO pins
 */
void init_hardware(void) {
    // Initialize contact sensor input
    gpio_init(CONTACT_PIN);
    gpio_set_dir(CONTACT_PIN, GPIO_IN);

    if (NORMALLY_OPEN) {
        gpio_pull_down(CONTACT_PIN);  // Pull down for normally open sensor
    } else {
        gpio_pull_up(CONTACT_PIN);    // Pull up for normally closed sensor
    }

    // Read initial state
    last_contact_state = gpio_get(CONTACT_PIN);
    if (!NORMALLY_OPEN) {
        last_contact_state = !last_contact_state;  // Invert for normally closed
    }
}

/**
 * @brief Check contact sensor state with debouncing
 *
 * @param is_open Output: true if contact is open (door/window open)
 * @return true if state changed
 */
bool check_contact_state(bool *is_open) {
    bool current_state = gpio_get(CONTACT_PIN);

    // Invert logic for normally closed sensors
    if (!NORMALLY_OPEN) {
        current_state = !current_state;
    }

    uint32_t now = to_ms_since_boot(get_absolute_time());

    // Check if state changed and debounce period passed
    if (current_state != last_contact_state &&
        (now - last_change_time > DEBOUNCE_MS)) {

        last_change_time = now;
        last_contact_state = current_state;
        *is_open = current_state;
        return true;  // State changed
    }

    return false;  // No change
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
    printf("SinricPro Contact Sensor Example\n");
    printf("Door/Window Sensor Monitor\n");
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
        .enable_debug = false
    };

    if (!sinricpro_init(&config)) {
        printf("ERROR: Failed to initialize SinricPro\n");
        return 1;
    }

    // Register state change callback
    sinricpro_on_state_change(on_state_change, NULL);

    // Initialize contact sensor device
    if (!sinricpro_contact_sensor_init(&my_contact_sensor, DEVICE_ID)) {
        printf("ERROR: Failed to initialize contact sensor device\n");
        return 1;
    }

    // Note: Contact sensors are event-only devices, no callbacks needed

    // Add device to SinricPro
    if (!sinricpro_add_device((sinricpro_device_t *)&my_contact_sensor)) {
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
    printf("Ready! Contact sensor is monitoring.\n");
    printf("\n");
    printf("Voice Commands:\n");
    printf("  'Alexa, is the [door/window] open?'\n");
    printf("\n");
    printf("Create Routines:\n");
    printf("  When [door/window] opens -> Turn on lights\n");
    printf("  When [door/window] closes -> Turn off lights\n");
    printf("================================================\n\n");

    // Initialize hardware
    init_hardware();

    printf("[Contact] Initial state: %s\n",
           last_contact_state ? "OPEN" : "CLOSED");

    // Send initial state when connected
    bool initial_state_sent = false;

    // Main loop
    while (1) {
        // Process SinricPro events
        sinricpro_handle();

        // Send initial state once connected
        if (!initial_state_sent && sinricpro_is_connected()) {
            sinricpro_contact_sensor_send_event(&my_contact_sensor, last_contact_state);
            initial_state_sent = true;
            printf("[Contact] Initial state sent to server\n");
        }

        // Check for state changes
        bool is_open;
        if (check_contact_state(&is_open)) {
            printf("[Contact] State changed: %s\n", is_open ? "OPEN" : "CLOSED");

            // Send event to SinricPro
            if (sinricpro_is_connected()) {
                if (sinricpro_contact_sensor_send_event(&my_contact_sensor, is_open)) {
                    printf("[Contact] Event sent successfully\n");
                } else {
                    printf("[Contact] Failed to send event (rate limited)\n");
                }
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
