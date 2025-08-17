#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <LiquidCrystal_I2C.h>


class Display {
  public:
    Display(uint8_t address);
    LiquidCrystal_I2C lcd;
    void begin();
    void clear();
    void clearRow(int row);
    void print(const float message);
    void print(const String &message);
    void print(const String &message, int row);
    void print(const String &message, int row, int col);

    void setCursor(uint8_t col, uint8_t row);
    void setBrightness(uint8_t brightness);
};

#endif // DISPLAY_H_
