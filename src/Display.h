#ifndef DISPLAY_H_
#define DISPLAY_H_

#ifdef DISPLAY_WAVESHARE
  #include <Waveshare_LCD1602.h>
#else
  #include <LiquidCrystal_I2C.h>
#endif

class Display {
  public:
    Display(uint8_t address);
    void begin();
    void clear();
    void clearRow(int row);
    void print(const float message);
    void print(const String &message);
    void print(const String &message, int row);
    void print(const String &message, int row, int col);

    void setCursor(uint8_t col, uint8_t row);
    void backlightOff();
    void backlightOn();
    void setBrightness(uint8_t brightness);
    void setRGB(uint8_t r, uint8_t g, uint8_t b);

  private:
#ifdef DISPLAY_WAVESHARE
    Waveshare_LCD1602 lcd;
#else
    LiquidCrystal_I2C lcd;
#endif
};

#endif // DISPLAY_H_
