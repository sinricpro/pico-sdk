/**
 * @file main.c
 * @brief SinricPro Power Sensor Example for Raspberry Pi Pico W
 *
 * This example demonstrates how to create a power sensor that monitors
 * voltage, current, power, and power factor, reporting readings to SinricPro
 * for energy monitoring and automation.
 *
 * Hardware:
 * - Raspberry Pi Pico W
 * - Power measurement IC (e.g., ACS712 current sensor)
 * - ADC connections:
 *   - GPIO26 (ADC0): Voltage measurement
 *   - GPIO27 (ADC1): Current measurement
 *   - GPIO28 (ADC2): Optional power measurement
 *
 * This example simulates a power meter reading 230V AC mains with current sensing.
 * For actual power monitoring, connect appropriate voltage divider and current sensor.
 *
 * Alexa Usage:
 * - "Alexa, what's the power usage?"
 * - Create routines based on power consumption thresholds
 * - Monitor energy usage in real-time
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
#include "hardware/adc.h"
#include "hardware/gpio.h"

#include "sinricpro/sinricpro.h"
#include "sinricpro/sinricpro_powersensor.h"

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

// ADC pin assignments
#define VOLTAGE_ADC_PIN     26  // GPIO26 - ADC0
#define CURRENT_ADC_PIN     27  // GPIO27 - ADC1

// Calibration values (adjust based on your hardware)
#define VOLTAGE_SCALE       100.0f  // Voltage divider ratio
#define CURRENT_SCALE       10.0f   // Current sensor sensitivity (A/V)
#define MAINS_VOLTAGE       230.0f  // Nominal mains voltage (adjust for your region)

// =============================================================================
// Global Variables
// =============================================================================

static sinricpro_powersensor_t my_power_sensor;

// Power measurement structure
typedef struct {
    float voltage;          // RMS voltage (V)
    float current;          // RMS current (A)
    float power;            // Active power (W)
    float apparent_power;   // Apparent power (VA)
    float reactive_power;   // Reactive power (VAR)
    float power_factor;     // Power factor (0-1)
} power_measurement_t;

// =============================================================================
// Sensor Functions
// =============================================================================

/**
 * @brief Initialize the power sensor hardware
 */
void init_sensor(void) {
    // Initialize ADC
    adc_init();

    // Initialize ADC pins
    adc_gpio_init(VOLTAGE_ADC_PIN);
    adc_gpio_init(CURRENT_ADC_PIN);

    printf("[Sensor] Power sensor initialized\n");
    printf("[Sensor] Voltage ADC: GPIO%d, Current ADC: GPIO%d\n",
           VOLTAGE_ADC_PIN, CURRENT_ADC_PIN);
}

/**
 * @brief Read voltage from ADC
 *
 * In a real implementation, this would read from a voltage divider circuit.
 * This example simulates 230V AC mains voltage.
 */
float read_voltage(void) {
    // For this example, we'll simulate readings
    // In real hardware:
    // adc_select_input(VOLTAGE_ADC_PIN - 26);  // ADC0
    // uint16_t raw = adc_read();
    // float voltage = (raw * 3.3f / 4096.0f) * VOLTAGE_SCALE;

    // Simulate 230V +/- small variation
    return MAINS_VOLTAGE + ((rand() % 20) - 10) * 0.1f;
}

/**
 * @brief Read current from ADC
 *
 * In a real implementation, this would read from a current sensor (e.g., ACS712).
 * This example simulates a typical household load.
 */
float read_current(void) {
    // For this example, we'll simulate readings
    // In real hardware:
    // adc_select_input(CURRENT_ADC_PIN - 26);  // ADC1
    // uint16_t raw = adc_read();
    // float voltage = raw * 3.3f / 4096.0f;
    // float current = (voltage - 1.65f) * CURRENT_SCALE;  // ACS712 outputs 1.65V at 0A

    // Simulate varying current load (0.3A to 0.7A)
    return 0.5f + ((rand() % 40) - 20) * 0.01f;
}

/**
 * @brief Perform power measurement
 *
 * Calculates all power-related values from voltage and current measurements.
 */
void measure_power(power_measurement_t *measurement) {
    // Read voltage and current
    measurement->voltage = read_voltage();
    measurement->current = read_current();

    // Calculate active power (W)
    // For AC power: P = V * I * power_factor
    // We'll assume a typical power factor of 0.95
    measurement->power_factor = 0.95f;
    measurement->power = measurement->voltage * measurement->current * measurement->power_factor;

    // Calculate apparent power (VA)
    measurement->apparent_power = measurement->voltage * measurement->current;

    // Calculate reactive power (VAR)
    // Q = sqrt(S^2 - P^2)
    float s_squared = measurement->apparent_power * measurement->apparent_power;
    float p_squared = measurement->power * measurement->power;
    measurement->reactive_power = sqrtf(s_squared - p_squared);
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
    printf("SinricPro Power Sensor Example\n");
    printf("Monitoring AC power consumption\n");
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

    // Initialize power sensor device
    if (!sinricpro_powersensor_init(&my_power_sensor, DEVICE_ID)) {
        printf("ERROR: Failed to initialize power sensor device\n");
        return 1;
    }

    // Note: Power sensors are event-only devices, no callbacks needed

    // Add device to SinricPro
    if (!sinricpro_add_device((sinricpro_device_t *)&my_power_sensor)) {
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
    printf("Ready! Monitoring power every 60 seconds.\n");
    printf("\n");
    printf("Voice Commands:\n");
    printf("  'Alexa, what's the power usage?'\n");
    printf("================================================\n\n");

    // Initialize sensor hardware
    init_sensor();

    // Main loop
    uint32_t last_report = 0;

    while (1) {
        // Process SinricPro events
        sinricpro_handle();

        uint32_t now = to_ms_since_boot(get_absolute_time());

        // Report power measurements every REPORT_INTERVAL_MS
        if (now - last_report >= REPORT_INTERVAL_MS) {
            last_report = now;

            // Read sensor
            power_measurement_t measurement;
            measure_power(&measurement);

            printf("[Sensor] Voltage: %.1fV, Current: %.2fA, Power: %.1fW\n",
                   measurement.voltage, measurement.current, measurement.power);
            printf("[Sensor] Apparent: %.1fVA, Reactive: %.1fVAR, PF: %.2f\n",
                   measurement.apparent_power, measurement.reactive_power,
                   measurement.power_factor);

            // Send event to SinricPro (only if connected)
            if (sinricpro_is_connected()) {
                if (sinricpro_powersensor_send_power_event(&my_power_sensor,
                                                          measurement.voltage,
                                                          measurement.current,
                                                          measurement.power,
                                                          measurement.apparent_power,
                                                          measurement.reactive_power,
                                                          measurement.power_factor)) {
                    printf("[Sensor] Power event sent successfully\n");
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
