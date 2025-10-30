#include "App.h"

App::App(String appName, SocketClient *sc)
    : display(LCD_ADDRESS),
      button(BUTTON_PIN),
      ledTimer([this]() { this->toggleLed_cb(nullptr); }, 1*1000, 0, MILLIS),
      displayTimer([this]() { this->display.backlightOff(); }, 10*1000, 0, MILLIS)
{
    this->sc = sc;
    display.begin();
    display.clear();
    display.print(appName);
    display.setCursor(0, 1);
    display.print("Version: ");
    display.print(VERSION);

    displayTimer.start(); // Start the LCD timer to turn off backlight after some time.
}


void App::recievedMessage(String message) {
    display.backlightOn(); // Turn on backlight when a new message is received.
    displayTimer.start();  // Restart the LCD timer.

    setMessage(message);
    showMessage(message);
    // Start the led timer.
    ledTimer.start();
    sc->sendLog(message);
}


void App::init()
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
    ledTimer.update();
    displayTimer.update();

    // Time related things.
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 1000) {
        lastUpdate = millis();
        if (sc->hasTime()) {
            int h, m, s;
            sc->getTime(h, m, s);

            int day, month, year;
            sc->getDate(year, month, day);
            char buf[25];
            snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d", year, month, day, h, m);
            display.print(buf, 1);
            Serial.printf("Time: %s\n", buf);
        }
    }
}


// ------------------------- Button functions ----------------------- // 
void App::handleClick(void *parameter)
{
    App *self = static_cast<App *>(parameter);
    Serial.println("Click() from static method");
    self->ledTimer.stop();
    self->ledOFF();

    self->display.backlightOn(); // Turn on backlight when a new message is received.
    self->displayTimer.start();  // Restart the LCD timer.
}

void App::handleDoubleClick(void *parameter)
{
    App *self = static_cast<App *>(parameter);
    Serial.println("DoubleClick() from static method");
    self->recievedMessage(self->defaultMessage);
}

void App::handleLongPressStart(void *parameter)
{
    Serial.println("LongPressStart() from static method");
}

void App::handleDuringLongPress(void *parameter)
{
    Serial.println("DuringLongPress() from static method");
}

void App::handleLongPressStop(void *parameter)
{
    Serial.println("LongPressStop() from static method");
}
