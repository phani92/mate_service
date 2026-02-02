/**
 * Configuration file for Mate Tracker ESP32-C3
 * 
 * IMPORTANT: Update WiFi credentials before uploading!
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// WiFi Configuration
// ============================================
// Update these with your WiFi network credentials
#define WIFI_SSID     "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// WiFi connection timeout (milliseconds)
#define WIFI_TIMEOUT_MS 30000

// ============================================
// mDNS Configuration
// ============================================
// Access your device at http://mate-tracker.local
#define MDNS_HOSTNAME "mate-tracker"

// ============================================
// Server Configuration
// ============================================
#define HTTP_PORT 80

// ============================================
// Data Storage Configuration  
// ============================================
// Namespace for NVS storage
#define NVS_NAMESPACE "mate_data"

// Maximum number of records
#define MAX_USERS 20
#define MAX_ITEMS 50
#define MAX_CONSUMPTION_RECORDS 500
#define MAX_PAYMENT_RECORDS 200

// ============================================
// Hardware Configuration
// ============================================
// Status LED GPIO (8 for most ESP32-C3 DevKits)
#define LED_PIN 8

// ============================================
// Debug Configuration
// ============================================
#define DEBUG_SERIAL 1

#if DEBUG_SERIAL
    #define DEBUG_PRINT(x) Serial.print(x)
    #define DEBUG_PRINTLN(x) Serial.println(x)
    #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINTF(...)
#endif

#endif // CONFIG_H
