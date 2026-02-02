/**
 * Mate Tracker ESP32-C3 Firmware
 * 
 * Hosts the Mate Service web application on ESP32-C3
 * with WiFi connectivity, mDNS, and persistent storage.
 * 
 * Features:
 * - WiFi connection with configurable credentials
 * - Async HTTP server for better performance
 * - LittleFS for static file hosting
 * - NVS/Preferences for persistent data storage
 * - mDNS for easy discovery (http://mate-tracker.local)
 * - RESTful API endpoints for consumption tracking
 */

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <Preferences.h>

#include "config.h"
#include "wifi_manager.h"
#include "data_storage.h"
#include "web_handlers.h"

// Global objects
AsyncWebServer server(80);
DataStorage dataStorage;
WiFiManager wifiManager;

// RGB LED (WS2812/NeoPixel on ESP32-C3-DevKitM-1)
#define RGB_LED_PIN 8  // GPIO8 - addressable RGB LED

// System status flag
bool systemReady = false;

// Set RGB LED color using ESP32's built-in neopixelWrite()
void setLEDColor(uint8_t r, uint8_t g, uint8_t b) {
    neopixelWrite(RGB_LED_PIN, r, g, b);
}

void setLEDRed()    { setLEDColor(25, 0, 0); }
void setLEDGreen()  { setLEDColor(0, 25, 0); }
void setLEDBlue()   { setLEDColor(0, 0, 25); }
void setLEDYellow() { setLEDColor(25, 25, 0); }
void setLEDOff()    { setLEDColor(0, 0, 0); }

void blinkLED(uint8_t r, uint8_t g, uint8_t b, int times, int delayMs = 200) {
    for (int i = 0; i < times; i++) {
        setLEDColor(r, g, b);
        delay(delayMs);
        setLEDOff();
        delay(delayMs);
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n========================================");
    Serial.println("   Mate Tracker ESP32-C3 Starting...   ");
    Serial.println("========================================\n");
    
    // Start with RED LED to indicate startup/not ready
    setLEDRed();
    Serial.println("[LED] Status: RED (starting up...)");
    
    blinkLED(25, 0, 0, 2);  // Blink RED to indicate startup
    
    // Initialize LittleFS for web files
    Serial.println("[FS] Initializing LittleFS...");
    if (!LittleFS.begin(true)) {
        Serial.println("[FS] ERROR: LittleFS mount failed!");
        setLEDRed();  // Keep RED for error
        blinkLED(25, 0, 0, 10, 100);  // Rapid RED blink indicates error
        return;
    }
    Serial.println("[FS] LittleFS mounted successfully");
    
    // List files for debugging
    Serial.println("[FS] Files in LittleFS:");
    File root = LittleFS.open("/");
    File file = root.openNextFile();
    while (file) {
        Serial.printf("  - %s (%d bytes)\n", file.name(), file.size());
        file = root.openNextFile();
    }
    
    // Initialize data storage
    Serial.println("\n[DATA] Initializing data storage...");
    if (!dataStorage.begin()) {
        Serial.println("[DATA] ERROR: Data storage initialization failed!");
    }
    Serial.println("[DATA] Data storage ready");
    
    // Connect to WiFi
    Serial.println("\n[WIFI] Connecting to WiFi...");
    Serial.printf("[WIFI] SSID: %s\n", WIFI_SSID);
    
    if (!wifiManager.connect(WIFI_SSID, WIFI_PASSWORD)) {
        Serial.println("[WIFI] ERROR: Failed to connect to WiFi!");
        Serial.println("[WIFI] Please check your credentials in config.h");
        setLEDRed();  // Keep RED for WiFi error
        blinkLED(25, 0, 0, 5, 500);  // Slow RED blink indicates WiFi error
        // Continue anyway - maybe WiFi will reconnect later
    }
    
    // Setup mDNS
    Serial.println("\n[mDNS] Setting up mDNS...");
    if (MDNS.begin(MDNS_HOSTNAME)) {
        Serial.printf("[mDNS] Hostname: http://%s.local\n", MDNS_HOSTNAME);
        MDNS.addService("http", "tcp", 80);
    } else {
        Serial.println("[mDNS] ERROR: mDNS setup failed!");
    }
    
    // Setup web server routes
    Serial.println("\n[WEB] Setting up web server...");
    setupWebHandlers(server, dataStorage);
    
    // Start server
    server.begin();
    Serial.println("[WEB] Server started on port 80");
    
    // Print access information
    Serial.println("\n========================================");
    Serial.println("   Mate Tracker Ready!                 ");
    Serial.println("========================================");
    Serial.println("\nAccess the web interface at:");
    Serial.printf("  http://%s\n", WiFi.localIP().toString().c_str());
    Serial.printf("  http://%s.local\n", MDNS_HOSTNAME);
    Serial.println("\nAPI Endpoints:");
    Serial.println("  GET  /api/state       - Get full state");
    Serial.println("  GET  /api/status      - Get system status");
    Serial.println("  POST /api/users       - Add user");
    Serial.println("  POST /api/items       - Add item");
    Serial.println("  POST /api/consumption - Record consumption");
    Serial.println("  POST /api/payments    - Process payment");
    Serial.println("  POST /api/reset       - Reset all data");
    Serial.println("========================================\n");
    
    // SUCCESS! Set LED to GREEN
    if (WiFi.status() == WL_CONNECTED) {
        setLEDGreen();
        systemReady = true;
        Serial.println("[LED] Status: GREEN (system ready!)");
        blinkLED(0, 25, 0, 3, 100);  // Quick GREEN triple blink = ready
        setLEDGreen();  // Keep green
    } else {
        setLEDRed();  // Stay RED if WiFi not connected
        Serial.println("[LED] Status: RED (WiFi not connected)");
    }
}

void loop() {
    // Handle WiFi reconnection
    static unsigned long lastWifiCheck = 0;
    if (millis() - lastWifiCheck > 30000) {  // Check every 30 seconds
        lastWifiCheck = millis();
        
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("[WIFI] Connection lost, attempting reconnect...");
            setLEDRed();  // Show RED when WiFi disconnected
            systemReady = false;
            wifiManager.reconnect();
        }
    }
    
    // Update LED based on WiFi status
    static unsigned long lastLEDUpdate = 0;
    if (millis() - lastLEDUpdate > 2000) {  // Check every 2 seconds
        lastLEDUpdate = millis();
        
        if (WiFi.status() == WL_CONNECTED) {
            if (!systemReady) {
                setLEDGreen();  // Turn GREEN when reconnected
                systemReady = true;
                Serial.println("[LED] Status: GREEN (reconnected!)");
            }
        } else {
            if (systemReady) {
                setLEDRed();  // Turn RED when disconnected
                systemReady = false;
                Serial.println("[LED] Status: RED (disconnected)");
            }
        }
    }
    
    // Heartbeat: brief dim on GREEN LED every 5 seconds when connected
    static unsigned long lastHeartbeat = 0;
    if (millis() - lastHeartbeat > 5000) {
        lastHeartbeat = millis();
        if (systemReady && WiFi.status() == WL_CONNECTED) {
            setLEDColor(0, 5, 0);  // Dim green briefly
            delay(50);
            setLEDGreen();  // Back to green
        }
    }
    
    // Small delay to prevent watchdog issues
    delay(10);
}
