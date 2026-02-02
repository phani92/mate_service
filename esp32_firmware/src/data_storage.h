/**
 * Data Storage for Mate Tracker ESP32-C3
 * 
 * Handles persistent data storage using Preferences (NVS)
 * and provides CRUD operations for all data types.
 */

#ifndef DATA_STORAGE_H
#define DATA_STORAGE_H

#include <Preferences.h>
#include <ArduinoJson.h>
#include <vector>
#include "config.h"

// Data structures
struct User {
    String id;
    String name;
};

struct Item {
    String id;
    String name;
    float price;
    int initialStock;
};

struct ConsumptionRecord {
    String id;
    String userId;
    String itemId;
    int quantity;
    String timestamp;
};

struct PaymentRecord {
    String id;
    String userId;
    String itemId;
    float amount;
    String timestamp;
};

class DataStorage {
public:
    DataStorage() {}
    
    /**
     * Initialize data storage
     */
    bool begin() {
        DEBUG_PRINTLN("[DATA] Initializing preferences...");
        
        // Open preferences
        _prefs.begin(NVS_NAMESPACE, false);
        
        // Load data from NVS
        loadData();
        
        return true;
    }
    
    /**
     * Get full state as JSON string
     */
    String getStateJson() {
        DynamicJsonDocument doc(16384);
        
        // Users array
        JsonArray usersArray = doc.createNestedArray("users");
        for (const auto& user : _users) {
            JsonObject userObj = usersArray.createNestedObject();
            userObj["id"] = user.id;
            userObj["name"] = user.name;
        }
        
        // Items array
        JsonArray itemsArray = doc.createNestedArray("items");
        for (const auto& item : _items) {
            JsonObject itemObj = itemsArray.createNestedObject();
            itemObj["id"] = item.id;
            itemObj["name"] = item.name;
            itemObj["price"] = item.price;
            itemObj["initialStock"] = item.initialStock;
        }
        
        // Consumption array
        JsonArray consumptionArray = doc.createNestedArray("consumption");
        for (const auto& record : _consumption) {
            JsonObject recObj = consumptionArray.createNestedObject();
            recObj["id"] = record.id;
            recObj["userId"] = record.userId;
            recObj["itemId"] = record.itemId;
            recObj["quantity"] = record.quantity;
            recObj["timestamp"] = record.timestamp;
        }
        
        // Payments array
        JsonArray paymentsArray = doc.createNestedArray("payments");
        for (const auto& payment : _payments) {
            JsonObject payObj = paymentsArray.createNestedObject();
            payObj["id"] = payment.id;
            payObj["userId"] = payment.userId;
            payObj["itemId"] = payment.itemId;
            payObj["amount"] = payment.amount;
            payObj["timestamp"] = payment.timestamp;
        }
        
        String output;
        serializeJson(doc, output);
        return output;
    }
    
    // ========================================
    // User Operations
    // ========================================
    
    bool userExists(const char* name) {
        for (const auto& user : _users) {
            if (user.name.equalsIgnoreCase(name)) {
                return true;
            }
        }
        return false;
    }
    
    bool addUser(const char* id, const char* name) {
        if (_users.size() >= MAX_USERS) {
            DEBUG_PRINTLN("[DATA] Max users reached");
            return false;
        }
        
        User user;
        user.id = id;
        user.name = name;
        _users.push_back(user);
        
        saveData();
        return true;
    }
    
    bool removeUser(const char* id) {
        String idStr = id;
        
        // Remove user
        auto it = _users.begin();
        while (it != _users.end()) {
            if (it->id == idStr) {
                it = _users.erase(it);
                
                // Also remove related consumption and payments
                removeConsumptionByUser(id);
                removePaymentsByUser(id);
                
                saveData();
                return true;
            } else {
                ++it;
            }
        }
        return false;
    }
    
    // ========================================
    // Item Operations
    // ========================================
    
    bool itemExists(const char* name) {
        for (const auto& item : _items) {
            if (item.name.equalsIgnoreCase(name)) {
                return true;
            }
        }
        return false;
    }
    
    bool addItem(const char* id, const char* name, float price, int stock) {
        if (_items.size() >= MAX_ITEMS) {
            DEBUG_PRINTLN("[DATA] Max items reached");
            return false;
        }
        
        Item item;
        item.id = id;
        item.name = name;
        item.price = price;
        item.initialStock = stock;
        _items.push_back(item);
        
        saveData();
        return true;
    }
    
    bool removeItem(const char* id) {
        String idStr = id;
        
        auto it = _items.begin();
        while (it != _items.end()) {
            if (it->id == idStr) {
                it = _items.erase(it);
                
                // Also remove related consumption and payments
                removeConsumptionByItem(id);
                removePaymentsByItem(id);
                
                saveData();
                return true;
            } else {
                ++it;
            }
        }
        return false;
    }
    
    bool updateItemStock(const char* id, int stock) {
        String idStr = id;
        
        for (auto& item : _items) {
            if (item.id == idStr) {
                item.initialStock = stock;
                saveData();
                return true;
            }
        }
        return false;
    }
    
    int getAvailableStock(const char* itemId) {
        String idStr = itemId;
        
        // Find item
        int initialStock = 0;
        for (const auto& item : _items) {
            if (item.id == idStr) {
                initialStock = item.initialStock;
                break;
            }
        }
        
        // Calculate consumed
        int consumed = 0;
        for (const auto& record : _consumption) {
            if (record.itemId == idStr) {
                consumed += record.quantity;
            }
        }
        
        return initialStock - consumed;
    }
    
