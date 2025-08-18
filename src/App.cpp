
#include "App.h"

App::App(String appName, SocketClient *sc) : display(LCD_ADDRESS), button(BUTTON_PIN)
{
    display.begin();
    display.clear();
    display.print(appName);
    display.setCursor(0, 1);
    display.print("Version: ");
    display.print(VERSION);
}


void App::setMessage(String message) {
    this->message = message;
    display.clearRow(0);
    display.print(message, 0);
}


void App::begin()
{
    // LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // turn off

    // Button
    button.setDebounceMs(50);
    button.setClickMs(200);
    button.setLongPressIntervalMs(1000);

    button.attachClick(&App::handleClick, this);
    button.attachDoubleClick(&App::handleDoubleClick, this);
    button.attachLongPressStart(&App::handleLongPressStart, this);
    button.attachDuringLongPress(&App::handleDuringLongPress, this);
    button.attachLongPressStop(&App::handleLongPressStop, this);
}

void App::loop()
{
    button.tick();
}


// ------------------------- Button functions ----------------------- // 
void App::handleClick(void *parameter)
{
    App *appInstance = static_cast<App *>(parameter);
    Serial.println("Click() from static method");
}

void App::handleDoubleClick(void *parameter)
{
    App *appInstance = static_cast<App *>(parameter);
    Serial.println("DoubleClick() from static method");
}

void App::handleLongPressStart(void *parameter)
{
    App *appInstance = static_cast<App *>(parameter);
    Serial.println("LongPressStart() from static method");
}

void App::handleDuringLongPress(void *parameter)
{
    App *appInstance = static_cast<App *>(parameter);
    Serial.println("DuringLongPress() from static method");
}

void App::handleLongPressStop(void *parameter)
{
    App *appInstance = static_cast<App *>(parameter);
    Serial.println("LongPressStop() from static method");
}
