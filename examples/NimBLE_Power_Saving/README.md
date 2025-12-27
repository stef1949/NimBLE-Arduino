# NimBLE Power Saving Example

This example demonstrates advanced power saving and efficiency features for ESP32-S3 and compatible ESP32 devices.

## Features

- **Automatic Light Sleep**: Enables the ESP32 to enter light sleep mode during BLE idle periods
- **Power Mode Presets**: Three predefined power modes for different use cases
- **Optimized Connection Parameters**: Helper functions for power-efficient BLE connections
- **TX Power Management**: Dynamic control of transmission power

## Power Modes

### Low Power Mode (Mode 0)
- TX Power: -12 dBm (minimum)
- Connection Interval: 250ms
- Slave Latency: 4 events
- **Best for**: Battery-powered sensors, periodic data reporting
- **Estimated current**: ~1-2 mA during idle (with light sleep)

### Balanced Mode (Mode 1)
- TX Power: 0 dBm
- Connection Interval: 100ms
- Slave Latency: 2 events
- **Best for**: General purpose applications, moderate responsiveness
- **Estimated current**: ~2-4 mA during idle

### High Performance Mode (Mode 2)
- TX Power: 9 dBm (maximum)
- Connection Interval: 30ms
- Slave Latency: 0 events
- **Best for**: Real-time control, gaming, audio streaming
- **Estimated current**: ~5-8 mA during idle

## Requirements

### For Automatic Light Sleep (Optional but Recommended)

To enable automatic light sleep, you need to configure your build system:

#### PlatformIO
Add to your `platformio.ini`:
```ini
build_flags = 
    -DCONFIG_PM_ENABLE
```

#### Arduino IDE
The feature will work automatically if your ESP-IDF configuration has power management enabled. For most ESP32-S3 boards, this is enabled by default.

### Supported Devices
- ESP32 (all variants)
- ESP32-C3
- ESP32-S3 (recommended for best power efficiency)
- ESP32-C6
- ESP32-H2
- ESP32-C2

Note: ESP32-P4 does not support these power management features.

## How to Use

1. Upload the sketch to your ESP32-S3 or compatible board
2. Open Serial Monitor (115200 baud)
3. The device will start advertising as "NimBLE-PowerSave"
4. Connect with a BLE client (phone app, etc.)
5. Monitor power consumption using the serial output

## Configuration

Modify these defines in the sketch to change behavior:

```cpp
#define POWER_MODE 0          // 0=Low, 1=Balanced, 2=High Performance
#define ENABLE_AUTO_SLEEP true // Enable automatic light sleep
```

## Connection Parameters Explained

Connection parameters significantly affect power consumption:

- **Connection Interval**: How often the devices communicate. Longer intervals = lower power but higher latency
- **Slave Latency**: Number of events the slave can skip. Higher values reduce power consumption
- **Supervision Timeout**: Maximum time before connection is considered lost

The `getOptimalConnParams()` function provides recommended values for power-efficient operation.

## API Usage

```cpp
// Set power mode (0=Low, 1=Balanced, 2=High Performance)
NimBLEDevice::setPowerMode(0);

// Enable automatic light sleep
NimBLEDevice::enablePowerSave(true);

// Check if power save is enabled
bool enabled = NimBLEDevice::isPowerSaveEnabled();

// Get optimal connection parameters for low power
uint16_t minInterval, maxInterval, latency, timeout;
NimBLEDevice::getOptimalConnParams(minInterval, maxInterval, latency, timeout, true);

// Apply the parameters
pServer->updateConnParams(connHandle, minInterval, maxInterval, latency, timeout);

// Set custom low power parameters
NimBLEDevice::setLowPowerParams(200, 4); // interval=250ms, latency=4

// Get current TX power
int power = NimBLEDevice::getPower();

// Set specific TX power
NimBLEDevice::setPower(-12, NimBLETxPowerType::All);
```

## Power Consumption Tips

1. **Use Low Power Mode** for battery-powered applications
2. **Increase advertising intervals** (already done in this example)
3. **Enable automatic light sleep** when possible
4. **Reduce notification/indication frequency** in your application
5. **Use connection parameter negotiation** to optimize based on use case
6. **Consider deep sleep** for very infrequent updates (requires wake-up mechanism)

## Troubleshooting

### "Power management not available" message
- Add `CONFIG_PM_ENABLE` to your build flags
- Or: The feature is optional; the sketch will still work without it

### High current consumption
- Check if light sleep is actually being entered (monitor logs)
- Verify connection parameters are being applied
- Ensure no other tasks are keeping CPU busy

### Connection issues in Low Power Mode
- Low TX power may reduce range
- Try Balanced mode for better reliability
- Adjust connection parameters if needed

## References

- [ESP32 Power Management](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/power_management.html)
- [Bluetooth Low Energy Power Optimization](https://www.bluetooth.com/blog/designing-bluetooth-low-energy-products-with-low-power-consumption/)
- [NimBLE Documentation](https://h2zero.github.io/NimBLE-Arduino/)
