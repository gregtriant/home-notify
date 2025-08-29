#pragma once
#include <Arduino.h>
#include <OneButton.h>
#include <SocketClient.h>
#include <arduino-timer.h>
#include <Ticker.h>
#include <time.h>

#include "Definitions.h"
#include "Display.h"

class App {
   private:
    SocketClient *sc;

    String message;
    String defaultMessage; // Message to show on double click.
    String datetime;
    bool isUnreadMessage = false;
    uint8_t ledState = HIGH; // LOW is ON

    float temperature;
    float humidity;
    float pressure;

    // Adafruit_BME280 bme;
    Display display;
    OneButton button;
    Ticker ledTimer;         // Timer for LED blinking.
    Ticker displayTimer;     // Timer to turn off LCD after some time.
    SocketClient *socketClient;

    // Time related things
    time_t gLocalTime = 0;       // Timestamp with offset applied
    struct tm gLocalTm;          // Broken-down time
    bool gTimeSynced = false;    // Flag to indicate NTP synced

public:
    App(String appName, SocketClient *sc);

    void init();
    void loop();

    void recievedMessage(String message);

    void showMessage(String message) {
        display.clearRow(0);
        display.print(message, 0);
    }

    void setMessage(String message) {
        this->message = message;
    }

    void setDefaultMessage(String message)
    {
        this->defaultMessage = message;
    }

    String getMessage() const
    {
        return message;
    }

    tm *getLocalTm() const
    {
        return (tm *)&gLocalTm;
    }

    String getDatetime() const
    {
        return datetime;
    }

    bool toggleLed_cb(void *) {
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);
        return true; // Keep the timer running.
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
