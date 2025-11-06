#pragma once
#include <Arduino.h>

namespace MqttPub {
  bool begin(const char* host, uint16_t port, bool tls,
             const char* user, const char* pass);
  bool publishMeta(const char* topic, const char* json);
  // Đọc file và publish theo chunk. Trả về true nếu publish hết.
  bool publishFileChunked(const char* topic, const char* path, size_t chunk = 4096);
  void loop();
}
