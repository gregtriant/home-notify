#include <Adafruit_BME280.h>
#include <Arduino.h>
#include <SocketClient.h>
#include <Wire.h>
#include <time.h>
#include <I2CScanner.h>

#include "App.h"
#include "Definitions.h"
#include "globals.h"
#include <Mqtt/HAMqtt.h>
#include "MQTTConfig.h"

I2CScanner scanner;
SocketClient socketClient;
App *app;
HAMqtt *mqttClient = nullptr;

void sendStatus(JsonDoc status)
{
    if (!app) {
        status["message"] = "App not initialized";
        return;
    }
    status["message"] = app->getMessage();
}

void receivedCommand(JsonDoc doc)
{
    if (!app) {
        Serial.println("App not initialized");
        return;
    }
    String command = doc["data"].as<String>();
    // If command starts with @, it is a command, else set message.
    if (command.startsWith("@")) {
        if (command == "@clear") {
            app->setMessage("");
            app->showMessage(app->getMessage());
        } else if (command == "@ledon") {
            app->ledON();
        } else if (command == "@ledoff") {
            app->ledOFF();
        } else if (command == "@reboot") {
            socketClient.disconnect();
            ESP.restart();
        }

    } else if (command != app->getMessage()) {  // New message recieved
        app->recievedMessage(command);
    }
    socketClient.sendStatusWithSocket();
}

void entityChanged(JsonDoc doc)
{
    if (strcmp(doc["entity"], "counter") == 0) {
        // counter = doc["value"].as<int>();
    }
    socketClient.sendStatusWithSocket();
}

void connected(JsonDoc doc)
{
    Serial.print("Connected data:  ");
    serializeJson(doc, Serial);
    Serial.println();
    // do something with the connected data
    String defaultMessage = doc["default_message"].as<String>();
    Serial.print("Default message: ");
    Serial.println(defaultMessage);
    if (app) {
        app->setDefaultMessage(defaultMessage);
    }

    socketClient.sendNotification("Connected!");
    socketClient.sendStatusWithSocket(true);
}

static const HAMqttConfig_t mqttCfg = {
    .server      = MQTT_SERVER,
    .port        = MQTT_PORT,
    .username    = MQTT_USERNAME,
    .password    = MQTT_PASSWORD,
    .deviceName  = DEVICE_NAME,
    .displayName = "Home Notify",
};

/**
 * Configuration for the SocketClient
 */
SocketClientConfig_t config = {
    .name = "Home-Notify",
    .version = VERSION,
    .type = "ESP32",
    .ledPin = LED_PIN,
    .host = "api.sensordata.space", //"insecure2.sensordata.space",
    .port = 443,
    .isSSL = true,
    .token = token,  // from globals.h
    .handleWifi = true,
    .sendStatus = sendStatus,
    .receivedCommand = receivedCommand,
    .entityChanged = entityChanged,
    .connected = connected,
};

void setup()
{
    Serial.begin(115200);
    while(!Serial) {;}
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // turn off LED
    pinMode(BUTTON_PIN, INPUT_PULLUP); // Set button pin with pullup
    pinMode(BUTTON1, INPUT_PULLUP);    // Set button1 pin with pullup
    pinMode(BUTTON2, INPUT_PULLUP);    // Set button2 pin with pullup

    // Initialize and run I2C scanner
    scanner.Init();
    scanner.Scan();

    socketClient.init(&config);
    socketClient.initWebserver(80);
    mqttClient = new HAMqtt(&mqttCfg);
    app = new App(config.name, &socketClient);
    app->setMqttClient(mqttClient);
    app->init();
    Serial.println("---Setup complete---");
}

void loop()
{
    socketClient.loop();
    if (mqttClient) mqttClient->loop();
    app->loop();

    // Small delay to prevent overwhelming the CPU
    delay(10);
}
