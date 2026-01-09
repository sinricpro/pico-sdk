/**
 * @file main.c
 * @brief SinricPro Lock Example for Raspberry Pi Pico W
 *
 * This example demonstrates how to create a smart lock device
 * that can be controlled via Alexa, Google Home, or the SinricPro app.
 *
 * Hardware:
 * - Raspberry Pi Pico W
 * - 12V solenoid lock or electric strike
 * - Relay module or MOSFET driver connected to GPIO 15
 * - LED connected to GPIO 14 (lock indicator - optional)
 * - Button connected to GPIO 13 (physical lock/unlock - optional)
 * - Magnetic sensor for jam detection (optional)
 *
 * Wiring Example (Relay Module):
 * - GPIO 15 -> Relay IN
 * - Relay COM -> Lock positive (+12V from power supply)
 * - Relay NO -> Lock negative (GND)
 * - Add flyback diode across lock terminals!
 *
 * Safety Note:
 * - Always include a manual override for emergency access
 * - Test fail-safe behavior (what happens if power is lost?)
 * - Consider using fail-safe locks (unlock when power is lost)
 *
 * Setup:
 * 1. Create a "Lock" device on sinric.pro and get your credentials
 * 2. Update WIFI_SSID, WIFI_PASSWORD, APP_KEY, APP_SECRET, DEVICE_ID
 * 3. Build and flash to your Pico W
 * 4. Control via:
 *    - "Alexa, lock the [device name]"
 *    - "Alexa, unlock the [device name]"
 *    - "Hey Google, lock the front door"
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
#include "sinricpro/sinricpro_lock.h"

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

#define LOCK_PIN        15  // GPIO for lock relay/solenoid control
#define LED_PIN         14  // GPIO for lock status LED
#define BUTTON_PIN      13  // GPIO for physical lock/unlock button
#define DEBOUNCE_MS     50  // Button debounce time

// Lock timing (adjust for your lock mechanism)
#define LOCK_ENGAGE_MS  500   // Time to keep solenoid energized to lock (ms)
#define UNLOCK_ENGAGE_MS 500  // Time to keep solenoid energized to unlock (ms)

// =============================================================================
// Global Variables
// =============================================================================

static sinricpro_lock_t my_lock;
static bool current_lock_state = false;  // false = unlocked, true = locked
static uint32_t last_button_press = 0;
static uint32_t lock_engage_time = 0;
static bool lock_engaging = false;

// =============================================================================
// Callbacks
// =============================================================================

/**
 * @brief Handle lock state change from cloud (Alexa/Google/App)
 *
 * This is called when a voice command or app locks/unlocks the device.
 *
 * @param device The device receiving the command
 * @param state  Pointer to lock state (true = lock, false = unlock)
 * @return true if successful, false if jammed/failed
 */
bool on_lock_state(sinricpro_device_t *device, bool *state) {
    printf("[Callback] Lock state: %s\n", *state ? "LOCK" : "UNLOCK");

    // Simulate lock mechanism
    // In a real application, you would:
    // 1. Energize solenoid/motor
    // 2. Check sensor to verify lock engaged
    // 3. Return false if jammed

    current_lock_state = *state;

    // Update hardware - energize lock
    gpio_put(LOCK_PIN, true);
    gpio_put(LED_PIN, current_lock_state);

    lock_engaging = true;
    lock_engage_time = to_ms_since_boot(get_absolute_time());

    printf("[Hardware] Lock mechanism %s\n", current_lock_state ? "LOCKED" : "UNLOCKED");

    // In a real implementation, check sensor here and return false if jammed
    // For this example, we always succeed
    return true;
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
    // Initialize lock control output (relay/solenoid)
    gpio_init(LOCK_PIN);
    gpio_set_dir(LOCK_PIN, GPIO_OUT);
    gpio_put(LOCK_PIN, false);  // Start with lock de-energized

    // Initialize LED output
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, false);  // Start with LED off (unlocked)

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
 * @brief Handle lock engaging/disengaging timing
 *
 * Keeps the solenoid energized for a brief period, then de-energizes it
 * to prevent overheating and save power.
 */
void handle_lock_timing(void) {
    if (!lock_engaging) return;

    uint32_t now = to_ms_since_boot(get_absolute_time());
    uint32_t engage_duration = current_lock_state ? LOCK_ENGAGE_MS : UNLOCK_ENGAGE_MS;

    if (now - lock_engage_time >= engage_duration) {
        // De-energize solenoid
        gpio_put(LOCK_PIN, false);
        lock_engaging = false;
        printf("[Hardware] Lock mechanism de-energized\n");
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
    printf("SinricPro Lock Example for Pico W\n");
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

    // Initialize lock device
    if (!sinricpro_lock_init(&my_lock, DEVICE_ID)) {
        printf("ERROR: Failed to initialize lock device\n");
        return 1;
    }

    // Set lock state callback
    sinricpro_lock_on_lock_state(&my_lock, on_lock_state);

    // Add device to SinricPro
    if (!sinricpro_add_device((sinricpro_device_t *)&my_lock)) {
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
    printf("Ready! Voice commands:\n");
    printf("  'Alexa, lock the [device name]'\n");
    printf("  'Alexa, unlock the [device name]'\n");
    printf("  'Hey Google, lock the front door'\n");
    printf("\n");
    printf("Press the button to toggle lock/unlock.\n");
    printf("================================================\n\n");
    printf("WARNING: Always ensure manual override access!\n\n");

    // Main loop
    while (1) {
        // Get current time
        uint32_t now = to_ms_since_boot(get_absolute_time());

        // Process SinricPro events
        sinricpro_handle();

        // Handle lock timing (de-energize solenoid after engage time)
        handle_lock_timing();

        // Check for physical button press
        if (check_button()) {
            // Toggle lock state
            current_lock_state = !current_lock_state;

            // Energize lock mechanism
            gpio_put(LOCK_PIN, true);
            gpio_put(LED_PIN, current_lock_state);

            lock_engaging = true;
            lock_engage_time = now;

            printf("[Button] Lock %s\n", current_lock_state ? "LOCKED" : "UNLOCKED");

            // Send event to cloud
            if (sinricpro_is_connected()) {
                sinricpro_lock_send_lock_state_event(&my_lock, current_lock_state);
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
