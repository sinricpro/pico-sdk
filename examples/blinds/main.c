/**
 * @file main.c
 * @brief SinricPro Blinds Example for Raspberry Pi Pico W
 *
 * This example demonstrates how to create smart blinds with position control
 * that can be controlled via Alexa, Google Home, or the SinricPro app.
 *
 * Hardware:
 * - Raspberry Pi Pico W
 * - Stepper motor or DC motor with encoder for position tracking
 * - Motor driver (L298N, TB6612, or similar)
 * - GPIO 15: PWM for motor speed control
 * - GPIO 14: Direction pin (HIGH=close, LOW=open)
 * - Button connected to GPIO 13 (optional, for manual control)
 *
 * Wiring Example (DC Motor with Driver):
 * - GPIO 15 -> Motor Driver PWM/Enable pin
 * - GPIO 14 -> Motor Driver Direction pin
 * - Motor Driver OUT1/OUT2 -> Motor terminals
 * - Motor Driver VCC -> External power supply (6-12V)
 * - Motor Driver GND -> Common ground with Pico
 * - Add flyback diode across motor terminals!
 *
 * Position Tracking:
 * - This example uses time-based position estimation
 * - For production use, add an encoder or limit switches for accurate position
 * - 0% = fully open, 100% = fully closed
 *
 * Setup:
 * 1. Create a "Blinds" device on sinric.pro and get your credentials
 * 2. Update WIFI_SSID, WIFI_PASSWORD, APP_KEY, APP_SECRET, DEVICE_ID
 * 3. Build and flash to your Pico W
 * 4. Control via:
 *    - "Alexa, open the blinds"
 *    - "Alexa, close the blinds"
 *    - "Alexa, set blinds to 50 percent"
 *    - "Hey Google, open the blinds halfway"
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
#include "sinricpro/sinricpro_blinds.h"

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

#define MOTOR_PWM_PIN   15  // GPIO for PWM motor speed control
#define MOTOR_DIR_PIN   14  // GPIO for motor direction (HIGH=close, LOW=open)
#define BUTTON_PIN      13  // GPIO for physical button input
#define DEBOUNCE_MS     50  // Button debounce time

// Motor timing configuration
#define MOTOR_SPEED     80  // Motor speed percentage (0-100%)
#define FULL_TRAVEL_MS  10000  // Time for full open->close travel (adjust for your blinds)

// =============================================================================
// Global Variables
// =============================================================================

static sinricpro_blinds_t my_blinds;
static bool current_power_state = false;
static int current_position = 0;  // 0-100% (0=open, 100=closed)
static int target_position = 0;
static uint32_t last_button_press = 0;

static uint pwm_slice_num;
static bool motor_moving = false;
static uint32_t move_start_time = 0;
static int move_start_position = 0;

// =============================================================================
// Callbacks
// =============================================================================

/**
 * @brief Handle power state change from cloud (Alexa/Google/App)
 *
 * This is called when a voice command or app toggles the blinds on/off.
 *
 * @param device The device receiving the command
 * @param state  Pointer to new state (can be modified)
 * @return true if successful, false on error
 */
bool on_power_state(sinricpro_device_t *device, bool *state) {
    printf("[Callback] Power state: %s\n", *state ? "ON" : "OFF");

    current_power_state = *state;

    // Update hardware
    if (!current_power_state) {
        // Turn off motor
        pwm_set_gpio_level(MOTOR_PWM_PIN, 0);
        motor_moving = false;
        printf("[Hardware] Motor stopped\n");
    }

    return true;
}

/**
 * @brief Handle position change from cloud
 *
 * This is called when a voice command sets the blind position.
 * Example: "Alexa, set blinds to 50 percent"
 *
 * @param device   The device receiving the command
 * @param position Pointer to new position 0-100 (can be modified)
 * @return true if successful, false on error
 */
bool on_range_value(sinricpro_device_t *device, int *position) {
    printf("[Callback] Position: %d%%\n", *position);

    // Clamp to valid range
    if (*position < 0) *position = 0;
    if (*position > 100) *position = 100;

    target_position = *position;
    current_power_state = true;  // Turn on when setting position

    // Start moving motor
    move_start_position = current_position;
    move_start_time = to_ms_since_boot(get_absolute_time());
    motor_moving = true;

    // Set direction
    bool closing = (target_position > current_position);
    gpio_put(MOTOR_DIR_PIN, closing);

    // Set motor speed
    uint16_t pwm_level = (MOTOR_SPEED * 65535) / 100;
    pwm_set_gpio_level(MOTOR_PWM_PIN, pwm_level);

    printf("[Hardware] Moving to %d%% (direction: %s)\n",
           target_position,
           closing ? "CLOSE" : "OPEN");

    return true;
}

/**
 * @brief Handle relative position adjustment
 *
 * This is called when a voice command adjusts the position relatively.
 * Example: "Alexa, open the blinds a little"
 *
 * @param device       The device receiving the command
 * @param range_delta  Pointer to delta value (can be modified)
 * @return true if successful, false on error
 */
