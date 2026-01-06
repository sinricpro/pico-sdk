/**
 * @file main.c
 * @brief SinricPro RGB+CCT Light Example for Raspberry Pi Pico W
 *
 * This example demonstrates how to create a smart RGB light with color
 * temperature control (CCT), supporting full voice control via Alexa/Google.
 *
 * Hardware:
 * - Raspberry Pi Pico W
 * - RGB LED strip or RGB+CCT LED connected to:
 *   - GPIO 13: Red channel (PWM)
 *   - GPIO 14: Green channel (PWM)
 *   - GPIO 15: Blue channel (PWM)
 *   - GPIO 16: Warm White channel (PWM, optional)
 *   - GPIO 17: Cool White channel (PWM, optional)
 *
 * Voice Commands:
 * - "Alexa, turn on [light name]"
 * - "Alexa, set [light name] to 50 percent"
 * - "Alexa, set [light name] to red"
 * - "Alexa, set [light name] to warm white"
 * - "Alexa, make [light name] warmer/cooler"
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
#include <math.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

#include "sinricpro/sinricpro.h"
#include "sinricpro/sinricpro_light.h"

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

#define PIN_RED         13  // GPIO for red channel
#define PIN_GREEN       14  // GPIO for green channel
#define PIN_BLUE        15  // GPIO for blue channel
#define PIN_WARM_WHITE  16  // GPIO for warm white (optional)
#define PIN_COOL_WHITE  17  // GPIO for cool white (optional)

#define PWM_WRAP        255  // PWM resolution (8-bit)

// =============================================================================
// Global Variables
// =============================================================================

static sinricpro_light_t my_light;
static bool current_power_state = false;
static int current_brightness = 100;  // 0-100%
static sinricpro_color_t current_color = {255, 255, 255};  // Default: white
static int current_color_temp = 2700;  // Default: warm white (2700K)

static uint pwm_slice_red, pwm_slice_green, pwm_slice_blue;
static uint pwm_slice_warm, pwm_slice_cool;

// =============================================================================
// Hardware Functions
// =============================================================================

/**
 * @brief Initialize PWM for all LED channels
 */
void init_pwm(void) {
    // Configure RGB channels
    gpio_set_function(PIN_RED, GPIO_FUNC_PWM);
    gpio_set_function(PIN_GREEN, GPIO_FUNC_PWM);
    gpio_set_function(PIN_BLUE, GPIO_FUNC_PWM);

    pwm_slice_red = pwm_gpio_to_slice_num(PIN_RED);
    pwm_slice_green = pwm_gpio_to_slice_num(PIN_GREEN);
    pwm_slice_blue = pwm_gpio_to_slice_num(PIN_BLUE);

    // Configure PWM for RGB
    pwm_config config = pwm_get_default_config();
    pwm_config_set_wrap(&config, PWM_WRAP);

    pwm_init(pwm_slice_red, &config, true);
    pwm_init(pwm_slice_green, &config, true);
    pwm_init(pwm_slice_blue, &config, true);

    // Configure white channels (optional)
    gpio_set_function(PIN_WARM_WHITE, GPIO_FUNC_PWM);
    gpio_set_function(PIN_COOL_WHITE, GPIO_FUNC_PWM);

    pwm_slice_warm = pwm_gpio_to_slice_num(PIN_WARM_WHITE);
    pwm_slice_cool = pwm_gpio_to_slice_num(PIN_COOL_WHITE);

    pwm_init(pwm_slice_warm, &config, true);
    pwm_init(pwm_slice_cool, &config, true);

    // Start with all off
    pwm_set_gpio_level(PIN_RED, 0);
    pwm_set_gpio_level(PIN_GREEN, 0);
    pwm_set_gpio_level(PIN_BLUE, 0);
    pwm_set_gpio_level(PIN_WARM_WHITE, 0);
    pwm_set_gpio_level(PIN_COOL_WHITE, 0);
}

/**
 * @brief Convert color temperature (Kelvin) to RGB approximation
 *
 * @param kelvin Color temperature (2200-7000K)
 * @param r Output: red component (0-255)
 * @param g Output: green component (0-255)
 * @param b Output: blue component (0-255)
 */
void kelvin_to_rgb(int kelvin, uint8_t *r, uint8_t *g, uint8_t *b) {
    float temp = kelvin / 100.0f;
    float red, green, blue;

    // Red
    if (temp <= 66) {
        red = 255;
    } else {
        red = temp - 60;
        red = 329.698727446f * powf(red, -0.1332047592f);
        if (red < 0) red = 0;
        if (red > 255) red = 255;
    }

    // Green
    if (temp <= 66) {
        green = temp;
        green = 99.4708025861f * logf(green) - 161.1195681661f;
        if (green < 0) green = 0;
        if (green > 255) green = 255;
    } else {
        green = temp - 60;
        green = 288.1221695283f * powf(green, -0.0755148492f);
        if (green < 0) green = 0;
        if (green > 255) green = 255;
    }

    // Blue
    if (temp >= 66) {
        blue = 255;
    } else if (temp <= 19) {
        blue = 0;
    } else {
        blue = temp - 10;
        blue = 138.5177312231f * logf(blue) - 305.0447927307f;
        if (blue < 0) blue = 0;
        if (blue > 255) blue = 255;
    }

    *r = (uint8_t)red;
    *g = (uint8_t)green;
    *b = (uint8_t)blue;
}

/**
 * @brief Update LED output based on current state
 */
