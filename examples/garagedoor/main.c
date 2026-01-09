/**
 * @file main.c
 * @brief SinricPro Garage Door Example for Raspberry Pi Pico W
 *
 * This example demonstrates how to create a smart garage door opener
 * that can be controlled via Alexa, Google Home, or the SinricPro app.
 *
 * Hardware:
 * - Raspberry Pi Pico W
 * - Relay module connected to GPIO 15 (to trigger garage door opener)
 * - Door position sensor connected to GPIO 14 (to detect open/closed state)
 * - Button connected to GPIO 13 (for manual control)
 *
 * Wiring for Relay-Based Garage Door Opener:
 * ┌─────────────────────────────────────────────────────────┐
 * │ Pico W          Relay Module       Garage Door Opener   │
 * │ GPIO 15 ───────> IN                                      │
 * │ GND ───────────> GND                                     │
 * │ VBUS (5V) ─────> VCC                                     │
 * │                  COM ──────────────> Terminal 1          │
 * │                  NO ───────────────> Terminal 2          │
 * │                                                           │
 * │ Note: Relay simulates pressing the wall button by       │
 * │       momentarily closing the circuit (200ms pulse)      │
 * └─────────────────────────────────────────────────────────┘
 *
 * Door Position Sensor (Reed Switch/Magnetic Sensor):
 * - GPIO 14 to sensor signal (detects closed position)
 * - Sensor should pull GPIO 14 LOW when door is closed
 * - Use internal pull-up resistor
 *
 * Setup:
 * 1. Create a Garage Door device on sinric.pro and get your credentials
 * 2. Update WIFI_SSID, WIFI_PASSWORD, APP_KEY, APP_SECRET, DEVICE_ID
 * 3. Build and flash to your Pico W
 * 4. Control via "Alexa, open the garage" or "Alexa, close the garage"
 *
 * Connection Mode:
 * - Default: Secure mode (WSS on port 443) with TLS encryption
 * - Low Memory: Uncomment the line below to use non-secure mode (WS on port 80)
 */

// Uncomment this line to use non-secure WebSocket (port 80) for low memory devices
#define SINRICPRO_NOSSL

// Uncomment the following line to enable/disable sdk debug output
#define ENABLE_DEBUG

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/gpio.h"

#include "sinricpro/sinricpro.h"
#include "sinricpro/sinricpro_garagedoor.h"

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

#define RELAY_PIN       15  // GPIO for relay output (triggers garage door opener)
#define SENSOR_PIN      14  // GPIO for door position sensor (LOW = closed, HIGH = open)
#define BUTTON_PIN      13  // GPIO for physical button input
#define DEBOUNCE_MS     50  // Button debounce time
#define RELAY_PULSE_MS  200 // Duration to activate relay (simulates button press)

// =============================================================================
// Global Variables
// =============================================================================

static sinricpro_garagedoor_t my_garagedoor;
static bool current_door_state = false;  // false = open, true = closed
static uint32_t last_button_press = 0;

// =============================================================================
// Callbacks
// =============================================================================

/**
 * @brief Handle door state change from cloud (Alexa/Google/App)
 *
 * This is called when a voice command or app requests to open/close the door.
 * Trigger the relay to activate the garage door opener.
 *
 * @param device The device receiving the command
 * @param state  Pointer to new state (true = close, false = open)
 * @return true if successful, false on error
 */
bool on_door_state(sinricpro_device_t *device, bool *state) {
    printf("[Callback] Door command: %s\n", *state ? "CLOSE" : "OPEN");

    // Activate relay with a momentary pulse (simulates pressing wall button)
    gpio_put(RELAY_PIN, true);
    sleep_ms(RELAY_PULSE_MS);
    gpio_put(RELAY_PIN, false);

    // Update current state
    // Note: In a real implementation, you would wait for the sensor
    // to confirm the door has moved. For this example, we assume success.
    current_door_state = *state;

    printf("[Door] Relay activated - door should be %s\n",
           *state ? "closing" : "opening");

    return true;  // Return true to indicate success
}

/**
 * @brief Connection state change callback
 */
void on_state_change(sinricpro_state_t state, void *user_data) {
    switch (state) {
        case SINRICPRO_STATE_DISCONNECTED:
            printf("[State] Disconnected\n");
            break;
        case SINRICPRO_STATE_WIFI_CONNECTING:
            printf("[State] Connecting to WiFi...\n");
            break;
        case SINRICPRO_STATE_WIFI_CONNECTED:
            printf("[State] WiFi connected\n");
            break;
        case SINRICPRO_STATE_WS_CONNECTING:
            printf("[State] Connecting to SinricPro...\n");
            break;
        case SINRICPRO_STATE_CONNECTED:
            printf("[State] Connected to SinricPro!\n");
            break;
        case SINRICPRO_STATE_ERROR:
            printf("[State] Error\n");
            break;
    }
}

// =============================================================================
// Hardware Functions
// =============================================================================

/**
 * @brief Initialize GPIO pins
 */
void init_hardware(void) {
    // Initialize relay output
    gpio_init(RELAY_PIN);
    gpio_set_dir(RELAY_PIN, GPIO_OUT);
    gpio_put(RELAY_PIN, false);

    // Initialize door sensor input with pull-up
    gpio_init(SENSOR_PIN);
    gpio_set_dir(SENSOR_PIN, GPIO_IN);
    gpio_pull_up(SENSOR_PIN);

    // Initialize button input with pull-up
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
}

