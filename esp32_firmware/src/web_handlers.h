/**
 * Web Handlers for Mate Tracker ESP32-C3
 * 
 * Sets up all HTTP routes and API endpoints
 */

#ifndef WEB_HANDLERS_H
#define WEB_HANDLERS_H

#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "data_storage.h"
#include "config.h"

// Forward declaration
void setupWebHandlers(AsyncWebServer& server, DataStorage& storage);

// Helper to set CORS headers
void setCORSHeaders(AsyncWebServerResponse* response) {
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");
}

// Send JSON response
void sendJsonResponse(AsyncWebServerRequest* request, DynamicJsonDocument& doc, int code = 200) {
    String json;
    serializeJson(doc, json);
    AsyncWebServerResponse* response = request->beginResponse(code, "application/json", json);
    setCORSHeaders(response);
    request->send(response);
}

// Send error response
void sendError(AsyncWebServerRequest* request, const char* message, int code = 400) {
    DynamicJsonDocument doc(256);
    doc["error"] = message;
    sendJsonResponse(request, doc, code);
}

// Get current timestamp as string
String getTimestamp() {
    // Simple timestamp based on millis since we don't have RTC
    // Format: relative time in seconds since boot
    return String(millis() / 1000);
}

/**
 * Setup all web server routes
 */
