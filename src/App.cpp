#include "App.h"
#include <WiFi.h>

App::App(String appName, SocketClient *sc)
    : display(LCD_ADDRESS),
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

    // Don't start timers in constructor - do it in init() instead
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
    // GPIO pins are already configured in main setup()
    
    // Button configuration
    button.setDebounceMs(50);
    button.setClickMs(200);
    button.setLongPressIntervalMs(1000);

    button.attachClick(&App::handleClick, this);
    button.attachDoubleClick(&App::handleDoubleClick, this);
    button.attachLongPressStart(&App::handleLongPressStart, this);
    button.attachDuringLongPress(&App::handleDuringLongPress, this);
    button.attachLongPressStop(&App::handleLongPressStop, this);
    

    button1.setDebounceMs(50);
    button1.setClickMs(200);
    button1.setLongPressIntervalMs(1000);

    button2.setDebounceMs(50);
    button2.setClickMs(200);
    button2.setLongPressIntervalMs(1000);

    button1.attachClick(&App::handleClick1, this);

    button2.attachClick(&App::handleClick2, this);

    if (_mqtt) {
        _mqtt->addEntity({"button1", "Home Notify Button 1"});
        _mqtt->addEntity({"button2", "Home Notify Button 2"});
    }

    // Start timers after everything is properly initialized
    displayTimer.start(); // Start the LCD timer to turn off backlight after some time.
    
    Serial.println("--- app init ok ---");
}

const char* App::getWDayStr() 
{
    time_t epochTime = time(nullptr);
    struct tm *timeinfo = localtime(&epochTime);
    int wday = timeinfo->tm_wday;
    switch (wday) {
        case 0: return "Sun";
        case 1: return "Mon";
        case 2: return "Tue";
        case 3: return "Wed";
        case 4: return "Thu";
        case 5: return "Fri";
        case 6: return "Sat";
        default: return "???";
    }
}

void App::loop()
{
    button.tick();
    button1.tick();
    button2.tick();
    ledTimer.update();
    displayTimer.update();

    // Time related things.
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 1000) {
        lastUpdate = millis();
        // Commented out time display code since display is not used
        if (sc->hasTime()) {
            int h, m, s;
            sc->getTime(h, m, s);

            int day, month, year;
            sc->getDate(year, month, day);

            char buf[25];
            snprintf(buf, sizeof(buf), "%s %02d-%02d  %02d:%02d", getWDayStr(), day, month, h, m);
            display.print(buf, 1);
            // Serial.printf("Time: %s\n", buf);
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


void App::handleClick1(void *parameter)
{
    App *self = static_cast<App *>(parameter);
    self->publishButtonEvent("button1", "click");
}


void App::handleClick2(void *parameter)
{
    App *self = static_cast<App *>(parameter);
    self->publishButtonEvent("button2", "click");
}

