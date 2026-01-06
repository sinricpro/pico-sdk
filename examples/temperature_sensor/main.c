/**
 * @file main.c
 * @brief SinricPro Temperature Sensor Example for Raspberry Pi Pico W
 *
 * This example demonstrates how to create a temperature and humidity sensor
 * that reports readings to SinricPro for monitoring and automation.
 *
 * Hardware:
 * - Raspberry Pi Pico W
 * - Option 1: Use built-in temperature sensor (RP2040 ADC4)
 * - Option 2: Connect DHT22, AHT10, or similar I2C/GPIO sensor
 *
 * This example uses the built-in RP2040 temperature sensor.
 * For external sensors (DHT22, AHT10, etc.), replace the read_sensor() function.
 *
 * Alexa Usage:
 * - "Alexa, what's the temperature in [room name]?"
 * - "Alexa, what's the humidity in [room name]?"
 * - Create routines based on temperature thresholds
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
#include "hardware/adc.h"

#include "sinricpro/sinricpro.h"
#include "sinricpro/sinricpro_temperature_sensor.h"

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
// Sensor Configuration
// =============================================================================

#define REPORT_INTERVAL_MS  60000  // Report every 60 seconds (minimum allowed)

// =============================================================================
// Global Variables
// =============================================================================

static sinricpro_temperature_sensor_t my_temp_sensor;

// =============================================================================
// Sensor Functions
// =============================================================================

/**
 * @brief Initialize the temperature sensor hardware
 */
void init_sensor(void) {
    // Initialize ADC for built-in temperature sensor
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);  // Select ADC4 (internal temperature sensor)
}

/**
 * @brief Read temperature from RP2040 built-in sensor
 *
 * @param temperature Output: temperature in Celsius
 * @param humidity    Output: humidity percentage (simulated for this example)
 */
void read_sensor(float *temperature, float *humidity) {
    // Read raw ADC value from temperature sensor
    uint16_t adc_raw = adc_read();

    // Convert to voltage (3.3V reference, 12-bit ADC)
    float voltage = adc_raw * 3.3f / 4096.0f;

    // Convert to temperature (RP2040 datasheet formula)
    // T = 27 - (ADC_voltage - 0.706) / 0.001721
    *temperature = 27.0f - (voltage - 0.706f) / 0.001721f;

    // Simulate humidity (replace with actual sensor reading if available)
    // For DHT22 or AHT10, read actual humidity here
    *humidity = 50.0f;
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
    printf("SinricPro Temperature Sensor Example\n");
    printf("Using RP2040 built-in temperature sensor\n");
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

    // Initialize temperature sensor device
    if (!sinricpro_temperature_sensor_init(&my_temp_sensor, DEVICE_ID)) {
        printf("ERROR: Failed to initialize temperature sensor device\n");
        return 1;
    }

    // Note: Temperature sensors are event-only devices, no callbacks needed

    // Add device to SinricPro
    if (!sinricpro_add_device((sinricpro_device_t *)&my_temp_sensor)) {
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
    printf("Ready! Reporting temperature every 60 seconds.\n");
    printf("\n");
    printf("Voice Commands:\n");
    printf("  'Alexa, what's the temperature?'\n");
    printf("  'Alexa, what's the humidity?'\n");
    printf("================================================\n\n");

    // Initialize sensor hardware
    init_sensor();

    // Main loop
    uint32_t last_report = 0;

    while (1) {
        // Process SinricPro events
        sinricpro_handle();

        uint32_t now = to_ms_since_boot(get_absolute_time());

        // Report temperature/humidity every REPORT_INTERVAL_MS
        if (now - last_report >= REPORT_INTERVAL_MS) {
            last_report = now;

            // Read sensor
            float temperature, humidity;
            read_sensor(&temperature, &humidity);

            printf("[Sensor] Temperature: %.1f°C, Humidity: %.1f%%\n",
                   temperature, humidity);

            // Send event to SinricPro (only if connected)
            if (sinricpro_is_connected()) {
                if (sinricpro_temperature_sensor_send_event(&my_temp_sensor,
                                                            temperature,
                                                            humidity)) {
                    printf("[Sensor] Event sent successfully\n");
                } else {
                    printf("[Sensor] Failed to send event (rate limited)\n");
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

        // Small delay to prevent CPU hogging
        sleep_ms(100);
    }

    return 0;
}
