/**
 * @file main.c
 * @brief SinricPro Doorbell Example for Raspberry Pi Pico W
 *
 * This example demonstrates how to create a smart doorbell device
 * that sends notifications via Alexa, Google Home, or the SinricPro app.
 *
 * Hardware:
 * - Raspberry Pi Pico W
 * - Doorbell button connected to GPIO 14
 * - Optional: buzzer/chime connected to GPIO 15
 * - Optional: LED indicator on GPIO 13
 *
 * Setup:
 * 1. Create a "Doorbell" or "Contact Sensor" device on sinric.pro
 * 2. Update WIFI_SSID, WIFI_PASSWORD, APP_KEY, APP_SECRET, DEVICE_ID
 * 3. Build and flash to your Pico W
 * 4. Enable notifications in Alexa/Google Home app
 * 5. Press the doorbell button to trigger notifications
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
#include "sinricpro/sinricpro_doorbell.h"

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

#define BUTTON_PIN      14  // GPIO for doorbell button
#define BUZZER_PIN      15  // GPIO for buzzer/chime (optional)
#define LED_PIN         13  // GPIO for status LED (optional)
#define DEBOUNCE_MS     500 // Button debounce time (longer to prevent spamming)

// =============================================================================
// Global Variables
// =============================================================================

static sinricpro_doorbell_t my_doorbell;
static uint32_t last_button_press = 0;

// =============================================================================
// Callbacks
// =============================================================================

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
    // Initialize button input with pull-up
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    // Initialize LED output
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, false);

    // Initialize buzzer output (optional)
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    gpio_put(BUZZER_PIN, false);
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
 * @brief Sound the buzzer/chime
 */
void sound_chime(void) {
    // Simple beep pattern
    for (int i = 0; i < 3; i++) {
        gpio_put(BUZZER_PIN, true);
        sleep_ms(100);
        gpio_put(BUZZER_PIN, false);
        sleep_ms(100);
    }
}

// =============================================================================
// WiFi Functions
// =============================================================================

/**
 * @brief Connect to WiFi network
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
    printf("SinricPro Doorbell Example for Pico W\n");
    printf("================================================\n\n");

    // Initialize hardware
    init_hardware();

    // Connect to WiFi
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
    // Initialize SinricPro SDK
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

    // Initialize doorbell device
    if (!sinricpro_doorbell_init(&my_doorbell, DEVICE_ID)) {
        printf("ERROR: Failed to initialize doorbell device\n");
        return 1;
    }

    // Add device to SinricPro
    if (!sinricpro_add_device((sinricpro_device_t *)&my_doorbell)) {
        printf("ERROR: Failed to add device\n");
        return 1;
    }

    // =============================================================================
    // Connect to SinricPro Server
    // =============================================================================

    printf("[4/4] Connecting to SinricPro...\n");
    if (!sinricpro_begin()) {
        printf("ERROR: Failed to connect to SinricPro\n");
        return 1;
    }

    printf("\n");
    printf("================================================\n");
    printf("Ready! Doorbell is active.\n");
    printf("Press the button to send a notification.\n");
    printf("\n");
    printf("Enable notifications in your Alexa/Google app\n");
    printf("to receive doorbell alerts.\n");
    printf("================================================\n\n");

    // Main loop
    while (1) {
        // Get current time
        uint32_t now = to_ms_since_boot(get_absolute_time());

        // Process SinricPro events
        sinricpro_handle();

        // Check for doorbell button press
        if (check_button()) {
            printf("[Doorbell] Button pressed!\n");

            // Flash LED
            gpio_put(LED_PIN, true);

            // Sound the chime
            sound_chime();

            // Send event to cloud (triggers notifications)
            if (sinricpro_is_connected()) {
                if (sinricpro_doorbell_send_press_event(&my_doorbell)) {
                    printf("[Doorbell] Event sent successfully\n");
                } else {
                    printf("[Doorbell] Failed to send event\n");
                }
            }

            // Turn off LED
            gpio_put(LED_PIN, false);
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
