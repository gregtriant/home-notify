#include <Arduino.h>
#include "Display.h"

// ── Constructor ──────────────────────────────────────────────────────────────

#ifdef DISPLAY_WAVESHARE
Display::Display(uint8_t address) : lcd(16, 2) {
    // address ignored — Waveshare uses fixed I2C addresses (0x3E / 0x60)
}
#else
Display::Display(uint8_t address) : lcd(address, 16, 2) {
}
#endif

// ── Lifecycle ────────────────────────────────────────────────────────────────

void Display::begin() {
#ifdef DISPLAY_WAVESHARE
    lcd.init();
    lcd.set_brightness(50);
#else
    lcd.begin(16, 2);
    lcd.backlight();
#endif
}

void Display::clear() {
    lcd.clear();
}

// ── Cursor ───────────────────────────────────────────────────────────────────

void Display::setCursor(uint8_t col, uint8_t row) {
    if (row > 1 || col > 15) return;
    lcd.setCursor(col, row);
}

// ── Print ────────────────────────────────────────────────────────────────────

void Display::clearRow(int row) {
    if (row < 0 || row > 1) return;
    lcd.setCursor(0, row);
#ifdef DISPLAY_WAVESHARE
    lcd.send_string("                "); // 16 spaces
#else
    for (int i = 0; i < 16; i++) lcd.print(" ");
#endif
    lcd.setCursor(0, row);
}

void Display::print(const float message) {
#ifdef DISPLAY_WAVESHARE
    char buf[16];
    sprintf(buf, "%.2f", message);
    lcd.send_string(buf);
#else
    lcd.print(message);
#endif
}

void Display::print(const String &message) {
#ifdef DISPLAY_WAVESHARE
    lcd.send_string(message.c_str());
#else
    lcd.print(message);
#endif
}

void Display::print(const String &message, int row) {
    if (row < 0 || row > 1) return;
    lcd.setCursor(0, row);
#ifdef DISPLAY_WAVESHARE
    lcd.send_string(message.c_str());
#else
    lcd.print(message);
#endif
}

void Display::print(const String &message, int row, int col) {
    if (row < 0 || row > 1 || col < 0 || col > 15) return;
    lcd.setCursor(col, row);
#ifdef DISPLAY_WAVESHARE
    lcd.send_string(message.c_str());
#else
    lcd.print(message);
#endif
}

// ── Backlight ────────────────────────────────────────────────────────────────

void Display::backlightOn() {
#ifdef DISPLAY_WAVESHARE
    lcd.set_brightness(50);
#else
    lcd.backlight();
#endif
}

void Display::backlightOff() {
#ifdef DISPLAY_WAVESHARE
    lcd.set_brightness(2);
#else
    lcd.noBacklight();
#endif
}

void Display::setBrightness(uint8_t brightness) {
#ifdef DISPLAY_WAVESHARE
    // brightness is 0-255, Waveshare takes 0-100
    lcd.set_brightness(brightness * 100 / 255);
#endif
    // no-op for LiquidCrystal_I2C (no hardware brightness support)
}

void Display::setRGB(uint8_t r, uint8_t g, uint8_t b) {
    // no-op — Waveshare_LCD1602 has a single white backlight
}
