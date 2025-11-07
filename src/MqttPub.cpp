#include "MqttPub.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <LittleFS.h>
#include "Cfg.h"
#include "Memory.h"
static WiFiClient* netClient = nullptr;
static PubSubClient* mqtt = nullptr;

static String g_host;
static uint16_t g_port = 1883;
static bool g_tls = false;
static String g_user, g_pass;
static String clientId;

static uint32_t lastReconnect = 0;

// MqttPub.cpp
static bool ensureConnected() {
  if (!mqtt) return false;
  if (mqtt->connected()) return true;

  uint32_t now = millis();
  // backoff 2s cho các lần thử sau, nhưng cho phép thử ngay lần đầu
  if (lastReconnect != 0 && (now - lastReconnect < 2000)) {
    return false;
  }
  lastReconnect = now;

  // tạo clientId và thử connect
  String clientId = "esp32s3-" + String((uint32_t)ESP.getEfuseMac(), HEX);
  bool ok = false;
  if (g_user.length()) {
    ok = mqtt->connect(clientId.c_str(), g_user.c_str(), g_pass.c_str());
  } else {
    ok = mqtt->connect(clientId.c_str());
  }

  // Log chỉ dùng biến trong scope này
  #ifdef CORE_DEBUG_LEVEL
  if (ok) {
    Serial.printf("[MQTT] Connected as %s\n", clientId.c_str());
  } else {
    Serial.printf("[MQTT] Connect failed (server=%s:%u)\n",
                  g_host.c_str(), g_port);
  }
  #endif
  return ok;
}


bool MqttPub::begin(const char* host, uint16_t port, bool tls,
                    const char* user, const char* pass) {
  g_host = host; g_port = port; g_tls = tls;
  g_user = user ? user : ""; g_pass = pass ? pass : "";

  if (mqtt) { delete mqtt; mqtt = nullptr; }
  if (netClient) { delete netClient; netClient = nullptr; }

  if (tls) {
    auto *c = new WiFiClientSecure();
    c->setInsecure();
    netClient = c;
  } else {
    netClient = new WiFiClient();
  }

  mqtt = new PubSubClient(*netClient);
  mqtt->setServer(g_host.c_str(), g_port);
  mqtt->setKeepAlive(20);
  mqtt->setSocketTimeout(5);

  // thử kết nối ngay lập tức
  ensureConnected();
  return true;
}


bool MqttPub::publishMeta(const char* topic, const char* json) {
  if (!ensureConnected()) return false;
  // QoS 1 không có trực tiếp trong PubSubClient; dùng retain=false, dup do client
  return mqtt->publish(topic, json, false);
}

bool MqttPub::publishFileChunked(const char* topic, const char* path, size_t chunk) {
  if (!ensureConnected()) return false;
  File f = LittleFS.open(path, "rb");
  if (!f) return false;

  // Gửi từng mảnh: topic/filename + payload = raw bytes
  // Nếu broker cần metadata chunk index -> có thể bọc JSON/CBOR; ở đây gửi thẳng bytes WAV
  uint8_t* buf = (uint8_t*)audio_alloc(chunk);
  if (!buf) { f.close(); return false; }

  bool ok = true;
  while (ok && f.available()) {
    size_t n = f.read(buf, chunk);
    if (n == 0) break;
    // Lưu ý: PubSubClient giới hạn payload ~256-1024B tùy config.
    // Nếu quá nhỏ, giảm chunk xuống 1024 hoặc 512.
    ok = mqtt->publish(topic, buf, n, false);
    mqtt->loop(); // đẩy socket
    if (!ok) break;
    delay(2);
  }
  free(buf);
  f.close();
  return ok;
}

void MqttPub::loop() {
  if (mqtt) mqtt->loop();
  ensureConnected();
}
