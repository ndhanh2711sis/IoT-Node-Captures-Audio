#pragma once
#include <Arduino.h>

namespace Cfg {
  // Nạp cấu hình (hiện trả về mặc định)
  void        load();

  // Wi-Fi
  const char* wifiSsid();
  const char* wifiPass();

  // Thiết bị / model
  const char* deviceId();
  const char* modelVer();
  const char* modelPath();

  // Audio / segment
  int         srHz();
  int         preMs();
  int         postMs();

  // MQTT
  const char* mqttHost();
  uint16_t    mqttPort();
  bool        mqttTLS();
  const char* mqttUser();
  const char* mqttPass();
  const char* topicEvents();
  const char* topicAudio();
}
