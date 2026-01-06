# Quick Start Guide

This guide will help you get your first SinricPro device running on Raspberry Pi Pico W in under 10 minutes.

## Prerequisites

- Raspberry Pi Pico W (or Pico 2 W)
- USB cable
- WiFi network credentials
- [SinricPro account](https://sinric.pro) (free)

## Step 1: Get Your Credentials

1. Sign up at [sinric.pro](https://sinric.pro)
2. Create a new device:
   - Click "Devices" → "Add Device"
   - Select device type (e.g., "Switch")
   - Give it a name
   - Click "Save"
3. Copy your credentials:
   - **App Key**: Found in "Credentials" section
   - **App Secret**: Found in "Credentials" section
   - **Device ID**: 24-character ID shown on device card

## Step 2: Download Pre-built Firmware

Visit the [Releases](https://github.com/sinricpro/pico-sdk/releases) page and download the latest `.uf2` file for your device type.

**OR** build from source (see below).

## Step 3: Flash Your Pico W

1. Hold the **BOOTSEL** button on your Pico W
2. Connect it to your computer via USB (while holding BOOTSEL)
3. Release BOOTSEL - a drive named `RPI-RP2` will appear
4. Copy the `.uf2` file to the `RPI-RP2` drive
5. Pico W will automatically reboot

## Step 4: Configure WiFi and Credentials

Since credentials are baked into the firmware during build, you need to build from source with your credentials.

### Building from Source

#### Install Dependencies (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y cmake gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential git
```

#### Install pico-sdk

```bash
cd ~
git clone https://github.com/raspberrypi/pico-sdk.git --branch 2.2.0
cd pico-sdk
git submodule update --init
export PICO_SDK_PATH=~/pico-sdk
# Add to ~/.bashrc to make permanent
echo 'export PICO_SDK_PATH=~/pico-sdk' >> ~/.bashrc
```

#### Clone and Build

```bash
# Clone SinricPro SDK
git clone https://github.com/sinricpro/pico-sdk.git sinricpro-pico
cd sinricpro-pico

# Initialize submodules
git submodule add https://github.com/DaveGamble/cJSON.git lib/cJSON
git submodule update --init --recursive

# Edit example with your credentials
nano examples/switch/main.c
```

Update these lines with your credentials:

```c
#define WIFI_SSID       "YOUR_WIFI_SSID"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"
#define APP_KEY         "your-app-key-from-sinricpro"
#define APP_SECRET      "your-app-secret-from-sinricpro"
#define DEVICE_ID       "your-24-char-device-id"
```

Save and build:

```bash
# Build
cmake -B build -DPICO_BOARD=pico_w
cmake --build build -j$(nproc)

# Flash
# Hold BOOTSEL, connect Pico W, then:
cp build/examples/switch/sinricpro_switch_example.uf2 /media/$USER/RPI-RP2/
```

## Step 5: Test the Connection

1. **Open Serial Monitor**
   ```bash
   # Find your Pico's serial port
   ls /dev/ttyACM*

   # Connect (replace /dev/ttyACM0 with your port)
   screen /dev/ttyACM0 115200
   # Or use minicom, putty, etc.
   ```

2. **Watch for Connection**
   You should see:
   ```
   ================================================
   SinricPro Switch Example
   ================================================

   [1/4] Initializing WiFi...
   [2/4] Connecting to WiFi SSID: YourNetwork
   WiFi connected! IP: 192.168.1.123
   [3/4] Initializing SinricPro...
   [4/4] Connecting to SinricPro...
   [SinricPro] State: WIFI_CONNECTED
   [SinricPro] State: WS_CONNECTING
   [SinricPro] State: CONNECTED

   ================================================
   Ready! Voice commands:
     'Alexa, turn on [device name]'
     'Alexa, turn off [device name]'
   ================================================
   ```

## Step 6: Link with Alexa

1. **Install Alexa App** on your phone
2. **Add SinricPro Skill**:
   - Open Alexa app
   - Menu → Skills & Games
   - Search "SinricPro"
   - Enable skill
   - Sign in with your SinricPro credentials
3. **Discover Devices**:
   - Say: "Alexa, discover devices"
   - Wait ~20 seconds
   - Your device should be found

## Step 7: Test Voice Control

Try these commands:
- "Alexa, turn on [your device name]"
- "Alexa, turn off [your device name]"

You should see in the serial output:
```
[Callback] Power state: ON
[Callback] Power state: OFF
```

## Common Issues

### WiFi Not Connecting

- Check SSID and password are correct
- Ensure WiFi is 2.4GHz (Pico W doesn't support 5GHz)
- Check WiFi security is WPA2 (WPA3 not supported)

### SinricPro Not Connecting

- Verify credentials are correct (App Key, Secret, Device ID)
- Check device status in SinricPro portal
- Ensure internet connection is working
- Check firewall allows outbound HTTPS (port 443)

### Alexa Can't Find Device

- Make sure device shows "Connected" in SinricPro portal
- Disable and re-enable SinricPro skill in Alexa
- Try manual discovery in Alexa app
- Check device name doesn't contain special characters

### Device Keeps Disconnecting

- Check WiFi signal strength
- Verify power supply is adequate (use powered USB hub if needed)
- Check for interference from other devices
- Try increasing heartbeat interval in code

## Next Steps

- **Try Other Examples**: Check `examples/` directory for more device types
- **Add More Devices**: Create additional devices in SinricPro portal
- **Create Routines**: Set up Alexa routines for automation
- **Customize**: Modify examples for your specific hardware

## Hardware Examples

### Basic LED Control (Switch)

```
Pico W Pin 15 → LED (+) → Resistor (220Ω) → GND
```

### PWM Dimming (DimSwitch)

```
Pico W Pin 15 → N-Channel MOSFET Gate
MOSFET Source → GND
MOSFET Drain → LED Strip (-)
12V Power Supply → LED Strip (+)
```

### RGB LED (Light)

```
Pico W Pin 13 → Red LED (via resistor)
Pico W Pin 14 → Green LED (via resistor)
Pico W Pin 15 → Blue LED (via resistor)
All LED cathodes → GND
```

### PIR Sensor (Motion Sensor)

```
PIR VCC → Pico W 3.3V (Pin 36)
PIR GND → Pico W GND
PIR OUT → Pico W GPIO 15
```

## Getting Help

- **Documentation**: [docs/](../docs/)
- **Examples**: [examples/](../examples/)
- **Issues**: [GitHub Issues](https://github.com/sinricpro/pico-sdk/issues)
- **SinricPro Help**: [help.sinric.pro](https://help.sinric.pro)
- **Discord**: [SinricPro Discord](https://discord.gg/sinricpro)

Happy hacking! 🎉
