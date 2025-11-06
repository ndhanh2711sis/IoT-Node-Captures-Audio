#include "MqttPub.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <LittleFS.h>

static WiFiClient* netClient = nullptr;
static PubSubClient* mqtt = nullptr;

static String g_host;
static uint16_t g_port = 1883;
static bool g_tls = false;
static String g_user, g_pass;

static uint32_t lastReconnect = 0;

static bool ensureConnected() {
  if (!mqtt) return false;
  if (mqtt->connected()) return true;
  uint32_t now = millis();
  if (now - lastReconnect < 2000) return false;
  lastReconnect = now;

  String clientId = "esp32s3-" + String((uint32_t)ESP.getEfuseMac(), HEX);
  if (g_user.length()) {
    if (mqtt->connect(clientId.c_str(), g_user.c_str(), g_pass.c_str())) return true;
  } else {
    if (mqtt->connect(clientId.c_str())) return true;
  }
  return false;
}

bool MqttPub::begin(const char* host, uint16_t port, bool tls,
                    const char* user, const char* pass) {
  g_host = host; g_port = port; g_tls = tls;
  g_user = user ? user : ""; g_pass = pass ? pass : "";
  if (mqtt) { delete mqtt; mqtt = nullptr; }
  if (netClient) { delete netClient; netClient = nullptr; }

  if (tls) {
    auto *c = new WiFiClientSecure();
    c->setInsecure(); // TODO: thay bằng CA/cert thật nếu có
    netClient = c;
  } else {
    netClient = new WiFiClient();
  }

  mqtt = new PubSubClient(*netClient);
  mqtt->setServer(g_host.c_str(), g_port);
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
  uint8_t* buf = (uint8_t*)ps_malloc(chunk);
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
