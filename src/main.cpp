#include "Display.h"
#include "I2CScanner.h"
#include "OneButton.h"
#include "globals.h"
#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <SocketClient.h>
#include <Wire.h>

#define SEALEVELPRESSURE_HPA (1013.25)

OneButton button(D6, true);

#define VERSION 1.0

#ifdef LED_BUILTIN
#define LED_PIN LED_BUILTIN
#else
#define LED_PIN 2
#endif

Adafruit_BMP280 bmp; // I2C
TwoWire         wire2;

I2CScanner   scanner;
SocketClient testClient;
Display      display(0x27); // Assuming the LCD I2C address is 0x27
int          counter = 0;

void sendStatus(JsonDoc status)
{
    status["counter"] = counter;
}

void receivedCommand(JsonDoc doc)
{
    if (strcmp(doc["data"], "counterInc") == 0) {
        counter++;
    } else if (strcmp(doc["data"], "counterDec") == 0) {
        counter--;
    }
    testClient.sendStatusWithSocket();
}

void entityChanged(JsonDoc doc)
{
    if (strcmp(doc["entity"], "counter") == 0) {
        counter = doc["value"].as<int>();
    }
    testClient.sendStatusWithSocket();
}

void connected(JsonDoc doc)
{
    Serial.print("Connected data:  ");
    serializeJson(doc, Serial);
    Serial.println();
    // do something with the connected data

    testClient.sendNotification("Connected!");
    testClient.sendStatusWithSocket(true);
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


// this function will be called when the button started long pressed.
void LongPressStart(void *oneButton)
{
  Serial.print(((OneButton *)oneButton)->getPressedMs());
  Serial.println("\t - LongPressStart()");
}

// this function will be called when the button is released.
void LongPressStop(void *oneButton)
{
  Serial.print(((OneButton *)oneButton)->getPressedMs());
  Serial.println("\t - LongPressStop()\n");
}

// this function will be called when the button is held down.
void DuringLongPress(void *oneButton)
{
  Serial.print(((OneButton *)oneButton)->getPressedMs());
  Serial.println("\t - DuringLongPress()");
}

void printValues()
{
    Serial.print("Temperature = ");
    Serial.print(bmp.readTemperature());
    Serial.println(" °C");

    Serial.print("Pressure = ");
    Serial.print(bmp.readPressure() / 100.0F);
    Serial.println(" hPa");

    Serial.print("Approx. Altitude = ");
    Serial.print(bmp.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(" m");

    Serial.println();
}

void setup()
{
    Serial.begin(115200);
    while (!Serial) {
    };

    scanner.Init();
    scanner.Scan();

    display.begin();
    display.clear();
    display.print(config.name);
    display.setCursor(0, 1);
    display.print("Version: ");
    display.print(VERSION);

    testClient.init(&config);


    // BMP280 setup
    // Serial.println(F("BMP280 test"));

    // unsigned status;
    // status = bmp.begin();
    // if (!status) {
    //     Serial.println("Could not find a valid BMP280 sensor, check wiring, address!");
    //     while (1) delay(10);
    // }

    // Serial.println("-- Default Test --");
    // printValues();

    // link functions to be called on events.
    button.attachLongPressStart(LongPressStart, &button);
    button.attachDuringLongPress(DuringLongPress, &button);
    button.attachLongPressStop(LongPressStop, &button);

    button.setDebounceMs(50);
    button.setClickMs(200);
    
    button.attachClick([](void *oneButton) {
        Serial.print(((OneButton *)oneButton)->getPressedMs());
        Serial.println("\t - Click()");
        counter++;
        testClient.sendStatusWithSocket();
    }, &button);

    button.attachDoubleClick([](void *oneButton) {
        Serial.print(((OneButton *)oneButton)->getPressedMs());
        Serial.println("\t - DoubleClick()");
        counter += 2;
        testClient.sendStatusWithSocket();
    }, &button);

    button.setLongPressIntervalMs(1000);
}

void loop()
{
    testClient.loop();
    button.tick();
}
