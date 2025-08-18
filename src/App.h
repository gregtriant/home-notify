#pragma once
#include <Arduino.h>
#include <OneButton.h>
#include <SocketClient.h>

#include "Definitions.h"
#include "Display.h"

class App {
   private:
    String message;
    String datetime;

    uint8_t ledState = HIGH; // LOW is ON

    float temperature;
    float humidity;
    float pressure;

    // Adafruit_BME280 bme;
    Display display;
    OneButton button;
    SocketClient *socketClient;

   public:
    App(String appName, SocketClient *sc);

    void begin();
    void loop();

    void setMessage(String message);

    String getMessage() const
    {
        return message;
    }

    String getDatetime() const
    {
        return datetime;
    }

    void ledON()
    {
        ledState = LOW;
        digitalWrite(LED_PIN, ledState);
    }

    void ledOFF()
    {
        ledState = HIGH;
        digitalWrite(LED_PIN, ledState);
    }

    static void handleClick(void *parameter);
    static void handleDoubleClick(void *parameter);
    static void handleLongPressStart(void *parameter);
    static void handleDuringLongPress(void *parameter);
    static void handleLongPressStop(void *parameter);

    //   void printBME280Values() {
    //     Serial.print("Temperature = ");
    //     Serial.print(bme.readTemperature());
    //     Serial.println(" °C");

    //     Serial.print("Humidity = ");
    //     Serial.print(bme.readHumidity());
    //     Serial.println(" %");

    //     Serial.print("Pressure = ");
    //     Serial.print(bme.readPressure() * 0.01f);
    //     Serial.println(" hPa");

    //     Serial.println();
    //   }
};
