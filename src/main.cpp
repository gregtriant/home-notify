#include <Adafruit_BME280.h>
#include <Arduino.h>
#include <SocketClient.h>
#include <Wire.h>
#include <time.h>

#include "App.h"
#include "Definitions.h"
#include "Display.h"
#include "I2CScanner.h"
#include "OneButton.h"
#include "globals.h"


I2CScanner scanner;
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
    app->setDefaultMessage(defaultMessage);

    socketClient.sendNotification("Connected!");
    socketClient.sendStatusWithSocket(true);
}

/**
 * Configuration for the SocketClient
 */
SocketClientConfig_t config = {
    .name = "Home-Notify",
    .version = VERSION,
    .type = "ESP8266",
    .ledPin = LED_PIN,
    .host = "insecure2.sensordata.space", //"api.sensordata.space",
    .port = 80,
    .isSSL = false,
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
    while (!Serial) {
    };

    scanner.Init();
    scanner.Scan();

    socketClient.init(&config);

    app = new App(config.name, &socketClient);
    app->init();

    Serial.println("ESP8266 Memory Info");

    // Basic free heap info
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Max allocatable block: %d bytes\n", ESP.getMaxFreeBlockSize());
    Serial.printf("Heap fragmentation: %d %%\n", ESP.getHeapFragmentation());

    // Flash / sketch size info
    Serial.printf("Flash chip size: %d bytes\n", ESP.getFlashChipRealSize());
    Serial.printf("Sketch size: %d bytes\n", ESP.getSketchSize());
    Serial.printf("Free sketch space: %d bytes\n", ESP.getFreeSketchSpace());
}

void loop()
{
    socketClient.loop();
    app->loop();

    // Print heap status periodically
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 5000) {
        lastPrint = millis();
        Serial.printf("\n[Heap check] Free: %d, Max block: %d, Frag: %d%%\n", ESP.getFreeHeap(), ESP.getMaxFreeBlockSize(), ESP.getHeapFragmentation());
    
        char buf[25];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", app->getLocalTm());
        Serial.println(buf);
    }

}
