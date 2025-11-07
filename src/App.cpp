#include <LittleFS.h>   // <- để dùng LittleFS.begin()
#include "AI.h"         // <- để gọi AI::init() và AI::inferScore()
#include "App.h"
#include <Arduino.h>
#include "AudioIn.h"
#include "VADSeg.h"
#include "FE.h"
#include "Storage.h"
#include "MqttPub.h"
#include "TimeSync.h"
#include "Cfg.h"
#include <WiFi.h>
#include <WiFiClient.h>

static void onClipReady(const AudioClip& clip) {
  // 1) FE
  FETensor X;
  if (!FE::computeLogMel(clip.pcm, clip.samples, clip.sr, X)) return;

  // 2) AI
  float score = AI::inferScore(X);
  FE::free(X);

  // 3) Metadata JSON
  char tsStart[40], tsEnd[40];
  TimeSync::toISO8601(clip.tsStartMs, tsStart, sizeof(tsStart));
  TimeSync::toISO8601(clip.tsEndMs,   tsEnd,   sizeof(tsEnd));
  String meta = Storage::buildMetaJson(
      Cfg::deviceId(), tsStart, tsEnd, clip.sr, score, Cfg::modelVer());

  // 4) Lưu WAV (debug) & publish
  String path = String("/evt_") + tsStart + ".wav"; path.replace(':','-');
  Storage::writeWav(path.c_str(), clip.pcm, clip.samples, clip.sr);

  MqttPub::publishMeta(Cfg::topicEvents(), meta.c_str());
  MqttPub::publishFileChunked(Cfg::topicAudio(), path.c_str(), 4096);
}
static void net_diag() {
  Serial.println("=== NET INFO ===");
  Serial.printf("WiFi.status = %d (3=WL_CONNECTED)\n", WiFi.status());
  Serial.printf("ESP IP: %s  GW: %s  DNS: %s  RSSI: %d dBm\n",
    WiFi.localIP().toString().c_str(),
    WiFi.gatewayIP().toString().c_str(),
    WiFi.dnsIP().toString().c_str(),
    WiFi.RSSI());
}

static bool tcp_probe(const char* host, uint16_t port, uint32_t timeoutMs=3000) {
  WiFiClient c;
  Serial.printf("TCP probe %s:%u ...\n", host, port);
  if (!c.connect(host, port, timeoutMs)) {
    Serial.printf("[FAIL] errno=%d (timeout/no route/refused)\n", errno);
    return false;
  }
  Serial.println("[OK] TCP connected");
  c.stop();
  return true;
}
App& App::instance() { static App app; return app; }
void App::setup() {
  Serial.begin(115200);
  delay(300);
  psramInit();
  LittleFS.begin(true);

  Cfg::load();
  TimeSync::begin(Cfg::wifiSsid(), Cfg::wifiPass());

  // AI init (M3: stub sẽ trả true)
  AI::init(Cfg::modelPath());
  //
  Serial.printf("MQTT target: %s:%u TLS=%d\n",
              Cfg::mqttHost(), Cfg::mqttPort(), Cfg::mqttTLS());
    net_diag();
    tcp_probe(Cfg::mqttHost(), Cfg::mqttPort());

  // MQTT
  MqttPub::begin(Cfg::mqttHost(), Cfg::mqttPort(), Cfg::mqttTLS(),
                 Cfg::mqttUser(), Cfg::mqttPass());
    unsigned long t0 = millis();
while (millis() - t0 < 1500) { // chờ ngắn cho CONNECT
  MqttPub::loop();   // rất quan trọng để PubSubClient xử lý socket
  delay(10);
}
// 1) Gửi meta "hello"
String hello = "{\"type\":\"hello\",\"msg\":\"esp32-s3 online\"}";
MqttPub::publishMeta(Cfg::topicEvents(), hello.c_str());

// 2) Nếu đã có file WAV debug, gửi thử theo chunk nhỏ
//    (PubSubClient thường giới hạn payload, 256–1024 B)
MqttPub::publishFileChunked(Cfg::topicAudio(), "/evt_last.wav", 512);

  // VAD + Segmenter
  VADSeg::begin(Cfg::srHz(), Cfg::preMs(), Cfg::postMs(), onClipReady);

  // Audio input
  AudioIn::begin(Cfg::srHz(), 20 /*ms*/, [](const AudioChunk& c){
    uint64_t nowMs = TimeSync::nowMs();
    VADSeg::onChunk(c.data, c.samples, nowMs);
  });
  Serial.println("App ready.");
}

void App::loop() {
  AudioIn::loop();   // mock đọc từ file / I2S đọc DMA
  MqttPub::loop();   // giữ kết nối, retry, backoff
  delay(1);
}
