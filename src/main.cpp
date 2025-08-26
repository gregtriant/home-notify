#include <Arduino.h>
#include "I2CScanner.h"
#include "Display.h"
#include "OneButton.h"
#include "globals.h"
#include <SocketClient.h>

#include <Wire.h>
#include <Adafruit_BME280.h>

#include "Definitions.h"
#include "App.h"


I2CScanner   scanner;
SocketClient socketClient;
App *app;

void sendStatus(JsonDoc status)
{
    status["message"] = app->getMessage();
}

void receivedCommand(JsonDoc doc)
{
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

    } else if (command != app->getMessage()) { // New message recieved
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
    app->setDefaultMessage(defaultMessage);

    socketClient.sendNotification("Connected!");
    socketClient.sendStatusWithSocket(true);
}

/**
 * Configuration for the SocketClient
 */
SocketClientConfig config = {
    .name            = "Home-Notify",
    .version         = VERSION,
    .type            = "ESP8266",
    .ledPin          = LED_PIN,
    .host            = "api.sensordata.space",
    .port            = 443,
    .isSSL           = true,
    .token           = token, // from globals.h
    .handleWifi      = true,
    .sendStatus      = sendStatus,
    .receivedCommand = receivedCommand,
    .entityChanged   = entityChanged,
    .connected       = connected,
};


void setup()
{
    Serial.begin(115200);
    while (!Serial) {
    };

    scanner.Init();
    scanner.Scan();

    socketClient.init(&config);

    app = new App(config.name, &socketClient);
    app->init();
}

void loop()
{
    socketClient.loop();
    app->loop();
}
