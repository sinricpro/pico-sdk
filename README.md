# SinricPro SDK for Raspberry Pi Pico W

[![Build Examples](https://github.com/sinricpro/pico-sdk/workflows/Build%20Examples/badge.svg)](https://github.com/sinricpro/pico-sdk/actions)

Native C SDK for connecting Raspberry Pi Pico W devices to [SinricPro](https://sinric.pro) smart home platform.

## Features

- Native pico-sdk implementation (C11)
- WebSocket client with TLS support
- Compatible with Alexa, Google Home, and SinricPro app
- Low memory footprint
- Polling-based architecture (no RTOS required)

## Supported Devices

### Actuators
- **Switch** - Simple on/off control for relays, smart plugs, etc.
- **DimSwitch** - Dimmable switch with power level control (0-100%)
- **Light** - RGB+CCT smart light with full color and color temperature control
  - On/off control
  - Brightness adjustment (0-100%)
  - RGB color (0-255 per channel)
  - Color temperature (2200K-7000K warm to cool white)
- **Fan** - Speed control for fans and ventilation systems
  - On/off control
  - Speed adjustment (0-100%)
- **Lock** - Smart lock control for doors and access control
  - Lock/unlock commands
  - Status reporting (LOCKED/UNLOCKED/JAMMED)
- **GarageDoor** - Garage door opener control
  - Open/close commands
  - Position status monitoring
- **Blinds** - Motorized blinds/shades position control
  - Position control (0-100%, where 0=fully open, 100=fully closed)
  - Relative adjustments ("open blinds 30 percent")

### Sensors
- **Motion Sensor** - PIR motion detection for automation and security
- **Temperature Sensor** - Temperature and humidity monitoring
- **Contact Sensor** - Door/window open/close detection
- **PowerSensor** - Real-time power consumption monitoring
  - Voltage, current, power measurements
  - Watt-hours energy tracking
  - Power factor calculation
- **AirQualitySensor** - Air quality monitoring
  - PM1.0, PM2.5, PM10 particulate matter detection
  - Air quality index (AQI) reporting

### Event Devices
- **Doorbell** - Doorbell press notifications
  - Push notifications on doorbell press
  - Automation triggers

All devices support:
- Voice control via Alexa and Google Home
- SinricPro mobile app integration
- Alexa routines and automations
- Real-time status updates

### Future Device Support
The following complex devices require additional capabilities and are planned for future releases:
- **TV** - Television control (requires 8 capabilities)
- **Speaker** - Smart speaker control (requires 7 capabilities)
- **WindowAC** - Window air conditioner control (requires 5 capabilities)
- **Camera** - IP camera streaming (requires specialized camera capabilities)

## Documentation

- 📚 **[Quick Start Guide](docs/QUICK_START.md)** - Get started in 10 minutes
- 🔧 **[Troubleshooting](docs/TROUBLESHOOTING.md)** - Common issues and solutions
- 🤝 **[Contributing Guide](CONTRIBUTING.md)** - How to contribute
- 📖 **[API Reference](docs/)** - Detailed API documentation

## Requirements

- Raspberry Pi Pico W or Pico 2 W
- pico-sdk (1.5.0 or later)
- CMake 3.13+
- ARM GCC toolchain

## Quick Start

### 1. Clone and setup

```bash
# Clone the SDK
git clone https://github.com/sinricpro/pico-sdk.git
cd pico-sdk

# Add cJSON dependency
git submodule add https://github.com/DaveGamble/cJSON.git lib/cJSON
```

### 2. Set environment

```bash
export PICO_SDK_PATH=/path/to/pico-sdk
```

### 3. Build

```bash
mkdir build && cd build
cmake ..
make
```

### 4. Flash

Hold BOOTSEL button, connect Pico W via USB, then:
```bash
cp examples/switch/sinricpro_switch_example.uf2 /media/$USER/RPI-RP2/
```

## Example Code

```c
#include "sinricpro/sinricpro.h"
#include "sinricpro/sinricpro_switch.h"

sinricpro_switch_t my_switch;

bool on_power_state(sinricpro_device_t *device, bool *state) {
    printf("Power: %s\n", *state ? "ON" : "OFF");
    gpio_put(LED_PIN, *state);
    return true;
}

int main() {
    sinricpro_config_t config = {
        .app_key = "your-app-key",
        .app_secret = "your-app-secret"
    };

    sinricpro_init(&config);
    sinricpro_switch_init(&my_switch, "your-device-id");
    sinricpro_switch_on_power_state(&my_switch, on_power_state);
    sinricpro_add_device((sinricpro_device_t *)&my_switch);
    sinricpro_begin();

    while (1) {
        sinricpro_handle();
        sleep_ms(10);
    }
}
```

## Examples

The SDK includes complete examples for all device types:

### Actuators
- **`examples/switch/`** - Basic on/off relay control
- **`examples/dimswitch/`** - Dimmable LED with PWM control
- **`examples/light/`** - RGB+CCT LED strip with full color control
- **`examples/fan/`** - PWM fan speed control with automatic cycling
- **`examples/lock/`** - Solenoid lock control with manual unlock button
- **`examples/garagedoor/`** - Garage door relay control with position sensor
- **`examples/blinds/`** - Motorized blinds with time-based position tracking

### Sensors
- **`examples/motion_sensor/`** - PIR motion detection
- **`examples/temperature_sensor/`** - Temperature/humidity monitoring (uses built-in RP2040 sensor)
- **`examples/contact_sensor/`** - Door/window sensor with magnetic reed switch
- **`examples/powersensor/`** - Real-time power monitoring with ADC voltage/current sensing
- **`examples/airqualitysensor/`** - Air quality monitoring with I2C PM sensor

### Event Devices
- **`examples/doorbell/`** - Doorbell button with buzzer chime

Each example includes:
- Complete working code with detailed comments
- Hardware connection diagrams in comments
- Voice command examples
- Configuration instructions

## Getting Credentials

1. Create an account at [sinric.pro](https://sinric.pro)
2. Create a device in the dashboard
3. Copy your App Key, App Secret, and Device ID
4. Link your SinricPro account with Alexa or Google Home

## License

MIT License - see LICENSE file


## Community & Support

- 💬 **[Discord Community](https://discord.gg/rq9vcRcSqA)** - Chat with other users
- 🐛 **[Report Issues](https://github.com/sinricpro/pico-sdk/issues)** - Bug reports and feature requests
- 📧 **[Support](https://sinric.pro/support)** - Contact SinricPro support

## Links

- [SinricPro Portal](https://portal.sinric.pro)
- [SinricPro Documentation](https://help.sinric.pro)
- [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk)
