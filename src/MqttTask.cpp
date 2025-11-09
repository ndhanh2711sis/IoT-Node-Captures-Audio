#ifndef NO_MQTT

#include "MqttTask.h"
#include <PubSubClient.h>
#include <WiFi.h>

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void MqttTask_init(const char* server, uint16_t port) {
  mqttClient.setServer(server, port);
  Serial.printf("[MQTT] Server set: %s:%d\n", server, port);
}

bool MqttTask_publish(const char* topic, const String &payload) {
  if (!mqttClient.connected()) {
    Serial.println("[MQTT] not connected");
    return false;
  }
  return mqttClient.publish(topic, payload.c_str());
}

void MqttTask_loop() {
  mqttClient.loop();
}

#endif  // NO_MQTT