/**
 * @brief Check for button press with debouncing
 */
bool check_button(void) {
    static bool last_state = true;  // Pull-up means high when not pressed

    bool button_state = gpio_get(BUTTON_PIN);
    uint32_t now = to_ms_since_boot(get_absolute_time());

    // Check for falling edge (button pressed) with debounce
    if (!button_state && last_state && (now - last_button_press > DEBOUNCE_MS)) {
        last_button_press = now;
        last_state = button_state;
        return true;
    }

    last_state = button_state;
    return false;
}

/**
 * @brief Read door position sensor
 * @return true if door is closed, false if open
 */
bool read_door_sensor(void) {
    // Sensor pulls pin LOW when door is closed
    return !gpio_get(SENSOR_PIN);
}

// =============================================================================
// WiFi Functions
// =============================================================================

/**
 * @brief Connect to WiFi network
 *
 * Initializes the WiFi hardware and connects to the specified network.
 * This must be called before sinricpro_begin().
 *
 * @return true if connected successfully, false on error
 */
bool connect_wifi(void) {
    printf("[1/4] Initializing WiFi...\n");

    if (cyw43_arch_init()) {
        printf("ERROR: Failed to initialize WiFi hardware\n");
        return false;
    }

    cyw43_arch_enable_sta_mode();

    printf("[2/4] Connecting to WiFi: %s\n", WIFI_SSID);

    int result = cyw43_arch_wifi_connect_timeout_ms(
        WIFI_SSID,
        WIFI_PASSWORD,
        CYW43_AUTH_WPA2_AES_PSK,
        30000);  // 30 second timeout

    if (result != 0) {
        printf("ERROR: WiFi connection failed (error: %d)\n", result);
        return false;
    }

    printf("      WiFi connected!\n\n");
    return true;
}

// =============================================================================
// Main
// =============================================================================

int main() {
    // Initialize stdio
    stdio_init_all();
    sleep_ms(2000);  // Wait for USB serial

    printf("\n");
    printf("================================================\n");
    printf("SinricPro Garage Door Example for Pico W\n");
    printf("================================================\n\n");

    // Initialize hardware
    init_hardware();

    // Read initial door state from sensor
    current_door_state = read_door_sensor();
    printf("[Sensor] Initial door state: %s\n",
           current_door_state ? "CLOSED" : "OPEN");

    // =============================================================================
    // Step 1: Connect to WiFi (user responsibility)
    // =============================================================================

    if (!connect_wifi()) {
        // Blink LED rapidly on WiFi error
        while (1) {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
            sleep_ms(100);
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
            sleep_ms(100);
        }
    }

    // =============================================================================
    // Step 2: Initialize SinricPro SDK
    // =============================================================================

    printf("[3/4] Initializing SinricPro SDK...\n");

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
        while (1) tight_loop_contents();
    }

    // Set state change callback
    sinricpro_on_state_change(on_state_change, NULL);

    // Initialize garage door device
    if (!sinricpro_garagedoor_init(&my_garagedoor, DEVICE_ID)) {
        printf("ERROR: Failed to initialize garage door device\n");
        return 1;
    }

    // Set door state callback
    sinricpro_garagedoor_on_door_state(&my_garagedoor, on_door_state);

    // Add device to SinricPro
    if (!sinricpro_add_device((sinricpro_device_t *)&my_garagedoor)) {
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
    printf("Ready! Voice commands:\n");
    printf("  'Alexa, open the garage'\n");
    printf("  'Alexa, close the garage'\n");
    printf("  'Hey Google, open the garage'\n");
    printf("\n");
    printf("Press the button to trigger door manually.\n");
    printf("================================================\n\n");

    // Main loop
    while (1) {
        // Get current time (used by multiple features)
        uint32_t now = to_ms_since_boot(get_absolute_time());

        // Process SinricPro events
        sinricpro_handle();

        // Check for physical button press
        if (check_button()) {
            printf("[Button] Manual door activation\n");

            // Activate relay with a momentary pulse
            gpio_put(RELAY_PIN, true);
            sleep_ms(RELAY_PULSE_MS);
            gpio_put(RELAY_PIN, false);

            // Toggle state (or wait for sensor confirmation)
            current_door_state = !current_door_state;

            // Send event to cloud
            if (sinricpro_is_connected()) {
                sinricpro_garagedoor_send_door_state_event(&my_garagedoor, current_door_state);
            }
        }

        // Monitor door sensor for state changes
        static uint32_t last_sensor_check = 0;
        if (now - last_sensor_check > 500) {  // Check sensor every 500ms
            last_sensor_check = now;
            bool sensor_state = read_door_sensor();

            // If sensor state differs from our current state, update cloud
            if (sensor_state != current_door_state) {
                current_door_state = sensor_state;
                printf("[Sensor] Door state changed: %s\n",
                       current_door_state ? "CLOSED" : "OPEN");

                // Send event to cloud
                if (sinricpro_is_connected()) {
                    sinricpro_garagedoor_send_door_state_event(&my_garagedoor, current_door_state);
                }
            }
        }

        // Blink onboard LED when connected
        static uint32_t last_blink = 0;
        if (now - last_blink > 1000) {
            last_blink = now;
            if (sinricpro_is_connected()) {
                cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
                sleep_ms(50);
                cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
            }
        }

        // Small delay to prevent tight loop
        sleep_ms(10);
    }

    return 0;
}
