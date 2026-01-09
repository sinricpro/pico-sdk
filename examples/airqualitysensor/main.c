/**
 * @file main.c
 * @brief SinricPro Air Quality Sensor Example for Raspberry Pi Pico W
 *
 * This example demonstrates how to create an air quality sensor that monitors
 * particulate matter (PM1.0, PM2.5, PM10) and reports readings to SinricPro
 * for air quality monitoring and automation.
 *
 * Hardware:
 * - Raspberry Pi Pico W
 * - PM sensor (e.g., PMS5003, SDS011, GP2Y1014AU0F)
 * - I2C connections:
 *   - GPIO4: I2C0 SDA
 *   - GPIO5: I2C0 SCL
 *
 * This example simulates a PM sensor reading typical indoor air quality values.
 * For actual air quality monitoring, connect a compatible PM sensor via I2C or UART.
 *
 * PM Values (μg/m³):
 * - PM1.0:  Particles < 1.0 micrometers
 * - PM2.5:  Particles < 2.5 micrometers (most health-relevant)
 * - PM10:   Particles < 10 micrometers
 *
 * Alexa Usage:
 * - "Alexa, what's the air quality?"
 * - Create routines based on air quality thresholds
 * - Monitor PM2.5 levels for health alerts
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
#include "hardware/i2c.h"
#include "hardware/gpio.h"

#include "sinricpro/sinricpro.h"
#include "sinricpro/sinricpro_airqualitysensor.h"

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

// I2C configuration
#define I2C_PORT        i2c0
#define I2C_SDA_PIN     4
#define I2C_SCL_PIN     5
#define I2C_BAUDRATE    100000  // 100 kHz

// =============================================================================
// Global Variables
// =============================================================================

static sinricpro_airqualitysensor_t my_air_sensor;

// Air quality measurement structure
typedef struct {
    int pm1_0;      // PM1.0 concentration (μg/m³)
    int pm2_5;      // PM2.5 concentration (μg/m³)
    int pm10;       // PM10 concentration (μg/m³)
} air_quality_t;

// =============================================================================
// Sensor Functions
// =============================================================================

/**
 * @brief Initialize the I2C bus for air quality sensor
 */
void init_sensor(void) {
    // Initialize I2C
    i2c_init(I2C_PORT, I2C_BAUDRATE);

    // Set up GPIO pins for I2C
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);

    // Enable internal pull-ups (optional if external pull-ups are present)
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    printf("[Sensor] Air quality sensor initialized\n");
    printf("[Sensor] I2C: SDA=GPIO%d, SCL=GPIO%d, Baudrate=%d Hz\n",
           I2C_SDA_PIN, I2C_SCL_PIN, I2C_BAUDRATE);
}

/**
 * @brief Read air quality data from PM sensor
 *
 * In a real implementation, this would read from a PM sensor (PMS5003, SDS011, etc.)
 * via I2C or UART protocol.
 *
 * This example simulates typical indoor air quality readings.
 *
 * @param data Output structure to store PM values
 */
void read_air_quality(air_quality_t *data) {
    // For this example, we'll simulate readings
    // In real hardware, you would:
    // 1. Send read command to sensor via I2C
    // 2. Wait for data to be ready
    // 3. Read PM data from sensor registers
    //
    // Example for PMS5003 (UART-based):
    // - Send wake-up command
    // - Read 32-byte data frame
    // - Parse PM1.0, PM2.5, PM10 values
    //
    // Example for I2C-based sensor:
    // uint8_t buffer[12];
    // i2c_read_blocking(I2C_PORT, SENSOR_ADDR, buffer, sizeof(buffer), false);
    // data->pm1_0 = (buffer[0] << 8) | buffer[1];
    // data->pm2_5 = (buffer[2] << 8) | buffer[3];
    // data->pm10 = (buffer[4] << 8) | buffer[5];

    // Simulate typical indoor air quality values (μg/m³)
    // Good air quality: PM2.5 < 12, Moderate: 12-35, Unhealthy: > 35
    data->pm1_0 = 8 + (rand() % 5);      // 8-12 μg/m³
    data->pm2_5 = 20 + (rand() % 10);    // 20-29 μg/m³
    data->pm10 = 45 + (rand() % 15);     // 45-59 μg/m³
}

/**
 * @brief Get air quality index description
 */
const char* get_aqi_description(int pm2_5) {
    if (pm2_5 <= 12) return "Good";
    if (pm2_5 <= 35) return "Moderate";
    if (pm2_5 <= 55) return "Unhealthy for Sensitive";
    if (pm2_5 <= 150) return "Unhealthy";
    if (pm2_5 <= 250) return "Very Unhealthy";
    return "Hazardous";
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
    printf("SinricPro Air Quality Sensor Example\n");
    printf("Monitoring PM1.0, PM2.5, and PM10\n");
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

    // Initialize air quality sensor device
    if (!sinricpro_airqualitysensor_init(&my_air_sensor, DEVICE_ID)) {
        printf("ERROR: Failed to initialize air quality sensor device\n");
        return 1;
    }

    // Note: Air quality sensors are event-only devices, no callbacks needed

    // Add device to SinricPro
    if (!sinricpro_add_device((sinricpro_device_t *)&my_air_sensor)) {
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
    printf("Ready! Monitoring air quality every 60 seconds.\n");
    printf("\n");
    printf("Voice Commands:\n");
    printf("  'Alexa, what's the air quality?'\n");
    printf("\n");
    printf("Air Quality Index (based on PM2.5):\n");
    printf("  0-12:   Good\n");
    printf("  12-35:  Moderate\n");
    printf("  35-55:  Unhealthy for Sensitive Groups\n");
    printf("  55-150: Unhealthy\n");
    printf("  150+:   Very Unhealthy/Hazardous\n");
    printf("================================================\n\n");

    // Initialize sensor hardware
    init_sensor();

    // Main loop
    uint32_t last_report = 0;

    while (1) {
        // Process SinricPro events
        sinricpro_handle();

        uint32_t now = to_ms_since_boot(get_absolute_time());

        // Report air quality every REPORT_INTERVAL_MS
        if (now - last_report >= REPORT_INTERVAL_MS) {
            last_report = now;

            // Read sensor
            air_quality_t air_data;
            read_air_quality(&air_data);

            const char *aqi = get_aqi_description(air_data.pm2_5);

            printf("[Sensor] PM1.0: %d μg/m³, PM2.5: %d μg/m³, PM10: %d μg/m³\n",
                   air_data.pm1_0, air_data.pm2_5, air_data.pm10);
            printf("[Sensor] Air Quality: %s\n", aqi);

            // Send event to SinricPro (only if connected)
            if (sinricpro_is_connected()) {
                if (sinricpro_airqualitysensor_send_event(&my_air_sensor,
                                                          air_data.pm1_0,
                                                          air_data.pm2_5,
                                                          air_data.pm10)) {
                    printf("[Sensor] Air quality event sent successfully\n");
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
