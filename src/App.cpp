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

  // MQTT
  MqttPub::begin(Cfg::mqttHost(), Cfg::mqttPort(), Cfg::mqttTLS(),
                 Cfg::mqttUser(), Cfg::mqttPass());

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
