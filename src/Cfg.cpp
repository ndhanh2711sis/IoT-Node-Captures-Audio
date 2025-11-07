#include "Cfg.h"

namespace {
  // Giá trị mặc định để chạy được ngay (mock)
  constexpr const char* kWifiSsid    = "Quang Hai T3";
  constexpr const char* kWifiPass    = "19741975";

  constexpr const char* kDeviceId    = "S3-01";
  constexpr const char* kModelVer    = "v0.1.0";
  constexpr const char* kModelPath   = "/model.tflite";

  constexpr int         kSrHz        = 16000;
  constexpr int         kPreMs       = 2000;
  constexpr int         kPostMs      = 3000;

  constexpr const char* kMqttHost    = "192.168.88.113";
  constexpr uint16_t    kMqttPort    = 1883;
  constexpr bool        kMqttTLS     = false;
  constexpr const char* kMqttUser    = "";
  constexpr const char* kMqttPass    = "";
  constexpr const char* kTopicEvents = "factory/line1/sound/events";
  constexpr const char* kTopicAudio  = "factory/line1/sound/events/audio";
}

void        Cfg::load()                 { /* TODO: đọc từ LittleFS/NVS khi cần */ }
const char* Cfg::wifiSsid()             { return kWifiSsid; }
const char* Cfg::wifiPass()             { return kWifiPass; }

const char* Cfg::deviceId()             { return kDeviceId; }
const char* Cfg::modelVer()             { return kModelVer; }
const char* Cfg::modelPath()            { return kModelPath; }

int         Cfg::srHz()                 { return kSrHz; }
int         Cfg::preMs()                { return kPreMs; }
int         Cfg::postMs()               { return kPostMs; }

const char* Cfg::mqttHost()             { return kMqttHost; }
uint16_t    Cfg::mqttPort()             { return kMqttPort; }
bool        Cfg::mqttTLS()              { return kMqttTLS; }
const char* Cfg::mqttUser()             { return kMqttUser; }
const char* Cfg::mqttPass()             { return kMqttPass; }
const char* Cfg::topicEvents()          { return kTopicEvents; }
const char* Cfg::topicAudio()           { return kTopicAudio; }
