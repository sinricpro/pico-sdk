# Troubleshooting Guide

Common issues and solutions for SinricPro Pico SDK.

## Table of Contents

- [Build Issues](#build-issues)
- [WiFi Issues](#wifi-issues)
- [Connection Issues](#connection-issues)
- [Voice Control Issues](#voice-control-issues)
- [Hardware Issues](#hardware-issues)
- [Performance Issues](#performance-issues)

---

## Build Issues

### Error: `PICO_SDK_PATH not set`

**Problem**: CMake can't find pico-sdk

**Solution**:
```bash
export PICO_SDK_PATH=/path/to/pico-sdk
# Or add to ~/.bashrc
echo 'export PICO_SDK_PATH=/path/to/pico-sdk' >> ~/.bashrc
```

### Error: `cJSON not found`

**Problem**: cJSON submodule not initialized

**Solution**:
```bash
git submodule add https://github.com/DaveGamble/cJSON.git lib/cJSON
git submodule update --init --recursive
```

### Error: `arm-none-eabi-gcc: command not found`

**Problem**: ARM GCC toolchain not installed

**Solution**:
```bash
# Ubuntu/Debian
sudo apt install gcc-arm-none-eabi libnewlib-arm-none-eabi

# macOS
brew install --cask gcc-arm-embedded

# Windows
# Download from: https://developer.arm.com/downloads/-/gnu-rm
```

### Warnings about `implicit function declaration`

**Problem**: Missing includes or wrong function signature

**Solution**: Check you're using the correct API. See examples for reference.

---

## WiFi Issues

### WiFi: Failed to connect (Timeout)

**Possible Causes**:

1. **Wrong credentials**
   - Double-check SSID and password
   - Check for extra spaces or quotes

2. **5GHz WiFi**
   - Pico W only supports 2.4GHz
   - Change router to 2.4GHz or create separate SSID

3. **WPA3 Security**
   - Pico W supports WPA2 only
   - Change router to WPA2 or WPA/WPA2 mixed mode

4. **Hidden SSID**
   - Currently not supported
   - Make SSID visible

5. **Special Characters in Password**
   - Ensure password is properly escaped in C string
   - Use `\"` for quotes, `\\` for backslash

**Debug Steps**:
```c
// Enable WiFi debug (before includes)
#define ENABLE_DEBUG

// Check signal strength
int8_t rssi = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);
printf("WiFi RSSI: %d dBm\n", rssi);
```

### WiFi Connects but Disconnects Frequently

**Possible Causes**:

1. **Weak Signal**
   - Move closer to router
   - Reduce interference (microwave, other 2.4GHz devices)
   - Use external antenna if available

2. **Power Issues**
   - Use powered USB hub
   - Check USB cable quality
   - Add capacitor (10µF) across Pico W power pins

3. **Router Issues**
   - Update router firmware
   - Reduce WiFi clients on network
   - Change WiFi channel

---

## Connection Issues

### SinricPro: Connection Refused / Timeout

**Possible Causes**:

1. **Firewall Blocking**
   - Allow outbound HTTPS (port 443)
   - Allow WebSocket connections

2. **Wrong Credentials**
   - Verify App Key and App Secret
   - Check Device ID is exactly 24 characters
   - Ensure no extra spaces

3. **DNS Issues**
   - Try pinging `ws.sinric.pro`
   - Check DNS server settings

4. **Clock Not Synced**
   - Wait a few seconds for NTP sync after WiFi connects

**Debug Steps**:
```c
// Enable debug output
#define ENABLE_DEBUG

// Check credentials format
printf("App Key length: %d (should be 36)\n", strlen(APP_KEY));
printf("Device ID length: %d (should be 24)\n", strlen(DEVICE_ID));
```

### TLS Handshake Failed

**Possible Causes**:

1. **mbedTLS Issues**
   - Ensure pico-sdk includes mbedTLS
   - Check mbedTLS is enabled in CMake

2. **Memory Issues**
   - TLS requires ~32KB RAM
   - Reduce buffer sizes if needed
   - Use non-secure mode (not recommended):
     ```c
     #define SINRICPRO_NOSSL
     ```

3. **Time Sync Issues**
   - TLS requires correct system time
   - Wait for NTP sync

### WebSocket Disconnects After Connection

**Possible Causes**:

1. **Heartbeat Timeout**
   - Pings must be sent every 300 seconds
   - Check `sinricpro_handle()` is called regularly

2. **Invalid Messages**
   - Check signature is correct
   - Verify JSON format

3. **Device Not Registered**
   - Check device exists in SinricPro portal
   - Ensure device ID matches exactly

---

## Voice Control Issues

### Alexa: "Device is not responding"

**Possible Causes**:

1. **Device Offline**
   - Check device shows "Connected" in SinricPro portal
   - Verify serial output shows CONNECTED state

2. **Callback Returns False**
   - Ensure callback returns `true` on success
   ```c
   bool on_power_state(sinricpro_device_t *device, bool *state) {
       // Do your thing
       gpio_put(LED_PIN, *state);
       return true;  // ← Important!
   }
   ```

3. **Slow Response**
   - `sinricpro_handle()` not called frequently enough
   - Add timing check:
   ```c
   // Should be called at least every 100ms
   while (1) {
       sinricpro_handle();
       sleep_ms(10);  // Good
       // sleep_ms(1000);  // Bad - too slow
   }
   ```

### Alexa Can't Find Device

**Possible Causes**:

1. **Device Not Added to SinricPro**
   - Verify device exists in portal
   - Check device type is correct

2. **SinricPro Skill Not Linked**
   - Enable SinricPro skill in Alexa app
   - Sign in with correct account

3. **Account Mismatch**
   - Use same account for SinricPro and Alexa

4. **Discovery Issues**
   - Try manual discovery in Alexa app
   - Disable and re-enable skill
   - Wait 24 hours and try again

**Solution Steps**:
1. Delete device from SinricPro portal
2. Disable SinricPro skill in Alexa
3. Create new device in portal
4. Enable skill and sign in
5. Discover devices

### Wrong Device Responds

**Problem**: Commands go to wrong device

**Solution**: Check device names are unique and clear
- Bad: "Light 1", "Light 2" (Alexa confuses them)
- Good: "Bedroom Light", "Kitchen Light"

---

## Hardware Issues

### LED Not Turning On/Off

**Check**:
1. **GPIO Pin Number**
   - Verify pin matches code
   - Pico W pin != GPIO number

2. **Voltage Levels**
   - GPIO outputs 3.3V
   - Use level shifter for 5V devices
   - Check LED polarity

3. **Current Limits**
   - GPIO max: 12mA
   - Use transistor/MOSFET for loads >10mA

### PWM Not Working

**Check**:
1. **PWM Configuration**
   ```c
   gpio_set_function(PIN, GPIO_FUNC_PWM);
   uint slice = pwm_gpio_to_slice_num(PIN);
   pwm_set_enabled(slice, true);
   ```

2. **Frequency**
   - Default is ~125MHz / 65536 ≈ 1.9kHz
   - Adjust with `pwm_set_wrap()` for different frequency

3. **Pin Capabilities**
   - All GPIOs support PWM
   - But some pins share PWM channels

### Sensor Not Detecting

**Check**:
1. **Pull Resistors**
   ```c
   gpio_pull_up(PIN);   // For active-low sensors
   gpio_pull_down(PIN); // For active-high sensors
   ```

2. **Voltage Levels**
   - PIR sensors typically output 3.3V (compatible)
   - Some sensors need 5V (use level shifter)

3. **Debouncing**
   - Add delay to avoid noise
   - Check example code for debounce logic

---

## Performance Issues

### Slow Response to Commands

**Causes**:
1. **`sinricpro_handle()` not called often enough**
   - Call every 10-50ms
   - Don't block in callbacks

2. **Long-running Callbacks**
   - Keep callbacks fast (<10ms)
   - Use flags to defer work

3. **WiFi Issues**
   - Check signal strength
   - Reduce network traffic

### High Memory Usage

**Solutions**:
1. **Use Non-secure Mode** (saves ~32KB)
   ```c
   #define SINRICPRO_NOSSL  // Before includes
   ```

2. **Reduce Buffer Sizes**
   - Edit `sinricpro_config.h`
   - Lower queue sizes

3. **Optimize Code**
   - Use `const` for strings
   - Reduce local variables
   - Use static allocation

### Frequent Disconnections

**Causes**:
1. **Event Rate Limiting**
   - State events: 1 per second max
   - Sensor events: 1 per 60 seconds max
   - Check event limiter isn't blocking

2. **Heartbeat Missed**
   - Ensure `sinricpro_handle()` called regularly
   - Don't have long delays in main loop

3. **Memory Corruption**
   - Check for buffer overflows
   - Validate all pointer accesses
   - Use `-fsanitize=address` in debug builds

---

## Debug Tips

### Enable Detailed Logging

```c
// In your main.c, before includes
#define ENABLE_DEBUG

// In sinricpro config
sinricpro_config_t config = {
    .app_key = APP_KEY,
    .app_secret = APP_SECRET,
    .enable_debug = true  // ← Enable debug output
};
```

### Monitor Serial Output

```bash
# Linux
screen /dev/ttyACM0 115200

# macOS
screen /dev/cu.usbmodem* 115200

# Windows
# Use PuTTY or Tera Term
```

### Check Memory Usage

```c
#include "pico/stdlib.h"

// Get free heap
extern char __StackLimit, __bss_end__;
uint32_t free_ram = &__StackLimit  - &__bss_end__;
printf("Free RAM: %lu bytes\n", free_ram);
```

### Verify Credentials

```bash
# Check format
echo -n "YOUR_APP_KEY" | wc -c     # Should be 36
echo -n "YOUR_DEVICE_ID" | wc -c   # Should be 24
```

---

## Still Need Help?

1. **Check Examples**: Look at working example code
2. **Search Issues**: [GitHub Issues](https://github.com/sinricpro/pico-sdk/issues)
3. **Ask Community**: [SinricPro Discord](https://discord.gg/sinricpro)
4. **Contact Support**: [SinricPro Support](https://sinric.pro/support)

When asking for help, include:
- Hardware (Pico W or Pico 2 W)
- SDK version
- Full serial output
- Code snippet (minimal example)
- What you've tried