void setupWebHandlers(AsyncWebServer& server, DataStorage& storage) {
    
    // ========================================
    // Static File Serving
    // ========================================
    
    // Serve static files from LittleFS
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    
    // ========================================
    // CORS Preflight Handler
    // ========================================
    server.on("/*", HTTP_OPTIONS, [](AsyncWebServerRequest* request) {
        AsyncWebServerResponse* response = request->beginResponse(204);
        setCORSHeaders(response);
        request->send(response);
    });
    
    // ========================================
    // System Status API
    // ========================================
    
    // GET /api/status - System status
    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest* request) {
        DynamicJsonDocument doc(1024);
        
        doc["device"] = "ESP32-C3";
        doc["firmware"] = "1.0.0";
        doc["uptime"] = millis() / 1000;
        doc["freeHeap"] = ESP.getFreeHeap();
        doc["totalHeap"] = ESP.getHeapSize();
        doc["wifi"]["connected"] = WiFi.status() == WL_CONNECTED;
        doc["wifi"]["ssid"] = WiFi.SSID();
        doc["wifi"]["ip"] = WiFi.localIP().toString();
        doc["wifi"]["rssi"] = WiFi.RSSI();
        doc["wifi"]["signalQuality"] = WiFi.RSSI() <= -100 ? 0 : (WiFi.RSSI() >= -50 ? 100 : 2 * (WiFi.RSSI() + 100));
        
        sendJsonResponse(request, doc);
    });
    
    // ========================================
    // State API (Full State)
    // ========================================
    
    // GET /api/state - Get full application state
    server.on("/api/state", HTTP_GET, [&storage](AsyncWebServerRequest* request) {
        String stateJson = storage.getStateJson();
        AsyncWebServerResponse* response = request->beginResponse(200, "application/json", stateJson);
        setCORSHeaders(response);
        request->send(response);
    });
    
    // ========================================
    // Users API
    // ========================================
    
    // POST /api/users - Add new user
    server.on("/api/users", HTTP_POST, [](AsyncWebServerRequest* request) {},
        NULL,
        [&storage](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
            DynamicJsonDocument doc(512);
            DeserializationError error = deserializeJson(doc, data, len);
            
            if (error) {
                sendError(request, "Invalid JSON");
                return;
            }
            
            const char* name = doc["name"];
            if (!name || strlen(name) == 0) {
                sendError(request, "Name is required");
                return;
            }
            
            if (storage.userExists(name)) {
                sendError(request, "User already exists");
                return;
            }
            
            String id = String(millis());
            if (!storage.addUser(id.c_str(), name)) {
                sendError(request, "Failed to add user", 500);
                return;
            }
            
            String stateJson = storage.getStateJson();
            AsyncWebServerResponse* response = request->beginResponse(200, "application/json", stateJson);
            setCORSHeaders(response);
            request->send(response);
        }
    );
    
    // DELETE /api/users/{id} - Remove user
    server.on("^\\/api\\/users\\/([a-zA-Z0-9]+)$", HTTP_DELETE, [&storage](AsyncWebServerRequest* request) {
        String userId = request->pathArg(0);
        
        if (!storage.removeUser(userId.c_str())) {
            sendError(request, "User not found", 404);
            return;
        }
        
        String stateJson = storage.getStateJson();
        AsyncWebServerResponse* response = request->beginResponse(200, "application/json", stateJson);
        setCORSHeaders(response);
        request->send(response);
    });
    
    // ========================================
    // Items API
    // ========================================
    
    // POST /api/items - Add new item
    server.on("/api/items", HTTP_POST, [](AsyncWebServerRequest* request) {},
        NULL,
        [&storage](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
            DynamicJsonDocument doc(512);
            DeserializationError error = deserializeJson(doc, data, len);
            
            if (error) {
                sendError(request, "Invalid JSON");
                return;
            }
            
            const char* name = doc["name"];
            float price = doc["price"] | 0.0f;
            int stock = doc["stock"] | 24;
            
            if (!name || strlen(name) == 0) {
                sendError(request, "Name is required");
                return;
            }
            
            if (price <= 0) {
                sendError(request, "Invalid price");
                return;
            }
            
            if (storage.itemExists(name)) {
                sendError(request, "Item already exists");
                return;
            }
            
            String id = String(millis());
            if (!storage.addItem(id.c_str(), name, price, stock)) {
                sendError(request, "Failed to add item", 500);
                return;
            }
            
            String stateJson = storage.getStateJson();
            AsyncWebServerResponse* response = request->beginResponse(200, "application/json", stateJson);
            setCORSHeaders(response);
            request->send(response);
        }
    );
    
    // DELETE /api/items/{id} - Remove item
    server.on("^\\/api\\/items\\/([a-zA-Z0-9]+)$", HTTP_DELETE, [&storage](AsyncWebServerRequest* request) {
        String itemId = request->pathArg(0);
        
        if (!storage.removeItem(itemId.c_str())) {
            sendError(request, "Item not found", 404);
            return;
        }
        
        String stateJson = storage.getStateJson();
        AsyncWebServerResponse* response = request->beginResponse(200, "application/json", stateJson);
        setCORSHeaders(response);
        request->send(response);
    });
    
    // PUT /api/items/{id}/stock - Update item stock
    server.on("^\\/api\\/items\\/([a-zA-Z0-9]+)\\/stock$", HTTP_PUT, [](AsyncWebServerRequest* request) {},
        NULL,
        [&storage](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
            String itemId = request->pathArg(0);
            
            DynamicJsonDocument doc(256);
            DeserializationError error = deserializeJson(doc, data, len);
            
            if (error) {
                sendError(request, "Invalid JSON");
                return;
            }
            
            int stock = doc["stock"] | -1;
            if (stock < 0) {
                sendError(request, "Invalid stock value");
                return;
            }
            
            if (!storage.updateItemStock(itemId.c_str(), stock)) {
                sendError(request, "Item not found", 404);
                return;
            }
            
            String stateJson = storage.getStateJson();
            AsyncWebServerResponse* response = request->beginResponse(200, "application/json", stateJson);
            setCORSHeaders(response);
            request->send(response);
        }
    );
    
    // ========================================
    // Consumption API
    // ========================================
    
    // POST /api/consumption - Record consumption
    server.on("/api/consumption", HTTP_POST, [](AsyncWebServerRequest* request) {},
        NULL,
        [&storage](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
            DynamicJsonDocument doc(512);
            DeserializationError error = deserializeJson(doc, data, len);
            
            if (error) {
                sendError(request, "Invalid JSON");
                return;
            }
            
            const char* userId = doc["userId"];
            const char* itemId = doc["itemId"];
            int quantity = doc["quantity"] | 0;
            
            if (!userId || !itemId || quantity <= 0) {
                sendError(request, "Invalid input");
                return;
            }
            
            // Check available stock
            int available = storage.getAvailableStock(itemId);
            if (quantity > available) {
                sendError(request, "Not enough stock");
                return;
            }
            
            String id = String(millis());
            if (!storage.addConsumption(id.c_str(), userId, itemId, quantity)) {
                sendError(request, "Failed to record consumption", 500);
                return;
            }
            
            String stateJson = storage.getStateJson();
            AsyncWebServerResponse* response = request->beginResponse(200, "application/json", stateJson);
            setCORSHeaders(response);
            request->send(response);
        }
    );
    
    // DELETE /api/consumption/{id} - Remove consumption record
    server.on("^\\/api\\/consumption\\/([a-zA-Z0-9]+)$", HTTP_DELETE, [&storage](AsyncWebServerRequest* request) {
        String consumptionId = request->pathArg(0);
        
        if (!storage.removeConsumption(consumptionId.c_str())) {
            sendError(request, "Consumption record not found", 404);
            return;
        }
        
        String stateJson = storage.getStateJson();
        AsyncWebServerResponse* response = request->beginResponse(200, "application/json", stateJson);
        setCORSHeaders(response);
        request->send(response);
    });
    
    // ========================================
    // Payments API
    // ========================================
    
    // POST /api/payments - Process payment
    server.on("/api/payments", HTTP_POST, [](AsyncWebServerRequest* request) {},
        NULL,
        [&storage](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
            DynamicJsonDocument doc(512);
            DeserializationError error = deserializeJson(doc, data, len);
            
            if (error) {
                sendError(request, "Invalid JSON");
                return;
            }
            
            const char* userId = doc["userId"];
            const char* itemId = doc["itemId"];
            float amount = doc["amount"] | 0.0f;
            
            if (!userId || !itemId || amount <= 0) {
                sendError(request, "Invalid input");
                return;
            }
            
            String id = String(millis());
            if (!storage.addPayment(id.c_str(), userId, itemId, amount)) {
                sendError(request, "Failed to process payment", 500);
                return;
            }
            
            String stateJson = storage.getStateJson();
            AsyncWebServerResponse* response = request->beginResponse(200, "application/json", stateJson);
            setCORSHeaders(response);
            request->send(response);
        }
    );
    
    // ========================================
    // Reset API
    // ========================================
    
    // POST /api/reset - Reset all data
    server.on("/api/reset", HTTP_POST, [&storage](AsyncWebServerRequest* request) {
        storage.reset();
        
        String stateJson = storage.getStateJson();
        AsyncWebServerResponse* response = request->beginResponse(200, "application/json", stateJson);
        setCORSHeaders(response);
        request->send(response);
    });
    
    // ========================================
    // 404 Handler
    // ========================================
    server.onNotFound([](AsyncWebServerRequest* request) {
        // For API routes, return JSON error
        if (request->url().startsWith("/api/")) {
            sendError(request, "Endpoint not found", 404);
            return;
        }
        
        // For other routes, try to serve index.html (SPA support)
        request->send(LittleFS, "/index.html", "text/html");
    });
    
    DEBUG_PRINTLN("[WEB] All routes configured");
}

#endif // WEB_HANDLERS_H
