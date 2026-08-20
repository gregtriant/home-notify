#ifndef MQTT_CONFIG_H_
#define MQTT_CONFIG_H_

// -----------------------------------------------------------------------------
// EXAMPLE FILE — copy this to src/MQTTConfig.h and fill in your own values.
// src/MQTTConfig.h is gitignored so local credentials stay out of the repo.
//
//   cp src/MQTTConfig.example.h src/MQTTConfig.h
// -----------------------------------------------------------------------------

// MQTT Broker Configuration
// Change these settings to match your MQTT broker setup

#define MQTT_SERVER      "192.168.0.100"   // IP address of your MQTT broker
#define MQTT_PORT        1883              // MQTT port (usually 1883)
#define MQTT_USERNAME    ""                // Leave empty for no authentication
#define MQTT_PASSWORD    ""                // Leave empty for no authentication
#define DEVICE_NAME      "home-notify"     // Device name for Home Assistant

// Advanced Settings (usually don't need to change)
#define MQTT_RECONNECT_INTERVAL  5000     // Milliseconds between reconnection attempts
#define MQTT_PUBLISH_QOS         0        // MQTT Quality of Service (0, 1, or 2)
#define MQTT_RETAIN             true      // Retain MQTT messages

#endif // MQTT_CONFIG_H_
