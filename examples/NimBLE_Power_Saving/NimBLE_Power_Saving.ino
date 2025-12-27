
/**
 *  NimBLE_Power_Saving Demo:
 *
 *  Demonstrates advanced power saving and efficiency features for ESP32-S3
 *  and compatible ESP32 devices.
 *
 *  Features demonstrated:
 *  - Automatic light sleep during BLE idle periods
 *  - Power mode presets (Low Power, Balanced, High Performance)
 *  - Optimized connection parameters for power efficiency
 *  - TX power management
 *
 *  Created: December 2024
 *      Author: NimBLE-Arduino Contributors
 */

#include <Arduino.h>
#include <NimBLEDevice.h>

static NimBLEServer* pServer;

// Define which power mode to use:
// 0 = Low Power (maximum battery life)
// 1 = Balanced (good balance between power and performance)
// 2 = High Performance (maximum responsiveness)
#define POWER_MODE 0

// Enable automatic light sleep
#define ENABLE_AUTO_SLEEP true

/** Server callbacks to handle connection events */
class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
        Serial.printf("Client connected - Address: %s\n", connInfo.getAddress().toString().c_str());

        // Get optimal connection parameters for power saving
        uint16_t minInterval, maxInterval, latency, timeout;
        NimBLEDevice::getOptimalConnParams(minInterval, maxInterval, latency, timeout, true);
        
        // Update connection parameters for power efficiency
        pServer->updateConnParams(connInfo.getConnHandle(), minInterval, maxInterval, latency, timeout);
        
        Serial.printf("Connection params set - Interval: %d-%d, Latency: %d, Timeout: %d\n",
                     minInterval, maxInterval, latency, timeout);
    }

    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
        Serial.printf("Client disconnected - Reason: %d\n", reason);
        Serial.println("Starting advertising...");
        NimBLEDevice::startAdvertising();
    }

    void onMTUChange(uint16_t MTU, NimBLEConnInfo& connInfo) override {
        Serial.printf("MTU updated: %u for connection ID: %u\n", MTU, connInfo.getConnHandle());
    }
};

/** Characteristic callbacks to handle read/write events */
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        Serial.printf("Characteristic read - UUID: %s\n", 
                     pCharacteristic->getUUID().toString().c_str());
    }

    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        Serial.printf("Characteristic written - UUID: %s, Value: %s\n",
                     pCharacteristic->getUUID().toString().c_str(),
                     pCharacteristic->getValue().c_str());
    }

    void onStatus(NimBLECharacteristic* pCharacteristic, int code) override {
        Serial.printf("Notification/Indication status: %d\n", code);
    }
};

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n==================================");
    Serial.println("NimBLE Power Saving Demo");
    Serial.println("==================================\n");

    // Initialize NimBLE
    Serial.println("Initializing BLE...");
    NimBLEDevice::init("NimBLE-PowerSave");

    // Set power mode before starting services
    Serial.printf("Setting power mode: %d ", POWER_MODE);
    switch(POWER_MODE) {
        case 0: Serial.println("(Low Power)"); break;
        case 1: Serial.println("(Balanced)"); break;
        case 2: Serial.println("(High Performance)"); break;
    }
    
    if (NimBLEDevice::setPowerMode(POWER_MODE)) {
        Serial.println("Power mode set successfully");
    } else {
        Serial.println("Failed to set power mode");
    }

    // Enable automatic light sleep if supported and requested
#ifdef CONFIG_PM_ENABLE
    if (ENABLE_AUTO_SLEEP) {
        Serial.println("Enabling automatic light sleep...");
        if (NimBLEDevice::enablePowerSave(true)) {
            Serial.println("Power save enabled - device will enter light sleep during idle periods");
        } else {
            Serial.println("Failed to enable power save");
        }
    }
#else
    Serial.println("Note: CONFIG_PM_ENABLE not set - automatic light sleep unavailable");
    Serial.println("To enable: Add 'build_flags = -DCONFIG_PM_ENABLE' to platformio.ini");
#endif

    // Display current power configuration
    Serial.printf("\nCurrent TX Power: %d dBm\n", NimBLEDevice::getPower());
    Serial.printf("Power Save Enabled: %s\n", NimBLEDevice::isPowerSaveEnabled() ? "Yes" : "No");

    // Create BLE Server
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    // Create BLE Service
    NimBLEService* pService = pServer->createService("ABCD");
    
    // Create a characteristic for reading/writing
    NimBLECharacteristic* pCharacteristic = pService->createCharacteristic(
        "1234",
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY
    );
    
    pCharacteristic->setValue("Power Save Demo");
    pCharacteristic->setCallbacks(new CharacteristicCallbacks());

    // Start the service
    pService->start();

    // Configure advertising for power efficiency
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID("ABCD");
    pAdvertising->setScanResponse(true);
    
    // Use longer advertising intervals to save power
    // Min: 1000ms (1600 * 0.625ms), Max: 1280ms (2048 * 0.625ms)
    pAdvertising->setMinInterval(1600);
    pAdvertising->setMaxInterval(2048);
    
    Serial.println("\nStarting BLE advertising...");
    pAdvertising->start();

    Serial.println("\n==================================");
    Serial.println("Setup complete!");
    Serial.println("Device is now advertising with");
    Serial.println("power-optimized settings.");
    Serial.println("==================================\n");
}

void loop() {
    // Main loop - the device will automatically manage power
    // during idle periods when power save is enabled
    
    static unsigned long lastReport = 0;
    if (millis() - lastReport > 10000) {
        lastReport = millis();
        
        // Periodic status report
        Serial.println("Status: Running with power optimization");
        Serial.printf("  TX Power: %d dBm\n", NimBLEDevice::getPower());
        Serial.printf("  Power Save: %s\n", NimBLEDevice::isPowerSaveEnabled() ? "Enabled" : "Disabled");
        
        if (pServer->getConnectedCount() > 0) {
            Serial.printf("  Connected devices: %d\n", pServer->getConnectedCount());
        } else {
            Serial.println("  Status: Advertising (no connections)");
        }
    }
    
    delay(1000);
}
