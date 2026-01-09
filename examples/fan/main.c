/**
 * @file main.c
 * @brief SinricPro Fan Example for Raspberry Pi Pico W
 *
 * This example demonstrates how to create a smart fan with variable speed control
 * that can be controlled via Alexa, Google Home, or the SinricPro app.
 *
 * Hardware:
 * - Raspberry Pi Pico W
 * - DC Motor or Fan connected via MOSFET/transistor to GPIO 15 (PWM output)
 * - LED connected to GPIO 14 (optional, indicates on/off state)
 * - Button connected to GPIO 13 (optional, for local control)
 *
 * Wiring Example:
 * - GPIO 15 -> MOSFET Gate (controls motor via PWM)
 * - MOSFET Drain -> Motor negative terminal
 * - MOSFET Source -> GND
 * - Motor positive terminal -> VCC (external power supply, 5-12V)
 * - Add flyback diode across motor terminals!
 *
 * Setup:
 * 1. Create a "Fan" device on sinric.pro and get your credentials
 * 2. Update WIFI_SSID, WIFI_PASSWORD, APP_KEY, APP_SECRET, DEVICE_ID
 * 3. Build and flash to your Pico W
 * 4. Control via:
 *    - "Alexa, turn on [device name]"
 *    - "Alexa, set [device name] to 50 percent"
 *    - "Alexa, increase [device name]"
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
#include "hardware/pwm.h"

#include "sinricpro/sinricpro.h"
#include "sinricpro/sinricpro_fan.h"

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

#define FAN_PWM_PIN     15  // GPIO for PWM fan control (must be PWM-capable)
#define LED_PIN         14  // GPIO for LED indicator
#define BUTTON_PIN      13  // GPIO for physical button input
#define DEBOUNCE_MS     50  // Button debounce time

// =============================================================================
// Global Variables
// =============================================================================

static sinricpro_fan_t my_fan;
static bool current_power_state = false;
static int current_power_level = 0;  // 0-100%
static uint32_t last_button_press = 0;

static uint pwm_slice_num;

// =============================================================================
// Callbacks
// =============================================================================

/**
 * @brief Handle power state change from cloud (Alexa/Google/App)
 *
 * This is called when a voice command or app toggles the fan on/off.
 *
 * @param device The device receiving the command
 * @param state  Pointer to new state (can be modified)
 * @return true if successful, false on error
 */
bool on_power_state(sinricpro_device_t *device, bool *state) {
    printf("[Callback] Power state: %s\n", *state ? "ON" : "OFF");

    current_power_state = *state;

    // Update hardware
    gpio_put(LED_PIN, current_power_state);

    if (current_power_state) {
        // Fan is ON - apply current power level
        uint16_t pwm_level = (current_power_level * 65535) / 100;
        pwm_set_gpio_level(FAN_PWM_PIN, pwm_level);
        printf("[Hardware] Fan speed: %d%%\n", current_power_level);
    } else {
        // Fan is OFF - stop PWM
        pwm_set_gpio_level(FAN_PWM_PIN, 0);
    }

    return true;
}

/**
 * @brief Handle power level change from cloud
 *
 * This is called when a voice command sets the fan speed.
 * Example: "Alexa, set fan to 75 percent"
 *
 * @param device      The device receiving the command
 * @param power_level Pointer to new power level 0-100 (can be modified)
 * @return true if successful, false on error
 */
bool on_power_level(sinricpro_device_t *device, int *power_level) {
    printf("[Callback] Power level: %d%%\n", *power_level);

    // Clamp to valid range
    if (*power_level < 0) *power_level = 0;
    if (*power_level > 100) *power_level = 100;

    current_power_level = *power_level;

    // If fan is on, update PWM immediately
    if (current_power_state) {
        uint16_t pwm_level = (current_power_level * 65535) / 100;
        pwm_set_gpio_level(FAN_PWM_PIN, pwm_level);
        printf("[Hardware] Fan speed: %d%%\n", current_power_level);
    }

    return true;
}