    // ========================================
    // Consumption Operations
    // ========================================
    
    bool addConsumption(const char* id, const char* userId, const char* itemId, int quantity) {
        if (_consumption.size() >= MAX_CONSUMPTION_RECORDS) {
            DEBUG_PRINTLN("[DATA] Max consumption records reached");
            return false;
        }
        
        ConsumptionRecord record;
        record.id = id;
        record.userId = userId;
        record.itemId = itemId;
        record.quantity = quantity;
        record.timestamp = String(millis());
        _consumption.push_back(record);
        
        saveData();
        return true;
    }
    
    bool removeConsumption(const char* id) {
        String idStr = id;
        
        auto it = _consumption.begin();
        while (it != _consumption.end()) {
            if (it->id == idStr) {
                it = _consumption.erase(it);
                saveData();
                return true;
            } else {
                ++it;
            }
        }
        return false;
    }
    
    void removeConsumptionByUser(const char* userId) {
        String idStr = userId;
        
        auto it = _consumption.begin();
        while (it != _consumption.end()) {
            if (it->userId == idStr) {
                it = _consumption.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    void removeConsumptionByItem(const char* itemId) {
        String idStr = itemId;
        
        auto it = _consumption.begin();
        while (it != _consumption.end()) {
            if (it->itemId == idStr) {
                it = _consumption.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    // ========================================
    // Payment Operations
    // ========================================
    
    bool addPayment(const char* id, const char* userId, const char* itemId, float amount) {
        if (_payments.size() >= MAX_PAYMENT_RECORDS) {
            DEBUG_PRINTLN("[DATA] Max payment records reached");
            return false;
        }
        
        PaymentRecord payment;
        payment.id = id;
        payment.userId = userId;
        payment.itemId = itemId;
        payment.amount = amount;
        payment.timestamp = String(millis());
        _payments.push_back(payment);
        
        saveData();
        return true;
    }
    
    void removePaymentsByUser(const char* userId) {
        String idStr = userId;
        
        auto it = _payments.begin();
        while (it != _payments.end()) {
            if (it->userId == idStr) {
                it = _payments.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    void removePaymentsByItem(const char* itemId) {
        String idStr = itemId;
        
        auto it = _payments.begin();
        while (it != _payments.end()) {
            if (it->itemId == idStr) {
                it = _payments.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    // ========================================
    // Reset / Clear
    // ========================================
    
    void reset() {
        _users.clear();
        _items.clear();
        _consumption.clear();
        _payments.clear();
        
        saveData();
        DEBUG_PRINTLN("[DATA] All data reset");
    }

private:
    Preferences _prefs;
    std::vector<User> _users;
    std::vector<Item> _items;
    std::vector<ConsumptionRecord> _consumption;
    std::vector<PaymentRecord> _payments;
    
    /**
     * Load all data from NVS
     */
    void loadData() {
        String stateJson = _prefs.getString("state", "{}");
        
        if (stateJson == "{}") {
            DEBUG_PRINTLN("[DATA] No saved data found, starting fresh");
            return;
        }
        
        DynamicJsonDocument doc(16384);
        DeserializationError error = deserializeJson(doc, stateJson);
        
        if (error) {
            DEBUG_PRINTF("[DATA] Error parsing saved data: %s\n", error.c_str());
            return;
        }
        
        // Load users
        JsonArray usersArray = doc["users"].as<JsonArray>();
        for (JsonObject userObj : usersArray) {
            User user;
            user.id = userObj["id"].as<String>();
            user.name = userObj["name"].as<String>();
            _users.push_back(user);
        }
        
        // Load items
        JsonArray itemsArray = doc["items"].as<JsonArray>();
        for (JsonObject itemObj : itemsArray) {
            Item item;
            item.id = itemObj["id"].as<String>();
            item.name = itemObj["name"].as<String>();
            item.price = itemObj["price"].as<float>();
            item.initialStock = itemObj["initialStock"].as<int>();
            _items.push_back(item);
        }
        
        // Load consumption
        JsonArray consumptionArray = doc["consumption"].as<JsonArray>();
        for (JsonObject recObj : consumptionArray) {
            ConsumptionRecord record;
            record.id = recObj["id"].as<String>();
            record.userId = recObj["userId"].as<String>();
            record.itemId = recObj["itemId"].as<String>();
            record.quantity = recObj["quantity"].as<int>();
            record.timestamp = recObj["timestamp"].as<String>();
            _consumption.push_back(record);
        }
        
        // Load payments
        JsonArray paymentsArray = doc["payments"].as<JsonArray>();
        for (JsonObject payObj : paymentsArray) {
            PaymentRecord payment;
            payment.id = payObj["id"].as<String>();
            payment.userId = payObj["userId"].as<String>();
            payment.itemId = payObj["itemId"].as<String>();
            payment.amount = payObj["amount"].as<float>();
            payment.timestamp = payObj["timestamp"].as<String>();
            _payments.push_back(payment);
        }
        
        DEBUG_PRINTF("[DATA] Loaded %d users, %d items, %d consumption records, %d payments\n",
            _users.size(), _items.size(), _consumption.size(), _payments.size());
    }
    
    /**
     * Save all data to NVS
     */
    void saveData() {
        String stateJson = getStateJson();
        
        // NVS has limited space, check size
        if (stateJson.length() > 15000) {
            DEBUG_PRINTLN("[DATA] WARNING: Data size exceeds recommended limit!");
        }
        
        _prefs.putString("state", stateJson);
        DEBUG_PRINTLN("[DATA] State saved to NVS");
    }
};

#endif // DATA_STORAGE_H
