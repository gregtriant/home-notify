#include "Display.h"
#include <LiquidCrystal_I2C.h>

Display::Display(uint8_t address): lcd(address, 16, 2) {   
}

void Display::begin() {
    lcd.begin(16, 2);
    lcd.backlight();
}

void Display::clear() {
    lcd.clear();
}

void Display::clearRow(int row) {
    if (row < 0 || row > 1) return; // Only two rows for a 16x2 LCD
    lcd.setCursor(0, row);
    for (int i = 0; i < 16; i++) {
        lcd.print(" ");
    }
}

void Display::print(const float message) {
    lcd.print(message);
}

void Display::print(const String &message) {
    lcd.print(message);
}

void Display::print(const String &message, int row) {
    if (row < 0 || row > 1) return; // Only two rows for a 16x2 LCD
    lcd.setCursor(0, row);
    lcd.print(message);
}

void Display::print(const String &message, int row, int col) {
    if (row < 0 || row > 1 || col < 0 || col > 15) return; // Valid range check
    lcd.setCursor(col, row);
    lcd.print(message);
}

void Display::setCursor(uint8_t col, uint8_t row) {
    if (row < 0 || row > 1 || col < 0 || col > 15) return; // Valid range check
    lcd.setCursor(col, row);
}

void Display::backlightOff() {
    lcd.noBacklight();
}

void Display::backlightOn() {
    lcd.backlight();
}

void Display::setBrightness(uint8_t brightness) {
    // Assuming brightness is a value between 0 and 255
    if (brightness > 255) brightness = 255;
    // The LiquidCrystal_I2C library does not support brightness control directly,
    // so this function is a placeholder for future enhancements.
    // You might need to use an external library or hardware for actual brightness control.
}

