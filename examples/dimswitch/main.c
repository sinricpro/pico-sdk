/**
 * @file main.c
 * @brief SinricPro DimSwitch Example for Raspberry Pi Pico W
 *
 * This example demonstrates how to create a dimmable smart switch
 * that can be controlled via Alexa, Google Home, or the SinricPro app.
 *
 * Hardware:
 * - Raspberry Pi Pico W
 * - LED connected to GPIO 15 (PWM output for dimming)
 * - Button connected to GPIO 14 (optional)
 *
 * Voice Commands:
 * - "Alexa, turn on [device name]"
 * - "Alexa, set [device name] to 50 percent"
 * - "Alexa, dim [device name]"
 * - "Alexa, brighten [device name]"
 *
 * Connection Mode:
 * - Default: Secure mode (WSS on port 443) with TLS encryption
 * - Low Memory: Uncomment the line below to use non-secure mode (WS on port 80)
 */

// Uncomment this line to use non-secure WebSocket (port 80) for low memory devices
// #define SINRICPRO_NOSSL

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

#include "sinricpro/sinricpro.h"
#include "sinricpro/sinricpro_dimswitch.h"

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

#define LED_PIN         15  // GPIO for PWM LED output
#define BUTTON_PIN      14  // GPIO for physical button input
#define DEBOUNCE_MS     50  // Button debounce time

// PWM configuration
#define PWM_WRAP        255 // PWM resolution (8-bit)

// =============================================================================
// Global Variables
// =============================================================================

static sinricpro_dimswitch_t my_dimmer;
static bool current_power_state = false;
static int current_brightness = 100;  // Default 100%
static uint32_t last_button_press = 0;
static uint pwm_slice;

// =============================================================================
// Hardware Functions
// =============================================================================

/**
 * @brief Initialize PWM for LED dimming
 */
void init_pwm(void) {
    gpio_set_function(LED_PIN, GPIO_FUNC_PWM);
    pwm_slice = pwm_gpio_to_slice_num(LED_PIN);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_wrap(&config, PWM_WRAP);
    pwm_init(pwm_slice, &config, true);

    pwm_set_gpio_level(LED_PIN, 0);
}

/**
 * @brief Set LED brightness
 *
 * @param brightness 0-100 percentage
 */
void set_led_brightness(int brightness) {
    // Convert 0-100% to 0-255 PWM value
    uint16_t pwm_value = (brightness * PWM_WRAP) / 100;
    pwm_set_gpio_level(LED_PIN, pwm_value);
}

/**
 * @brief Update LED output based on power state and brightness
 */
void update_led(void) {
    if (current_power_state) {
        set_led_brightness(current_brightness);
    } else {
        set_led_brightness(0);
    }
}

/**
 * @brief Initialize GPIO pins
 */
void init_hardware(void) {
    // Initialize PWM for LED
    init_pwm();

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

// =============================================================================
// Callbacks
// =============================================================================

/**
 * @brief Handle power state change from cloud
 */
bool on_power_state(sinricpro_device_t *device, bool *state) {
    printf("[Callback] Power state: %s\n", *state ? "ON" : "OFF");

    current_power_state = *state;
    update_led();

    return true;
}

/**
 * @brief Handle brightness change from cloud
 */
bool on_brightness(sinricpro_device_t *device, int *brightness) {
    printf("[Callback] Brightness: %d%%\n", *brightness);

    current_brightness = *brightness;

    // If brightness is set while off, turn on
    if (current_brightness > 0 && !current_power_state) {
        current_power_state = true;
    }

    update_led();

    return true;
}

/**
 * @brief Handle adjust brightness from cloud (dim/brighten commands)
 */
bool on_adjust_brightness(sinricpro_device_t *device, int *brightness_delta) {
    printf("[Callback] Adjust brightness: %+d%%\n", *brightness_delta);

    // Apply delta to current brightness
    current_brightness += *brightness_delta;

    // Clamp to 0-100
    if (current_brightness < 0) current_brightness = 0;
    if (current_brightness > 100) current_brightness = 100;

    // Return absolute brightness
    *brightness_delta = current_brightness;

    // Turn on if adjusting while off
    if (current_brightness > 0 && !current_power_state) {
        current_power_state = true;
    }

    update_led();

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
// Main
// =============================================================================

int main() {
    // Initialize stdio
    stdio_init_all();
    sleep_ms(2000);  // Wait for USB serial

    printf("\n");
    printf("================================================\n");
    printf("SinricPro DimSwitch Example for Pico W\n");
    printf("SDK Version: %s\n", sinricpro_get_version());
    printf("================================================\n\n");

    // Initialize hardware
    init_hardware();

    // Configure SinricPro
    sinricpro_config_t config = {
        .app_key = APP_KEY,
        .app_secret = APP_SECRET,
        .wifi_ssid = WIFI_SSID,
        .wifi_password = WIFI_PASSWORD,
        .use_ssl = false
    };

    // Initialize SDK
    if (!sinricpro_init(&config)) {
        printf("ERROR: Failed to initialize SinricPro\n");
        while (1) {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
            sleep_ms(100);
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
            sleep_ms(100);
        }
    }

    // Set state change callback
    sinricpro_on_state_change(on_state_change, NULL);

    // Initialize dimswitch device
    if (!sinricpro_dimswitch_init(&my_dimmer, DEVICE_ID)) {
        printf("ERROR: Failed to initialize dimswitch device\n");
        return 1;
    }

    // Set callbacks
    sinricpro_dimswitch_on_power_state(&my_dimmer, on_power_state);
    sinricpro_dimswitch_on_brightness(&my_dimmer, on_brightness);
    sinricpro_dimswitch_on_adjust_brightness(&my_dimmer, on_adjust_brightness);

    // Add device to SinricPro
    if (!sinricpro_add_device((sinricpro_device_t *)&my_dimmer)) {
        printf("ERROR: Failed to add device\n");
        return 1;
    }

    // Connect to SinricPro
    printf("\nConnecting...\n");
    if (!sinricpro_begin()) {
        printf("ERROR: Failed to connect\n");
        return 1;
    }

    printf("\n");
    printf("================================================\n");
    printf("Ready! Voice commands:\n");
    printf("  'Alexa, turn on [device name]'\n");
    printf("  'Alexa, turn off [device name]'\n");
    printf("  'Alexa, set [device name] to 50 percent'\n");
    printf("  'Alexa, dim [device name]'\n");
    printf("  'Alexa, brighten [device name]'\n");
    printf("\n");
    printf("Press button to toggle power.\n");
    printf("================================================\n\n");

    // Main loop
    while (1) {
        // Process SinricPro events
        sinricpro_handle();

        // Check for physical button press
        if (check_button()) {
            // Toggle power state
            current_power_state = !current_power_state;
            update_led();

            printf("[Button] Power: %s (brightness: %d%%)\n",
                   current_power_state ? "ON" : "OFF", current_brightness);

            // Send event to cloud
            if (sinricpro_is_connected()) {
                sinricpro_dimswitch_send_power_state_event(&my_dimmer, current_power_state);
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

        // Small delay to prevent tight loop
        sleep_ms(10);
    }

    return 0;
}