/**
 * @brief Handle relative power level adjustment
 *
 * This is called when a voice command adjusts the fan speed relatively.
 * Example: "Alexa, increase fan" (adds +10%)
 *
 * @param device        The device receiving the command
 * @param power_level_delta Pointer to delta value (can be modified)
 * @return true if successful, false on error
 */
bool on_adjust_power_level(sinricpro_device_t *device, int *power_level_delta) {
    printf("[Callback] Adjust power level: %+d%%\n", *power_level_delta);

    // Calculate new level
    int new_level = current_power_level + *power_level_delta;

    // Clamp to valid range
    if (new_level < 0) new_level = 0;
    if (new_level > 100) new_level = 100;

    current_power_level = new_level;

    // If fan is on, update PWM immediately
    if (current_power_state) {
        uint16_t pwm_level = (current_power_level * 65535) / 100;
        pwm_set_gpio_level(FAN_PWM_PIN, pwm_level);
        printf("[Hardware] New fan speed: %d%%\n", current_power_level);
    }

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
 * @brief Initialize GPIO pins and PWM
 */
void init_hardware(void) {
    // Initialize LED output
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, false);

    // Initialize button input with pull-up
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    // Initialize PWM for fan control
    gpio_set_function(FAN_PWM_PIN, GPIO_FUNC_PWM);
    pwm_slice_num = pwm_gpio_to_slice_num(FAN_PWM_PIN);

    // Set PWM frequency to ~25kHz (good for motor control)
    pwm_set_wrap(pwm_slice_num, 65535);
    pwm_set_clkdiv(pwm_slice_num, 2.0f);

    // Start with fan off
    pwm_set_gpio_level(FAN_PWM_PIN, 0);
    pwm_set_enabled(pwm_slice_num, true);

    printf("[Hardware] PWM initialized on GPIO %d (slice %d)\n", FAN_PWM_PIN, pwm_slice_num);
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
    printf("SinricPro Fan Example for Pico W\n");
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

    // Initialize fan device
    if (!sinricpro_fan_init(&my_fan, DEVICE_ID)) {
        printf("ERROR: Failed to initialize fan device\n");
        return 1;
    }

    // Set callbacks
    sinricpro_fan_on_power_state(&my_fan, on_power_state);
    sinricpro_fan_on_power_level(&my_fan, on_power_level);
    sinricpro_fan_on_adjust_power_level(&my_fan, on_adjust_power_level);

    // Add device to SinricPro
    if (!sinricpro_add_device((sinricpro_device_t *)&my_fan)) {
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
    printf("  'Alexa, turn on [device name]'\n");
    printf("  'Alexa, set [device name] to 50 percent'\n");
    printf("  'Alexa, increase [device name]'\n");
    printf("  'Hey Google, set [device name] to high'\n");
    printf("\n");
    printf("Press the button to cycle fan speeds.\n");
    printf("================================================\n\n");

    // Main loop
    while (1) {
        // Get current time
        uint32_t now = to_ms_since_boot(get_absolute_time());

        // Process SinricPro events
        sinricpro_handle();

        // Check for physical button press
        if (check_button()) {
            // Cycle through speeds: OFF -> 33% -> 66% -> 100% -> OFF
            if (!current_power_state) {
                // Turn on at 33%
                current_power_state = true;
                current_power_level = 33;
            } else if (current_power_level < 100) {
                // Increase speed
                current_power_level += 33;
                if (current_power_level > 100) current_power_level = 100;
            } else {
                // Turn off
                current_power_state = false;
                current_power_level = 0;
            }

            // Update hardware
            gpio_put(LED_PIN, current_power_state);
            uint16_t pwm_level = current_power_state ? (current_power_level * 65535) / 100 : 0;
            pwm_set_gpio_level(FAN_PWM_PIN, pwm_level);

            printf("[Button] Fan %s at %d%%\n",
                   current_power_state ? "ON" : "OFF",
                   current_power_level);

            // Send events to cloud
            if (sinricpro_is_connected()) {
                sinricpro_fan_send_power_state_event(&my_fan, current_power_state);
                if (current_power_state) {
                    sinricpro_fan_send_power_level_event(&my_fan, current_power_level);
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
