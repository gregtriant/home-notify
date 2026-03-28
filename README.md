
# Home Notification Device

An ESP32-based device that displays remote notifications on a 16×2 LCD and publishes button events to Home Assistant via MQTT.

## Hardware

- **AZ-Delivery ESP32 DevKit V4**
- **Waveshare LCD1602** — 16×2 I2C display (`0x3E`) with SN3193 backlight controller (`0x6B`)
- **3 buttons** — boot button (GPIO 0) + two user buttons (GPIO 25, 26)
- **LED** — active-low on GPIO 2

## Features

- Receives and displays messages from [sensordata.space](https://api.sensordata.space)
- Shows date and time (row 1: `WWW DD-MM  HH:MM`)
- Publishes button events to Home Assistant via MQTT autodiscovery
- Backlight dims after 10 seconds of inactivity; brightens on new message

## Setup

1. Copy `src/MQTTConfig.h` from the template in `CLAUDE.md` and fill in your broker details
2. Copy `src/globals.h` with your sensordata.space auth token
3. Build and flash:

```bash
pio run --target upload && pio device monitor
```

## Dependencies

Managed via PlatformIO (`platformio.ini`). The `Waveshare_LCD1602` driver is bundled in `lib/`. The `SocketClient` library is expected as a symlink at `../SocketClient`.
