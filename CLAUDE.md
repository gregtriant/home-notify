# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Flash Commands

```bash
# Build
pio run

# Build and upload
pio run --target upload

# Monitor serial output
pio device monitor

# Build, upload, and monitor
pio run --target upload && pio device monitor

# Clean build
pio run --target clean
```

There are no automated tests — this is embedded firmware verified by flashing and observing serial output at 115200 baud.

## Architecture

The firmware has two parallel subsystems that run in the same `loop()`:

1. **SocketClient** (`../SocketClient` — symlinked sibling directory, not in this repo) — handles WiFi, SSL WebSocket connection to `api.sensordata.space:443`, NTP time sync, and the command/status/entity protocol. The token in `src/globals.h` authenticates this connection.

2. **App class** (`src/App.cpp`) — owns all hardware: LCD display, 3 buttons, LED, and the MQTT client. `App::loop()` ticks buttons, updates timers, handles MQTT, and refreshes the clock line on the LCD every second.

**Message flow:** Remote commands arrive via `SocketClient` → `receivedCommand()` in `main.cpp` → `app->recievedMessage()` → shown on LCD row 0 + LED blinks. Button1/Button2 clicks go the other direction: hardware → MQTT publish → Home Assistant.

**MQTT / Home Assistant:** `App` owns a `HAMqtt` instance (`src/HAMqtt.h/.cpp`) which connects to the broker defined in `src/MQTTConfig.h`, publishes Home Assistant autodiscovery configs on first connect, and exposes `publishEvent(entityId, action)`. Button events publish to `homeassistant/device/home_notify/<button>/state`. Reconnect is retried every `MQTT_RECONNECT_INTERVAL` ms.

## Key Files

| File | Purpose |
|------|---------|
| `src/Definitions.h` | Pin assignments and VERSION |
| `src/globals.h` | Auth token for SocketClient (not committed to shared history — keep out of logs) |
| `src/MQTTConfig.h` | MQTT broker IP/port/credentials — **untracked**, create locally |
| `src/Display.h/.cpp` | Dual-display abstraction: LiquidCrystal_I2C (default) or Waveshare LCD1602 (I2C addr `0x3E`, backlight via SN3193 at `0x6B`), selected at compile time via `-D DISPLAY_WAVESHARE` in `platformio.ini` |
| `src/HAMqtt.h/.cpp` | MQTT + Home Assistant autodiscovery; manages entity registration and event publishing |
| `lib/Waveshare_LCD1602/` | Local Waveshare LCD1602 driver (not from registry) — handles both LCD and SN3193 backlight |

## Local Config Files

`src/MQTTConfig.h` is gitignored/untracked and must be created manually. Template:

```cpp
#ifndef MQTT_CONFIG_H_
#define MQTT_CONFIG_H_
#define MQTT_SERVER   "192.168.x.x"
#define MQTT_PORT     1883
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""
#define DEVICE_NAME   "home-notify"
#define MQTT_RECONNECT_INTERVAL 5000
#define MQTT_PUBLISH_QOS 0
#define MQTT_RETAIN true
#endif
```

## Hardware Notes

- **LCD row 0** — notification messages; **row 1** — clock (`WWW DD-MM  HH:MM`)
- LED is active-low (GPIO 2); `LOW` = on, `HIGH` = off
- BUTTON_PIN (GPIO 0) is the boot button — hold behavior may interfere with flashing
- BME280 sensor code is fully commented out; I2CScanner runs at startup to detect connected devices
- SocketClient is a local symlink (`lib` or sibling dir); if the build can't find it, check `platformio.ini` symlink path

## Display Driver Selection

Two LCD drivers are supported, selected by a build flag in `platformio.ini`:

```ini
; Waveshare LCD1602 (lib/Waveshare_LCD1602):
build_flags = -D DISPLAY_WAVESHARE

; LiquidCrystal_I2C (default — comment out or leave empty):
; build_flags =
```

- `DISPLAY_WAVESHARE` is the active configuration — hardware confirmed as Waveshare LCD1602 (LCD at `0x3E`, SN3193 backlight at `0x6B`)
- `LCD_ADDRESS` in `Definitions.h` is guarded by `#ifndef DISPLAY_WAVESHARE` and only applies to LiquidCrystal_I2C builds
- Brightness defaults: 50 (on/init), 2 (dimmed/off); `Display::setBrightness()` maps 0–255 → SN3193's 0–100 scale
- `Display::setRGB()` is a no-op on both drivers