void update_light(void) {
    if (!current_power_state) {
        // Turn off all channels
        pwm_set_gpio_level(PIN_RED, 0);
        pwm_set_gpio_level(PIN_GREEN, 0);
        pwm_set_gpio_level(PIN_BLUE, 0);
        pwm_set_gpio_level(PIN_WARM_WHITE, 0);
        pwm_set_gpio_level(PIN_COOL_WHITE, 0);
        return;
    }

    // Apply brightness scaling (0-100% to 0-255)
    float brightness_scale = current_brightness / 100.0f;

    // Set RGB channels
    pwm_set_gpio_level(PIN_RED, (uint16_t)(current_color.r * brightness_scale));
    pwm_set_gpio_level(PIN_GREEN, (uint16_t)(current_color.g * brightness_scale));
    pwm_set_gpio_level(PIN_BLUE, (uint16_t)(current_color.b * brightness_scale));

    // For color temperature mode, also control white channels
    // Map 2200K-7000K to warm/cool mix
    float temp_normalized = (current_color_temp - 2200.0f) / (7000.0f - 2200.0f);
    uint16_t warm = (uint16_t)((1.0f - temp_normalized) * PWM_WRAP * brightness_scale);
    uint16_t cool = (uint16_t)(temp_normalized * PWM_WRAP * brightness_scale);

    pwm_set_gpio_level(PIN_WARM_WHITE, warm);
    pwm_set_gpio_level(PIN_COOL_WHITE, cool);
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
    update_light();

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

    update_light();

    return true;
}

/**
 * @brief Handle adjust brightness from cloud
 */
bool on_adjust_brightness(sinricpro_device_t *device, int *brightness_delta) {
    printf("[Callback] Adjust brightness: %+d%%\n", *brightness_delta);

    // Apply delta
    current_brightness += *brightness_delta;

    // Clamp to 0-100
    if (current_brightness < 0) current_brightness = 0;
    if (current_brightness > 100) current_brightness = 100;

    // Return absolute brightness
    *brightness_delta = current_brightness;

    if (current_brightness > 0 && !current_power_state) {
        current_power_state = true;
    }

    update_light();

    return true;
}

/**
 * @brief Handle color change from cloud
 */
bool on_color(sinricpro_device_t *device, sinricpro_color_t *color) {
    printf("[Callback] Color: RGB(%d, %d, %d)\n", color->r, color->g, color->b);

    current_color = *color;

    // If color is set while off, turn on
    if (!current_power_state) {
        current_power_state = true;
    }

    update_light();

    return true;
}

/**
 * @brief Handle color temperature change from cloud
 */
bool on_color_temperature(sinricpro_device_t *device, int *color_temp) {
    printf("[Callback] Color temperature: %dK\n", *color_temp);

    current_color_temp = *color_temp;

    // Convert color temp to RGB for display
    kelvin_to_rgb(current_color_temp, &current_color.r, &current_color.g, &current_color.b);

    if (!current_power_state) {
        current_power_state = true;
    }

    update_light();

    return true;
}

/**
 * @brief Handle increase color temperature
 */
bool on_increase_color_temp(sinricpro_device_t *device, int *delta) {
    printf("[Callback] Increase color temperature\n");

    // Increase by 500K
    current_color_temp += 500;
    if (current_color_temp > 7000) current_color_temp = 7000;

    // Return absolute temperature
    *delta = current_color_temp;

    kelvin_to_rgb(current_color_temp, &current_color.r, &current_color.g, &current_color.b);
    update_light();

    return true;
}

/**
 * @brief Handle decrease color temperature
 */
bool on_decrease_color_temp(sinricpro_device_t *device, int *delta) {
    printf("[Callback] Decrease color temperature\n");

    // Decrease by 500K
    current_color_temp -= 500;
    if (current_color_temp < 2200) current_color_temp = 2200;

    // Return absolute temperature
    *delta = current_color_temp;

    kelvin_to_rgb(current_color_temp, &current_color.r, &current_color.g, &current_color.b);
    update_light();

    return true;
}

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
    printf("SinricPro RGB+CCT Light Example\n");
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

    // Initialize light device
    if (!sinricpro_light_init(&my_light, DEVICE_ID)) {
        printf("ERROR: Failed to initialize light device\n");
        return 1;
    }

    // Set callbacks
    sinricpro_light_on_power_state(&my_light, on_power_state);
    sinricpro_light_on_brightness(&my_light, on_brightness);
    sinricpro_light_on_adjust_brightness(&my_light, on_adjust_brightness);
    sinricpro_light_on_color(&my_light, on_color);
    sinricpro_light_on_color_temperature(&my_light, on_color_temperature);
    sinricpro_light_on_increase_color_temperature(&my_light, on_increase_color_temp);
    sinricpro_light_on_decrease_color_temperature(&my_light, on_decrease_color_temp);

    // Add device to SinricPro
    if (!sinricpro_add_device((sinricpro_device_t *)&my_light)) {
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
    printf("  'Alexa, turn on [light name]'\n");
    printf("  'Alexa, set [light name] to 50 percent'\n");
    printf("  'Alexa, set [light name] to red'\n");
    printf("  'Alexa, set [light name] to warm white'\n");
    printf("  'Alexa, make [light name] warmer'\n");
    printf("  'Alexa, make [light name] cooler'\n");
    printf("================================================\n\n");

    // Initialize hardware
    init_pwm();

    // Main loop
    while (1) {
        // Process SinricPro events
        sinricpro_handle();

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
