/**
 * WiFi Manager for ESP32-C3
 * Handles WiFi connection and reconnection
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include "config.h"

class WiFiManager {
public:
    WiFiManager() : _connected(false) {}
    
    /**
     * Connect to WiFi network
     * @param ssid Network SSID
     * @param password Network password
     * @return true if connected successfully
     */
    bool connect(const char* ssid, const char* password) {
        _ssid = String(ssid);
        _password = String(password);
        
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
        
        DEBUG_PRINT("[WIFI] Connecting");
        
        unsigned long startTime = millis();
        while (WiFi.status() != WL_CONNECTED) {
            if (millis() - startTime > WIFI_TIMEOUT_MS) {
                DEBUG_PRINTLN("\n[WIFI] Connection timeout!");
                return false;
            }
            delay(500);
            DEBUG_PRINT(".");
        }
        
        DEBUG_PRINTLN();
        DEBUG_PRINTLN("[WIFI] Connected!");
        DEBUG_PRINTF("[WIFI] IP Address: %s\n", WiFi.localIP().toString().c_str());
        DEBUG_PRINTF("[WIFI] Signal Strength: %d dBm\n", WiFi.RSSI());
        DEBUG_PRINTF("[WIFI] MAC Address: %s\n", WiFi.macAddress().c_str());
        
        _connected = true;
        return true;
    }
    
    /**
     * Attempt to reconnect to WiFi
     * @return true if reconnected successfully
     */
    bool reconnect() {
        if (WiFi.status() == WL_CONNECTED) {
            return true;
        }
        
        DEBUG_PRINTLN("[WIFI] Attempting reconnection...");
        WiFi.disconnect();
        delay(1000);
        
        return connect(_ssid.c_str(), _password.c_str());
    }
    
    /**
     * Check if WiFi is connected
     * @return true if connected
     */
    bool isConnected() {
        return WiFi.status() == WL_CONNECTED;
    }
    
    /**
     * Get current IP address
     * @return IP address as string
     */
    String getIP() {
        return WiFi.localIP().toString();
    }
    
    /**
     * Get signal strength in dBm
     * @return RSSI value
     */
    int getRSSI() {
        return WiFi.RSSI();
    }
    
    /**
     * Get signal quality as percentage (0-100)
     * @return Signal quality percentage
     */
    int getSignalQuality() {
        int rssi = WiFi.RSSI();
        if (rssi <= -100) return 0;
        if (rssi >= -50) return 100;
        return 2 * (rssi + 100);
    }

private:
    String _ssid;
    String _password;
    bool _connected;
};

#endif // WIFI_MANAGER_H
