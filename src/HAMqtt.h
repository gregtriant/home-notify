#pragma once
#include <Arduino.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "MQTTConfig.h"

#define MAX_HA_ENTITIES 8

struct HAEntity {
    String id;    // e.g. "button1"
    String name;  // e.g. "Home Notify Button 1"
};

class HAMqtt {
public:
    HAMqtt();
    void addEntity(const HAEntity &entity);
    void init();
    void loop();
    void publishEvent(const String &entityId, const String &action);
    bool isConnected();

private:
    WiFiClient wifiClient;
    PubSubClient mqttClient;

    HAEntity entities[MAX_HA_ENTITIES];
    uint8_t entityCount = 0;

    String baseTopic;
    String deviceId;  // MAC-derived, populated on first connect

    void connect();
    void setupDiscovery();
    void publishDiscoveryForEntity(const HAEntity &entity);
};
