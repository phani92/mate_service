/**
 * Unit Tests for DataStorage class
 * 
 * Tests CRUD operations for users, items, consumption, and payments.
 * Uses Unity test framework (PlatformIO native testing).
 */

#include <unity.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <vector>

// Configuration constants (reuse config.h, override NVS_NAMESPACE for tests)
#include "config.h"
#undef NVS_NAMESPACE
#define NVS_NAMESPACE "test_data"

// Data structures (copied from data_storage.h to avoid include conflicts)
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
    
    bool begin() {
        _prefs.begin(NVS_NAMESPACE, false);
        loadData();
        return true;
    }
    
    String getStateJson() {
        DynamicJsonDocument doc(16384);
        
        JsonArray usersArray = doc.createNestedArray("users");
        for (const auto& user : _users) {
            JsonObject userObj = usersArray.createNestedObject();
            userObj["id"] = user.id;
            userObj["name"] = user.name;
        }
        
        JsonArray itemsArray = doc.createNestedArray("items");
        for (const auto& item : _items) {
            JsonObject itemObj = itemsArray.createNestedObject();
            itemObj["id"] = item.id;
            itemObj["name"] = item.name;
            itemObj["price"] = item.price;
            itemObj["initialStock"] = item.initialStock;
        }
        
        JsonArray consumptionArray = doc.createNestedArray("consumption");
        for (const auto& record : _consumption) {
            JsonObject recObj = consumptionArray.createNestedObject();
            recObj["id"] = record.id;
            recObj["userId"] = record.userId;
            recObj["itemId"] = record.itemId;
            recObj["quantity"] = record.quantity;
            recObj["timestamp"] = record.timestamp;
        }
        
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
    
    bool userExists(const char* name) {
        for (const auto& user : _users) {
            if (user.name.equalsIgnoreCase(name)) {
                return true;
            }
        }
        return false;
    }
    
    bool addUser(const char* id, const char* name) {
        if (_users.size() >= MAX_USERS) return false;
        User user;
        user.id = id;
        user.name = name;
        _users.push_back(user);
        saveData();
        return true;
    }
    
    bool removeUser(const char* id) {
        String idStr = id;
        auto it = _users.begin();
        while (it != _users.end()) {
            if (it->id == idStr) {
                it = _users.erase(it);
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
    
    bool itemExists(const char* name) {
        for (const auto& item : _items) {
            if (item.name.equalsIgnoreCase(name)) {
                return true;
            }
        }
        return false;
    }
    
    bool addItem(const char* id, const char* name, float price, int stock) {
        if (_items.size() >= MAX_ITEMS) return false;
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
        int initialStock = 0;
        for (const auto& item : _items) {
            if (item.id == idStr) {
                initialStock = item.initialStock;
                break;
            }
        }
        int consumed = 0;
        for (const auto& record : _consumption) {
            if (record.itemId == idStr) {
                consumed += record.quantity;
            }
        }
        return initialStock - consumed;
    }
    
    bool addConsumption(const char* id, const char* userId, const char* itemId, int quantity) {
        if (_consumption.size() >= MAX_CONSUMPTION_RECORDS) return false;
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
    
    bool addPayment(const char* id, const char* userId, const char* itemId, float amount) {
        if (_payments.size() >= MAX_PAYMENT_RECORDS) return false;
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
    
    void reset() {
        _users.clear();
        _items.clear();
        _consumption.clear();
        _payments.clear();
        saveData();
    }

private:
    Preferences _prefs;
    std::vector<User> _users;
    std::vector<Item> _items;
    std::vector<ConsumptionRecord> _consumption;
    std::vector<PaymentRecord> _payments;
    
    void loadData() {
        String stateJson = _prefs.getString("state", "{}");
        if (stateJson == "{}") return;
        
        DynamicJsonDocument doc(16384);
        DeserializationError error = deserializeJson(doc, stateJson);
        if (error) return;
        
        JsonArray usersArray = doc["users"].as<JsonArray>();
        for (JsonObject userObj : usersArray) {
            User user;
            user.id = userObj["id"].as<String>();
            user.name = userObj["name"].as<String>();
            _users.push_back(user);
        }
        
        JsonArray itemsArray = doc["items"].as<JsonArray>();
        for (JsonObject itemObj : itemsArray) {
            Item item;
            item.id = itemObj["id"].as<String>();
            item.name = itemObj["name"].as<String>();
            item.price = itemObj["price"].as<float>();
            item.initialStock = itemObj["initialStock"].as<int>();
            _items.push_back(item);
        }
        
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
    }
    
    void saveData() {
        String stateJson = getStateJson();
        _prefs.putString("state", stateJson);
    }
};

// Test instance
DataStorage* storage = nullptr;

void setUp(void) {
    storage = new DataStorage();
    storage->begin();
    storage->reset();  // Start with clean state
}

void tearDown(void) {
    if (storage) {
        storage->reset();
        delete storage;
        storage = nullptr;
    }
}

// ============================================
// User Tests
// ============================================

void test_add_user_success(void) {
    bool result = storage->addUser("user1", "Alice");
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(storage->userExists("Alice"));
}

void test_add_user_case_insensitive_check(void) {
    storage->addUser("user1", "Alice");
    TEST_ASSERT_TRUE(storage->userExists("alice"));
    TEST_ASSERT_TRUE(storage->userExists("ALICE"));
    TEST_ASSERT_TRUE(storage->userExists("Alice"));
}

void test_user_does_not_exist(void) {
    TEST_ASSERT_FALSE(storage->userExists("NonExistent"));
}

void test_add_multiple_users(void) {
    TEST_ASSERT_TRUE(storage->addUser("user1", "Alice"));
    TEST_ASSERT_TRUE(storage->addUser("user2", "Bob"));
    TEST_ASSERT_TRUE(storage->addUser("user3", "Charlie"));
    
    TEST_ASSERT_TRUE(storage->userExists("Alice"));
    TEST_ASSERT_TRUE(storage->userExists("Bob"));
    TEST_ASSERT_TRUE(storage->userExists("Charlie"));
}

void test_remove_user_success(void) {
    storage->addUser("user1", "Alice");
    TEST_ASSERT_TRUE(storage->userExists("Alice"));
    
    bool result = storage->removeUser("user1");
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_FALSE(storage->userExists("Alice"));
}

void test_remove_nonexistent_user(void) {
    bool result = storage->removeUser("nonexistent");
    TEST_ASSERT_FALSE(result);
}

void test_remove_user_removes_related_consumption(void) {
    storage->addUser("user1", "Alice");
    storage->addItem("item1", "Coffee", 2.50, 100);
    storage->addConsumption("cons1", "user1", "item1", 1);
    
    // Verify consumption exists (check via stock)
    TEST_ASSERT_EQUAL(99, storage->getAvailableStock("item1"));
    
    // Remove user
    storage->removeUser("user1");
    
    // Consumption should be removed, stock restored
    TEST_ASSERT_EQUAL(100, storage->getAvailableStock("item1"));
}

// ============================================
// Item Tests
// ============================================

void test_add_item_success(void) {
    bool result = storage->addItem("item1", "Coffee", 2.50, 100);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(storage->itemExists("Coffee"));
}

void test_item_exists_case_insensitive(void) {
    storage->addItem("item1", "Coffee", 2.50, 100);
    TEST_ASSERT_TRUE(storage->itemExists("coffee"));
    TEST_ASSERT_TRUE(storage->itemExists("COFFEE"));
    TEST_ASSERT_TRUE(storage->itemExists("Coffee"));
}

void test_item_does_not_exist(void) {
    TEST_ASSERT_FALSE(storage->itemExists("NonExistent"));
}

void test_add_multiple_items(void) {
    TEST_ASSERT_TRUE(storage->addItem("item1", "Coffee", 2.50, 100));
    TEST_ASSERT_TRUE(storage->addItem("item2", "Tea", 1.50, 50));
    TEST_ASSERT_TRUE(storage->addItem("item3", "Mate", 3.00, 200));
    
    TEST_ASSERT_TRUE(storage->itemExists("Coffee"));
    TEST_ASSERT_TRUE(storage->itemExists("Tea"));
    TEST_ASSERT_TRUE(storage->itemExists("Mate"));
}

void test_remove_item_success(void) {
    storage->addItem("item1", "Coffee", 2.50, 100);
    TEST_ASSERT_TRUE(storage->itemExists("Coffee"));
    
    bool result = storage->removeItem("item1");
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_FALSE(storage->itemExists("Coffee"));
}

void test_remove_nonexistent_item(void) {
    bool result = storage->removeItem("nonexistent");
    TEST_ASSERT_FALSE(result);
}

void test_update_item_stock(void) {
    storage->addItem("item1", "Coffee", 2.50, 100);
    
    bool result = storage->updateItemStock("item1", 150);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(150, storage->getAvailableStock("item1"));
}

void test_update_nonexistent_item_stock(void) {
    bool result = storage->updateItemStock("nonexistent", 100);
    TEST_ASSERT_FALSE(result);
}

void test_get_available_stock_initial(void) {
    storage->addItem("item1", "Coffee", 2.50, 100);
    TEST_ASSERT_EQUAL(100, storage->getAvailableStock("item1"));
}

void test_get_available_stock_after_consumption(void) {
    storage->addItem("item1", "Coffee", 2.50, 100);
    storage->addUser("user1", "Alice");
    
    storage->addConsumption("cons1", "user1", "item1", 5);
    TEST_ASSERT_EQUAL(95, storage->getAvailableStock("item1"));
    
    storage->addConsumption("cons2", "user1", "item1", 10);
    TEST_ASSERT_EQUAL(85, storage->getAvailableStock("item1"));
}

void test_get_available_stock_nonexistent_item(void) {
    TEST_ASSERT_EQUAL(0, storage->getAvailableStock("nonexistent"));
}

// ============================================
// Consumption Tests
// ============================================

void test_add_consumption_success(void) {
    storage->addUser("user1", "Alice");
    storage->addItem("item1", "Coffee", 2.50, 100);
    
    bool result = storage->addConsumption("cons1", "user1", "item1", 1);
    TEST_ASSERT_TRUE(result);
}

void test_add_multiple_consumptions(void) {
    storage->addUser("user1", "Alice");
    storage->addItem("item1", "Coffee", 2.50, 100);
    
    TEST_ASSERT_TRUE(storage->addConsumption("cons1", "user1", "item1", 1));
    TEST_ASSERT_TRUE(storage->addConsumption("cons2", "user1", "item1", 2));
    TEST_ASSERT_TRUE(storage->addConsumption("cons3", "user1", "item1", 3));
    
    // Total consumed: 6, available: 94
    TEST_ASSERT_EQUAL(94, storage->getAvailableStock("item1"));
}

void test_remove_consumption_success(void) {
    storage->addUser("user1", "Alice");
    storage->addItem("item1", "Coffee", 2.50, 100);
    storage->addConsumption("cons1", "user1", "item1", 10);
    
    TEST_ASSERT_EQUAL(90, storage->getAvailableStock("item1"));
    
    bool result = storage->removeConsumption("cons1");
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(100, storage->getAvailableStock("item1"));
}

void test_remove_nonexistent_consumption(void) {
    bool result = storage->removeConsumption("nonexistent");
    TEST_ASSERT_FALSE(result);
}

// ============================================
// Payment Tests
// ============================================

void test_add_payment_success(void) {
    storage->addUser("user1", "Alice");
    storage->addItem("item1", "Coffee", 2.50, 100);
    
    bool result = storage->addPayment("pay1", "user1", "item1", 25.00);
    TEST_ASSERT_TRUE(result);
}

void test_add_multiple_payments(void) {
    storage->addUser("user1", "Alice");
    storage->addItem("item1", "Coffee", 2.50, 100);
    
    TEST_ASSERT_TRUE(storage->addPayment("pay1", "user1", "item1", 10.00));
    TEST_ASSERT_TRUE(storage->addPayment("pay2", "user1", "item1", 15.00));
    TEST_ASSERT_TRUE(storage->addPayment("pay3", "user1", "item1", 5.00));
}

// ============================================
// JSON State Tests
// ============================================

void test_get_state_json_empty(void) {
    String json = storage->getStateJson();
    
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, json);
    
    TEST_ASSERT_FALSE(error);
    TEST_ASSERT_TRUE(doc.containsKey("users"));
    TEST_ASSERT_TRUE(doc.containsKey("items"));
    TEST_ASSERT_TRUE(doc.containsKey("consumption"));
    TEST_ASSERT_TRUE(doc.containsKey("payments"));
    
    TEST_ASSERT_EQUAL(0, doc["users"].size());
    TEST_ASSERT_EQUAL(0, doc["items"].size());
    TEST_ASSERT_EQUAL(0, doc["consumption"].size());
    TEST_ASSERT_EQUAL(0, doc["payments"].size());
}

void test_get_state_json_with_data(void) {
    storage->addUser("user1", "Alice");
    storage->addItem("item1", "Coffee", 2.50, 100);
    storage->addConsumption("cons1", "user1", "item1", 5);
    storage->addPayment("pay1", "user1", "item1", 12.50);
    
    String json = storage->getStateJson();
    
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, json);
    
    TEST_ASSERT_FALSE(error);
    TEST_ASSERT_EQUAL(1, doc["users"].size());
    TEST_ASSERT_EQUAL(1, doc["items"].size());
    TEST_ASSERT_EQUAL(1, doc["consumption"].size());
    TEST_ASSERT_EQUAL(1, doc["payments"].size());
    
    // Verify user data
    TEST_ASSERT_EQUAL_STRING("user1", doc["users"][0]["id"].as<const char*>());
    TEST_ASSERT_EQUAL_STRING("Alice", doc["users"][0]["name"].as<const char*>());
    
    // Verify item data
    TEST_ASSERT_EQUAL_STRING("item1", doc["items"][0]["id"].as<const char*>());
    TEST_ASSERT_EQUAL_STRING("Coffee", doc["items"][0]["name"].as<const char*>());
    TEST_ASSERT_EQUAL_FLOAT(2.50, doc["items"][0]["price"].as<float>());
    TEST_ASSERT_EQUAL(100, doc["items"][0]["initialStock"].as<int>());
    
    // Verify consumption data
    TEST_ASSERT_EQUAL_STRING("cons1", doc["consumption"][0]["id"].as<const char*>());
    TEST_ASSERT_EQUAL(5, doc["consumption"][0]["quantity"].as<int>());
    
    // Verify payment data
    TEST_ASSERT_EQUAL_STRING("pay1", doc["payments"][0]["id"].as<const char*>());
    TEST_ASSERT_EQUAL_FLOAT(12.50, doc["payments"][0]["amount"].as<float>());
}

// ============================================
// Reset Tests
// ============================================

void test_reset_clears_all_data(void) {
    storage->addUser("user1", "Alice");
    storage->addUser("user2", "Bob");
    storage->addItem("item1", "Coffee", 2.50, 100);
    storage->addConsumption("cons1", "user1", "item1", 5);
    storage->addPayment("pay1", "user1", "item1", 12.50);
    
    storage->reset();
    
    String json = storage->getStateJson();
    DynamicJsonDocument doc(4096);
    deserializeJson(doc, json);
    
    TEST_ASSERT_EQUAL(0, doc["users"].size());
    TEST_ASSERT_EQUAL(0, doc["items"].size());
    TEST_ASSERT_EQUAL(0, doc["consumption"].size());
    TEST_ASSERT_EQUAL(0, doc["payments"].size());
    
    TEST_ASSERT_FALSE(storage->userExists("Alice"));
    TEST_ASSERT_FALSE(storage->itemExists("Coffee"));
}

// ============================================
// Cascade Delete Tests
// ============================================

void test_remove_item_removes_related_records(void) {
    storage->addUser("user1", "Alice");
    storage->addItem("item1", "Coffee", 2.50, 100);
    storage->addItem("item2", "Tea", 1.50, 50);
    
    storage->addConsumption("cons1", "user1", "item1", 5);
    storage->addConsumption("cons2", "user1", "item2", 3);
    storage->addPayment("pay1", "user1", "item1", 12.50);
    
    // Remove item1 - should remove its consumption and payment
    storage->removeItem("item1");
    
    // item2 should still have its consumption
    TEST_ASSERT_EQUAL(47, storage->getAvailableStock("item2"));
    
    // Verify via JSON that item1 related records are gone
    String json = storage->getStateJson();
    DynamicJsonDocument doc(8192);
    deserializeJson(doc, json);
    
    // Only 1 item, 1 consumption should remain
    TEST_ASSERT_EQUAL(1, doc["items"].size());
    TEST_ASSERT_EQUAL(1, doc["consumption"].size());
    TEST_ASSERT_EQUAL(0, doc["payments"].size());  // Only payment was for item1
}

// ============================================
// Test Runner
// ============================================

void setup() {
    delay(2000);  // Wait for serial
    
    UNITY_BEGIN();
    
    // User tests
    RUN_TEST(test_add_user_success);
    RUN_TEST(test_add_user_case_insensitive_check);
    RUN_TEST(test_user_does_not_exist);
    RUN_TEST(test_add_multiple_users);
    RUN_TEST(test_remove_user_success);
    RUN_TEST(test_remove_nonexistent_user);
    RUN_TEST(test_remove_user_removes_related_consumption);
    
    // Item tests
    RUN_TEST(test_add_item_success);
    RUN_TEST(test_item_exists_case_insensitive);
    RUN_TEST(test_item_does_not_exist);
    RUN_TEST(test_add_multiple_items);
    RUN_TEST(test_remove_item_success);
    RUN_TEST(test_remove_nonexistent_item);
    RUN_TEST(test_update_item_stock);
    RUN_TEST(test_update_nonexistent_item_stock);
    RUN_TEST(test_get_available_stock_initial);
    RUN_TEST(test_get_available_stock_after_consumption);
    RUN_TEST(test_get_available_stock_nonexistent_item);
    
    // Consumption tests
    RUN_TEST(test_add_consumption_success);
    RUN_TEST(test_add_multiple_consumptions);
    RUN_TEST(test_remove_consumption_success);
    RUN_TEST(test_remove_nonexistent_consumption);
    
    // Payment tests
    RUN_TEST(test_add_payment_success);
    RUN_TEST(test_add_multiple_payments);
    
    // JSON state tests
    RUN_TEST(test_get_state_json_empty);
    RUN_TEST(test_get_state_json_with_data);
    
    // Reset tests
    RUN_TEST(test_reset_clears_all_data);
    
    // Cascade delete tests
    RUN_TEST(test_remove_item_removes_related_records);
    
    UNITY_END();
}

void loop() {
    // Nothing to do here
}
