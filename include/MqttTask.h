#pragma once
#include <Arduino.h>

bool MqttTask_init();
bool MqttTask_connected();
bool MqttTask_publishEvent(const char *jsonPayload);
bool MqttTask_publishAudioBinary(const uint8_t *data, size_t len, const char *topicSuffix);
void MqttTask_loop();
