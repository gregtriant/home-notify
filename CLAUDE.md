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

**MQTT / Home Assistant:** `App` connects to the local broker defined in `src/MQTTConfig.h` and publishes Home Assistant autodiscovery configs on first connect. Button events publish to `homeassistant/device/home_notify/<button>/state`. Connection is retried every 5 seconds inside `loopMQTT()`.

## Key Files

| File | Purpose |
|------|---------|
| `src/Definitions.h` | Pin assignments and VERSION |
| `src/globals.h` | Auth token for SocketClient (not committed to shared history — keep out of logs) |
| `src/MQTTConfig.h` | MQTT broker IP/port/credentials — **untracked**, create locally |
| `src/Display.h/.cpp` | Thin wrapper around LiquidCrystal_I2C (16×2, I2C addr `0x27`) |

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
