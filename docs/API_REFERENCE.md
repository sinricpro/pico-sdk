# API Reference

Complete API reference for SinricPro Pico SDK.

## Table of Contents

- [Core API](#core-api)
- [Device Types](#device-types)
- [Capabilities](#capabilities)
- [Common Patterns](#common-patterns)

---

## Core API

### Initialization

```c
#include "sinricpro/sinricpro.h"

sinricpro_config_t config = {
    .app_key = "your-app-key",
    .app_secret = "your-app-secret",
    .enable_debug = false
};

bool sinricpro_init(sinricpro_config_t *config);
bool sinricpro_begin(void);
void sinricpro_handle(void);
```

| Function | Description | Returns |
|----------|-------------|---------|
| `sinricpro_init()` | Initialize SDK with credentials | `true` on success |
| `sinricpro_begin()` | Connect to SinricPro server | `true` on success |
| `sinricpro_handle()` | Process events (call in loop) | N/A |

### Device Management

```c
bool sinricpro_add_device(sinricpro_device_t *device);
bool sinricpro_is_connected(void);
```

| Function | Description | Returns |
|----------|-------------|---------|
| `sinricpro_add_device()` | Register a device | `true` on success |
| `sinricpro_is_connected()` | Check connection status | `true` if connected |

### State Callbacks

```c
typedef enum {
    SINRICPRO_STATE_DISCONNECTED,
    SINRICPRO_STATE_WIFI_CONNECTING,
    SINRICPRO_STATE_WIFI_CONNECTED,
    SINRICPRO_STATE_WS_CONNECTING,
    SINRICPRO_STATE_CONNECTED,
    SINRICPRO_STATE_ERROR
} sinricpro_state_t;

typedef void (*sinricpro_state_callback_t)(sinricpro_state_t state, void *user_data);

void sinricpro_on_state_change(sinricpro_state_callback_t callback, void *user_data);
```

---

## Device Types

### Switch

Simple on/off control.

```c
#include "sinricpro/sinricpro_switch.h"

sinricpro_switch_t my_switch;

// Initialize
bool sinricpro_switch_init(sinricpro_switch_t *device, const char *device_id);

// Set callback
typedef bool (*sinricpro_power_state_callback_t)(sinricpro_device_t *device, bool *state);
void sinricpro_switch_on_power_state(sinricpro_switch_t *device,
                                     sinricpro_power_state_callback_t callback);

// Send events
bool sinricpro_switch_send_power_state_event(sinricpro_switch_t *device, bool state);

// Get state
bool sinricpro_switch_get_power_state(const sinricpro_switch_t *device);
```

### DimSwitch

On/off with power level control (0-100%).

```c
#include "sinricpro/sinricpro_dimswitch.h"

sinricpro_dimswitch_t my_dimmer;

// Initialize
bool sinricpro_dimswitch_init(sinricpro_dimswitch_t *device, const char *device_id);

// Set callbacks
void sinricpro_dimswitch_on_power_state(sinricpro_dimswitch_t *device,
                                        sinricpro_power_state_callback_t callback);

typedef bool (*sinricpro_power_level_callback_t)(sinricpro_device_t *device, int *power_level);
void sinricpro_dimswitch_on_power_level(sinricpro_dimswitch_t *device,
                                        sinricpro_power_level_callback_t callback);

typedef bool (*sinricpro_adjust_power_level_callback_t)(sinricpro_device_t *device, int *delta);
void sinricpro_dimswitch_on_adjust_power_level(sinricpro_dimswitch_t *device,
                                               sinricpro_adjust_power_level_callback_t callback);

// Send events
bool sinricpro_dimswitch_send_power_state_event(sinricpro_dimswitch_t *device, bool state);
bool sinricpro_dimswitch_send_power_level_event(sinricpro_dimswitch_t *device, int power_level);

// Get state
bool sinricpro_dimswitch_get_power_state(const sinricpro_dimswitch_t *device);
int sinricpro_dimswitch_get_power_level(const sinricpro_dimswitch_t *device);
```

### Light

Full RGB+CCT smart light.

```c
#include "sinricpro/sinricpro_light.h"

sinricpro_light_t my_light;

// Initialize
bool sinricpro_light_init(sinricpro_light_t *device, const char *device_id);

// Set callbacks
void sinricpro_light_on_power_state(sinricpro_light_t *device,
                                    sinricpro_power_state_callback_t callback);

typedef bool (*sinricpro_brightness_callback_t)(sinricpro_device_t *device, int *brightness);
void sinricpro_light_on_brightness(sinricpro_light_t *device,
                                   sinricpro_brightness_callback_t callback);

typedef bool (*sinricpro_adjust_brightness_callback_t)(sinricpro_device_t *device, int *delta);
void sinricpro_light_on_adjust_brightness(sinricpro_light_t *device,
                                          sinricpro_adjust_brightness_callback_t callback);

typedef struct {
    uint8_t r, g, b;  // 0-255
} sinricpro_color_t;

typedef bool (*sinricpro_color_callback_t)(sinricpro_device_t *device, sinricpro_color_t *color);
void sinricpro_light_on_color(sinricpro_light_t *device,
                              sinricpro_color_callback_t callback);

typedef bool (*sinricpro_color_temp_callback_t)(sinricpro_device_t *device, int *color_temp);
void sinricpro_light_on_color_temperature(sinricpro_light_t *device,
                                          sinricpro_color_temp_callback_t callback);

typedef bool (*sinricpro_color_temp_adjust_callback_t)(sinricpro_device_t *device, int *delta);
void sinricpro_light_on_increase_color_temperature(sinricpro_light_t *device,
                                                   sinricpro_color_temp_adjust_callback_t callback);
void sinricpro_light_on_decrease_color_temperature(sinricpro_light_t *device,
                                                   sinricpro_color_temp_adjust_callback_t callback);

// Send events
bool sinricpro_light_send_power_state_event(sinricpro_light_t *device, bool state);
bool sinricpro_light_send_brightness_event(sinricpro_light_t *device, int brightness);
bool sinricpro_light_send_color_event(sinricpro_light_t *device, sinricpro_color_t color);
bool sinricpro_light_send_color_temp_event(sinricpro_light_t *device, int color_temp);

// Get state
bool sinricpro_light_get_power_state(const sinricpro_light_t *device);
int sinricpro_light_get_brightness(const sinricpro_light_t *device);
sinricpro_color_t sinricpro_light_get_color(const sinricpro_light_t *device);
int sinricpro_light_get_color_temp(const sinricpro_light_t *device);
```

### Motion Sensor

PIR motion detection (event-only).

```c
#include "sinricpro/sinricpro_motion_sensor.h"

sinricpro_motion_sensor_t my_sensor;

// Initialize
bool sinricpro_motion_sensor_init(sinricpro_motion_sensor_t *device, const char *device_id);

// Send events
bool sinricpro_motion_sensor_send_event(sinricpro_motion_sensor_t *device, bool detected);
```

### Temperature Sensor

Temperature and humidity monitoring (event-only).

```c
#include "sinricpro/sinricpro_temperature_sensor.h"

sinricpro_temperature_sensor_t my_sensor;

// Initialize
bool sinricpro_temperature_sensor_init(sinricpro_temperature_sensor_t *device,
                                       const char *device_id);

// Send events
bool sinricpro_temperature_sensor_send_event(sinricpro_temperature_sensor_t *device,
                                             float temperature,
                                             float humidity);
```

### Contact Sensor

Door/window open/close detection (event-only).

```c
#include "sinricpro/sinricpro_contact_sensor.h"

sinricpro_contact_sensor_t my_sensor;

// Initialize
bool sinricpro_contact_sensor_init(sinricpro_contact_sensor_t *device, const char *device_id);

// Send events
bool sinricpro_contact_sensor_send_event(sinricpro_contact_sensor_t *device, bool is_open);
```

---

## Capabilities

Capabilities are the building blocks that devices use. You typically don't use these directly unless creating custom device types.

### PowerState

```c
#include "sinricpro/capabilities/power_state.h"

sinricpro_power_state_t power_state;

void sinricpro_power_state_init(sinricpro_power_state_t *power_state);
void sinricpro_power_state_set_callback(sinricpro_power_state_t *power_state,
                                        sinricpro_power_state_callback_t callback);
bool sinricpro_power_state_send_event(sinricpro_power_state_t *power_state,
                                      const char *device_id,
                                      bool state);
bool sinricpro_power_state_get_state(const sinricpro_power_state_t *power_state);
```

### Brightness

```c
#include "sinricpro/capabilities/brightness.h"

sinricpro_brightness_t brightness;

void sinricpro_brightness_init(sinricpro_brightness_t *brightness);
void sinricpro_brightness_set_callback(sinricpro_brightness_t *brightness,
                                       sinricpro_brightness_callback_t callback);
void sinricpro_brightness_set_adjust_callback(sinricpro_brightness_t *brightness,
                                              sinricpro_adjust_brightness_callback_t callback);
bool sinricpro_brightness_send_event(sinricpro_brightness_t *brightness,
                                     const char *device_id,
                                     int brightness_value);
int sinricpro_brightness_get_value(const sinricpro_brightness_t *brightness);
```

### PowerLevel

```c
#include "sinricpro/capabilities/power_level.h"

sinricpro_power_level_t power_level;

void sinricpro_power_level_init(sinricpro_power_level_t *power_level);
void sinricpro_power_level_set_callback(sinricpro_power_level_t *power_level,
                                        sinricpro_power_level_callback_t callback);
void sinricpro_power_level_set_adjust_callback(sinricpro_power_level_t *power_level,
                                               sinricpro_adjust_power_level_callback_t callback);
bool sinricpro_power_level_send_event(sinricpro_power_level_t *power_level,
                                      const char *device_id,
                                      int level);
int sinricpro_power_level_get_value(const sinricpro_power_level_t *power_level);
```

### Color

```c
#include "sinricpro/capabilities/color.h"

typedef struct {
    uint8_t r, g, b;  // 0-255
} sinricpro_color_t;

sinricpro_color_cap_t color_cap;

void sinricpro_color_init(sinricpro_color_cap_t *cap);
void sinricpro_color_set_callback(sinricpro_color_cap_t *cap,
                                  sinricpro_color_callback_t callback);
bool sinricpro_color_send_event(sinricpro_color_cap_t *cap,
                                const char *device_id,
                                sinricpro_color_t color);
sinricpro_color_t sinricpro_color_get_value(const sinricpro_color_cap_t *cap);
```

### ColorTemperature

```c
#include "sinricpro/capabilities/color_temperature.h"

sinricpro_color_temp_cap_t color_temp;

void sinricpro_color_temp_init(sinricpro_color_temp_cap_t *cap);
void sinricpro_color_temp_set_callback(sinricpro_color_temp_cap_t *cap,
                                       sinricpro_color_temp_callback_t callback);
void sinricpro_color_temp_set_increase_callback(sinricpro_color_temp_cap_t *cap,
                                                sinricpro_color_temp_adjust_callback_t callback);
void sinricpro_color_temp_set_decrease_callback(sinricpro_color_temp_cap_t *cap,
                                                sinricpro_color_temp_adjust_callback_t callback);
bool sinricpro_color_temp_send_event(sinricpro_color_temp_cap_t *cap,
                                     const char *device_id,
                                     int color_temp);
int sinricpro_color_temp_get_value(const sinricpro_color_temp_cap_t *cap);
```

---

## Common Patterns

### Basic Device Setup

```c
// 1. Include headers
#include "sinricpro/sinricpro.h"
#include "sinricpro/sinricpro_switch.h"

// 2. Declare device
sinricpro_switch_t my_switch;

// 3. Create callback
bool on_power_state(sinricpro_device_t *device, bool *state) {
    printf("Power: %s\n", *state ? "ON" : "OFF");
    // Control hardware here
    return true;  // Return true on success
}

// 4. In main()
int main() {
    // Initialize SinricPro
    sinricpro_config_t config = {
        .app_key = APP_KEY,
        .app_secret = APP_SECRET,
        .enable_debug = false
    };
    sinricpro_init(&config);

    // Initialize device
    sinricpro_switch_init(&my_switch, DEVICE_ID);
    sinricpro_switch_on_power_state(&my_switch, on_power_state);
    sinricpro_add_device((sinricpro_device_t *)&my_switch);

    // Connect
    sinricpro_begin();

    // Main loop
    while (1) {
        sinricpro_handle();
        sleep_ms(10);
    }
}
```

### Sending Events

```c
// Physical button pressed
if (button_pressed) {
    current_state = !current_state;

    // Update hardware
    gpio_put(LED_PIN, current_state);

    // Send event to cloud
    if (sinricpro_is_connected()) {
        sinricpro_switch_send_power_state_event(&my_switch, current_state);
    }
}
```

### Multiple Devices

```c
sinricpro_switch_t switch1, switch2;
sinricpro_dimswitch_t dimmer1;

// Initialize all devices
sinricpro_switch_init(&switch1, DEVICE_ID_1);
sinricpro_switch_init(&switch2, DEVICE_ID_2);
sinricpro_dimswitch_init(&dimmer1, DEVICE_ID_3);

// Set callbacks
sinricpro_switch_on_power_state(&switch1, on_switch1_power);
sinricpro_switch_on_power_state(&switch2, on_switch2_power);
sinricpro_dimswitch_on_power_state(&dimmer1, on_dimmer_power);
sinricpro_dimswitch_on_power_level(&dimmer1, on_dimmer_level);

// Add all devices
sinricpro_add_device((sinricpro_device_t *)&switch1);
sinricpro_add_device((sinricpro_device_t *)&switch2);
sinricpro_add_device((sinricpro_device_t *)&dimmer1);
```

### State Monitoring

```c
void on_state_change(sinricpro_state_t state, void *user_data) {
    switch (state) {
        case SINRICPRO_STATE_DISCONNECTED:
            printf("Disconnected\n");
            break;
        case SINRICPRO_STATE_WIFI_CONNECTING:
            printf("Connecting to WiFi...\n");
            break;
        case SINRICPRO_STATE_WIFI_CONNECTED:
            printf("WiFi connected\n");
            break;
        case SINRICPRO_STATE_WS_CONNECTING:
            printf("Connecting to SinricPro...\n");
            break;
        case SINRICPRO_STATE_CONNECTED:
            printf("Connected!\n");
            break;
        case SINRICPRO_STATE_ERROR:
            printf("Error occurred\n");
            break;
    }
}

// Register callback
sinricpro_on_state_change(on_state_change, NULL);
```

### Error Handling

```c
// Check initialization
if (!sinricpro_init(&config)) {
    printf("Failed to initialize SinricPro\n");
    return 1;
}

// Check device init
if (!sinricpro_switch_init(&my_switch, DEVICE_ID)) {
    printf("Failed to initialize switch\n");
    return 1;
}

// Check connection
if (!sinricpro_begin()) {
    printf("Failed to connect\n");
    return 1;
}

// Check event sending
if (!sinricpro_switch_send_power_state_event(&my_switch, true)) {
    printf("Failed to send event (rate limited or not connected)\n");
}
```

---

## Device Type Constants

```c
// From sinricpro_device.h
typedef enum {
    SINRICPRO_DEVICE_TYPE_SWITCH,
    SINRICPRO_DEVICE_TYPE_DIMSWITCH,
    SINRICPRO_DEVICE_TYPE_LIGHT,
    SINRICPRO_DEVICE_TYPE_MOTION_SENSOR,
    SINRICPRO_DEVICE_TYPE_TEMPERATURE_SENSOR,
    SINRICPRO_DEVICE_TYPE_CONTACT_SENSOR
} sinricpro_device_type_t;
```

---

## Rate Limits

- **State change events**: 1 per second maximum
- **Sensor readings**: 1 per 60 seconds maximum
- **Commands**: No limit (server-initiated)

Events sent more frequently will be dropped by the rate limiter.

---

## Memory Considerations

- Each device: ~100 bytes
- Message queue: ~16KB (8 messages × 2KB each)
- TLS buffers: ~32KB (can be disabled with `SINRICPRO_NOSSL`)
- Total SDK overhead: ~50-60KB with TLS, ~20-30KB without

---

## See Also

- [Examples](../examples/) - Working code examples
- [Quick Start Guide](QUICK_START.md) - Getting started
- [Troubleshooting](TROUBLESHOOTING.md) - Common issues
