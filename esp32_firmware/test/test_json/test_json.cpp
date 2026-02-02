/**
 * Unit Tests for JSON Operations
 * 
 * Tests JSON serialization/deserialization for API payloads.
 */

#include <unity.h>
#include <Arduino.h>
#include <ArduinoJson.h>

void setUp(void) {
    // Nothing to set up
}

void tearDown(void) {
    // Nothing to tear down
}

// ============================================
// JSON Parsing Tests
// ============================================

void test_parse_user_json(void) {
    const char* json = "{\"name\":\"Alice\"}";
    
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, json);
    
    TEST_ASSERT_FALSE(error);
    TEST_ASSERT_EQUAL_STRING("Alice", doc["name"].as<const char*>());
}

void test_parse_item_json(void) {
    const char* json = "{\"name\":\"Coffee\",\"price\":2.50,\"initialStock\":100}";
    
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, json);
    
    TEST_ASSERT_FALSE(error);
    TEST_ASSERT_EQUAL_STRING("Coffee", doc["name"].as<const char*>());
    TEST_ASSERT_EQUAL_FLOAT(2.50, doc["price"].as<float>());
    TEST_ASSERT_EQUAL(100, doc["initialStock"].as<int>());
}

void test_parse_consumption_json(void) {
    const char* json = "{\"userId\":\"user1\",\"itemId\":\"item1\",\"quantity\":5}";
    
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, json);
    
    TEST_ASSERT_FALSE(error);
    TEST_ASSERT_EQUAL_STRING("user1", doc["userId"].as<const char*>());
    TEST_ASSERT_EQUAL_STRING("item1", doc["itemId"].as<const char*>());
    TEST_ASSERT_EQUAL(5, doc["quantity"].as<int>());
}

void test_parse_payment_json(void) {
    const char* json = "{\"userId\":\"user1\",\"itemId\":\"item1\",\"amount\":25.00}";
    
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, json);
    
    TEST_ASSERT_FALSE(error);
    TEST_ASSERT_EQUAL_STRING("user1", doc["userId"].as<const char*>());
    TEST_ASSERT_EQUAL_STRING("item1", doc["itemId"].as<const char*>());
    TEST_ASSERT_EQUAL_FLOAT(25.00, doc["amount"].as<float>());
}

void test_parse_invalid_json(void) {
    const char* json = "{invalid json}";
    
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, json);
    
    TEST_ASSERT_TRUE(error);
}

void test_parse_empty_json(void) {
    const char* json = "{}";
    
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, json);
    
    TEST_ASSERT_FALSE(error);
    TEST_ASSERT_FALSE(doc.containsKey("name"));
}

void test_missing_required_field(void) {
    const char* json = "{\"other\":\"value\"}";
    
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, json);
    
    TEST_ASSERT_FALSE(error);
    
    // Check that "name" is null/missing
    const char* name = doc["name"];
    TEST_ASSERT_NULL(name);
}

// ============================================
// JSON Serialization Tests
// ============================================

void test_serialize_state_json(void) {
    DynamicJsonDocument doc(2048);
    
    // Create users array
    JsonArray users = doc.createNestedArray("users");
    JsonObject user = users.createNestedObject();
    user["id"] = "user1";
    user["name"] = "Alice";
    
    // Create items array
    JsonArray items = doc.createNestedArray("items");
    JsonObject item = items.createNestedObject();
    item["id"] = "item1";
    item["name"] = "Coffee";
    item["price"] = 2.50;
    item["initialStock"] = 100;
    
    // Create empty arrays
    doc.createNestedArray("consumption");
    doc.createNestedArray("payments");
    
    String output;
    serializeJson(doc, output);
    
    // Parse back and verify
    DynamicJsonDocument parsed(2048);
    DeserializationError error = deserializeJson(parsed, output);
    
    TEST_ASSERT_FALSE(error);
    TEST_ASSERT_EQUAL(1, parsed["users"].size());
    TEST_ASSERT_EQUAL(1, parsed["items"].size());
    TEST_ASSERT_EQUAL(0, parsed["consumption"].size());
    TEST_ASSERT_EQUAL(0, parsed["payments"].size());
}

