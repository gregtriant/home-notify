#include "HAMqtt.h"
#include <WiFi.h>

HAMqtt::HAMqtt() : mqttClient(wifiClient) {
    baseTopic = "homeassistant/device/" + String(DEVICE_NAME);
}

void HAMqtt::addEntity(const HAEntity &entity) {
    if (entityCount < MAX_HA_ENTITIES) {
        entities[entityCount++] = entity;
    }
}

void HAMqtt::init() {
    mqttClient.setBufferSize(1024);
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    Serial.printf("MQTT server configured: %s:%d (buffer size: 1024)\n", MQTT_SERVER, MQTT_PORT);
}

void HAMqtt::connect() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected - skipping MQTT connection");
        return;
    }

    if (mqttClient.connected()) return;

    Serial.printf("Attempting MQTT connection to %s:%d...\n", MQTT_SERVER, MQTT_PORT);

    // Derive device ID from MAC on first connect
    if (deviceId.isEmpty()) {
        deviceId = WiFi.macAddress();
        deviceId.replace(":", "");
        deviceId.toLowerCase();
    }

    String clientId = String(DEVICE_NAME) + "-" + WiFi.macAddress();
    Serial.printf("Client ID: %s\n", clientId.c_str());

    bool connected;
    String username = MQTT_USERNAME;
    if (username.length() > 0) {
        Serial.printf("Connecting with username: %s\n", MQTT_USERNAME);
        connected = mqttClient.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD);
    } else {
        Serial.println("Connecting without authentication");
        connected = mqttClient.connect(clientId.c_str());
    }

    if (connected) {
        Serial.println("MQTT connected successfully!");
        Serial.printf("Local IP: %s\n", WiFi.localIP().toString().c_str());
        setupDiscovery();
    } else {
        Serial.printf("MQTT connection failed, rc=%d\n", mqttClient.state());
    }
}

void HAMqtt::loop() {
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 5000) {
        lastCheck = millis();
        if (!mqttClient.connected()) {
            Serial.println("MQTT not connected - attempting to connect...");
            connect();
        }
    }
    if (mqttClient.connected()) {
        mqttClient.loop();
    }
}

void HAMqtt::setupDiscovery() {
    Serial.println("Setting up Home Assistant autodiscovery...");
    for (uint8_t i = 0; i < entityCount; i++) {
        publishDiscoveryForEntity(entities[i]);
        delay(100);
    }
    Serial.println("Home Assistant autodiscovery configuration completed");
}

void HAMqtt::publishDiscoveryForEntity(const HAEntity &entity) {
    JsonDocument device;
    device["identifiers"][0] = deviceId;
    device["name"] = "Home Notify";
    device["manufacturer"] = "Custom";
    device["model"] = "ESP32";

    JsonDocument config;
    config["name"] = entity.name;
    config["unique_id"] = deviceId + "_" + entity.id;
    config["state_topic"] = baseTopic + "/" + entity.id + "/state";
    config["json_attributes_topic"] = baseTopic + "/" + entity.id + "/state";
    config["device"] = device;

    String payload;
    serializeJson(config, payload);

    String topic = "homeassistant/sensor/" + deviceId + "_" + entity.id + "/config";
    Serial.printf("Discovery [%s] payload size: %d bytes\n", entity.id.c_str(), payload.length());
    bool result = mqttClient.publish(topic.c_str(), payload.c_str(), true);
    Serial.printf("Discovery [%s] publish: %s\n", entity.id.c_str(), result ? "SUCCESS" : "FAILED");
}

void HAMqtt::publishEvent(const String &entityId, const String &action) {
    if (!mqttClient.connected()) {
        Serial.printf("Cannot publish event for %s - MQTT not connected\n", entityId.c_str());
        return;
    }
    String topic = baseTopic + "/" + entityId + "/state";
    String payload = "{\"action\":\"" + action + "\",\"timestamp\":" + String(millis()) + "}";
    bool result = mqttClient.publish(topic.c_str(), payload.c_str());
    Serial.printf("Published event [%s/%s]: %s\n", entityId.c_str(), action.c_str(), result ? "SUCCESS" : "FAILED");
}

bool HAMqtt::isConnected() {
    return mqttClient.connected();
}