bool on_adjust_range(sinricpro_device_t *device, int *range_delta) {
    printf("[Callback] Adjust range: %+d%%\n", *range_delta);

    // Calculate new position
    int new_position = current_position + *range_delta;

    // Clamp to valid range
    if (new_position < 0) new_position = 0;
    if (new_position > 100) new_position = 100;

    target_position = new_position;
    current_power_state = true;  // Turn on when adjusting

    // Start moving motor
    move_start_position = current_position;
    move_start_time = to_ms_since_boot(get_absolute_time());
    motor_moving = true;

    // Set direction
    bool closing = (target_position > current_position);
    gpio_put(MOTOR_DIR_PIN, closing);

    // Set motor speed
    uint16_t pwm_level = (MOTOR_SPEED * 65535) / 100;
    pwm_set_gpio_level(MOTOR_PWM_PIN, pwm_level);

    printf("[Hardware] Adjusting to %d%% (direction: %s)\n",
           target_position,
           closing ? "CLOSE" : "OPEN");

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
    // Initialize direction pin
    gpio_init(MOTOR_DIR_PIN);
    gpio_set_dir(MOTOR_DIR_PIN, GPIO_OUT);
    gpio_put(MOTOR_DIR_PIN, false);  // Start with open direction

    // Initialize button input with pull-up
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    // Initialize PWM for motor control
    gpio_set_function(MOTOR_PWM_PIN, GPIO_FUNC_PWM);
    pwm_slice_num = pwm_gpio_to_slice_num(MOTOR_PWM_PIN);

    // Set PWM frequency to ~25kHz (good for motor control)
    pwm_set_wrap(pwm_slice_num, 65535);
    pwm_set_clkdiv(pwm_slice_num, 2.0f);

    // Start with motor off
    pwm_set_gpio_level(MOTOR_PWM_PIN, 0);
    pwm_set_enabled(pwm_slice_num, true);

    printf("[Hardware] Motor controller initialized (PWM: GPIO %d, DIR: GPIO %d)\n",
           MOTOR_PWM_PIN, MOTOR_DIR_PIN);
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
 * @brief Update motor position based on time elapsed
 */
void update_motor_position(void) {
    if (!motor_moving) return;

    uint32_t now = to_ms_since_boot(get_absolute_time());
    uint32_t elapsed = now - move_start_time;

    // Calculate position change based on time
    int position_delta = target_position - move_start_position;
    int abs_delta = position_delta < 0 ? -position_delta : position_delta;

    // Time needed for this movement
    uint32_t move_time_needed = (abs_delta * FULL_TRAVEL_MS) / 100;

    if (elapsed >= move_time_needed) {
        // Movement complete
        current_position = target_position;
        pwm_set_gpio_level(MOTOR_PWM_PIN, 0);
        motor_moving = false;
        printf("[Hardware] Reached position %d%%\n", current_position);
    } else {
        // Still moving - update estimated position
        int progress = (elapsed * abs_delta) / move_time_needed;
        if (position_delta > 0) {
            current_position = move_start_position + progress;
        } else {
            current_position = move_start_position - progress;
        }
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
    printf("SinricPro Blinds Example for Pico W\n");
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

    // Initialize blinds device
    if (!sinricpro_blinds_init(&my_blinds, DEVICE_ID)) {
        printf("ERROR: Failed to initialize blinds device\n");
        return 1;
    }

    // Set callbacks
    sinricpro_blinds_on_power_state(&my_blinds, on_power_state);
    sinricpro_blinds_on_range_value(&my_blinds, on_range_value);
    sinricpro_blinds_on_adjust_range(&my_blinds, on_adjust_range);

    // Add device to SinricPro
    if (!sinricpro_add_device((sinricpro_device_t *)&my_blinds)) {
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
    printf("  'Alexa, open the blinds'\n");
    printf("  'Alexa, close the blinds'\n");
    printf("  'Alexa, set blinds to 50 percent'\n");
    printf("  'Hey Google, open the blinds halfway'\n");
    printf("\n");
    printf("Press the button to cycle positions.\n");
    printf("================================================\n\n");

    // Main loop
    while (1) {
        // Get current time
        uint32_t now = to_ms_since_boot(get_absolute_time());

        // Process SinricPro events
        sinricpro_handle();

        // Update motor position
        update_motor_position();

        // Check for physical button press
        if (check_button()) {
            // Cycle through positions: 0% -> 50% -> 100% -> 0%
            if (current_position < 50) {
                target_position = 50;
            } else if (current_position < 100) {
                target_position = 100;
            } else {
                target_position = 0;
            }

            current_power_state = true;

            // Start moving motor
            move_start_position = current_position;
            move_start_time = now;
            motor_moving = true;

            // Set direction
            bool closing = (target_position > current_position);
            gpio_put(MOTOR_DIR_PIN, closing);

            // Set motor speed
            uint16_t pwm_level = (MOTOR_SPEED * 65535) / 100;
            pwm_set_gpio_level(MOTOR_PWM_PIN, pwm_level);

            printf("[Button] Moving to %d%% (direction: %s)\n",
                   target_position,
                   closing ? "CLOSE" : "OPEN");

            // Send events to cloud
            if (sinricpro_is_connected()) {
                sinricpro_blinds_send_power_state_event(&my_blinds, current_power_state);
                sinricpro_blinds_send_range_value_event(&my_blinds, target_position);
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