void test_serialize_float_precision(void) {
    DynamicJsonDocument doc(256);
    doc["price"] = 2.50;
    doc["amount"] = 12.99;
    
    String output;
    serializeJson(doc, output);
    
    DynamicJsonDocument parsed(256);
    deserializeJson(parsed, output);
    
    TEST_ASSERT_EQUAL_FLOAT(2.50, parsed["price"].as<float>());
    TEST_ASSERT_EQUAL_FLOAT(12.99, parsed["amount"].as<float>());
}

void test_serialize_nested_objects(void) {
    DynamicJsonDocument doc(512);
    
    JsonObject wifi = doc.createNestedObject("wifi");
    wifi["connected"] = true;
    wifi["ssid"] = "TestNetwork";
    wifi["rssi"] = -65;
    
    String output;
    serializeJson(doc, output);
    
    DynamicJsonDocument parsed(512);
    deserializeJson(parsed, output);
    
    TEST_ASSERT_TRUE(parsed["wifi"]["connected"].as<bool>());
    TEST_ASSERT_EQUAL_STRING("TestNetwork", parsed["wifi"]["ssid"].as<const char*>());
    TEST_ASSERT_EQUAL(-65, parsed["wifi"]["rssi"].as<int>());
}

// ============================================
// Document Size Tests
// ============================================

void test_document_size_for_state(void) {
    // Test that our document size is adequate
    DynamicJsonDocument doc(16384);
    
    // Add 20 users
    JsonArray users = doc.createNestedArray("users");
    for (int i = 0; i < 20; i++) {
        JsonObject user = users.createNestedObject();
        user["id"] = String("user") + String(i);
        user["name"] = String("User Name ") + String(i);
    }
    
    // Add 50 items
    JsonArray items = doc.createNestedArray("items");
    for (int i = 0; i < 50; i++) {
        JsonObject item = items.createNestedObject();
        item["id"] = String("item") + String(i);
        item["name"] = String("Item Name ") + String(i);
        item["price"] = 2.50 + (i * 0.1);
        item["initialStock"] = 100 + i;
    }
    
    // Add 100 consumption records
    JsonArray consumption = doc.createNestedArray("consumption");
    for (int i = 0; i < 100; i++) {
        JsonObject record = consumption.createNestedObject();
        record["id"] = String("cons") + String(i);
        record["userId"] = "user1";
        record["itemId"] = "item1";
        record["quantity"] = 1;
        record["timestamp"] = "12345678";
    }
    
    // Add 50 payments
    JsonArray payments = doc.createNestedArray("payments");
    for (int i = 0; i < 50; i++) {
        JsonObject payment = payments.createNestedObject();
        payment["id"] = String("pay") + String(i);
        payment["userId"] = "user1";
        payment["itemId"] = "item1";
        payment["amount"] = 10.00;
        payment["timestamp"] = "12345678";
    }
    
    // Should not overflow
    TEST_ASSERT_FALSE(doc.overflowed());
    
    String output;
    serializeJson(doc, output);
    
    // Output should not be empty
    TEST_ASSERT_TRUE(output.length() > 0);
}

// ============================================
// Test Runner
// ============================================

void setup() {
    delay(2000);  // Wait for serial
    
    UNITY_BEGIN();
    
    // JSON parsing tests
    RUN_TEST(test_parse_user_json);
    RUN_TEST(test_parse_item_json);
    RUN_TEST(test_parse_consumption_json);
    RUN_TEST(test_parse_payment_json);
    RUN_TEST(test_parse_invalid_json);
    RUN_TEST(test_parse_empty_json);
    RUN_TEST(test_missing_required_field);
    
    // JSON serialization tests
    RUN_TEST(test_serialize_state_json);
    RUN_TEST(test_serialize_float_precision);
    RUN_TEST(test_serialize_nested_objects);
    
    // Document size tests
    RUN_TEST(test_document_size_for_state);
    
    UNITY_END();
}

void loop() {
    // Nothing to do here
}
