#include <Arduino.h>
#include "Config.h"
#include "AudioTask.h"
#include "AudioFeature.h"
#include "Recorder.h"
#include "RTCUtil.h"
#include "MqttTask.h"
#include <vector>
#include <algorithm>

/*
 * main.cpp
 * - setup(): init Serial, LittleFS, RTC, Audio, MQTT
 * - loop(): đọc frame liên tục, push vào ring buffer (pre-roll),
 *           tính RMS, nếu vượt threshold và consensus -> trigger
 *           khi trigger: ghi wav (pre + post), gửi MQTT event + audio
 */

// Ring buffer (pre-roll) lưu int32_t samples
static int32_t *preRing = nullptr;
static size_t preRingSize = PREROLL_SAMPLES; // số mẫu
static size_t preWriteIndex = 0;
static bool preRingReady = false;

// dư chỗ cho post-roll temp buffer (lưu trực tiếp frame by frame)
static int32_t *postBuf = nullptr;
static size_t postCaptured = 0;
static size_t postNeeded = POSTROLL_SAMPLES;

static bool triggered = false;
static unsigned long triggerTimeMs = 0;

// baseline RMS tính trung bình trong vài giây đầu
static double baselineRms = 0.0;
static unsigned long baselineStartMs = 0;
static size_t baselineFrames = 0;

void pushPreRing(int32_t *frame, size_t frameSamples) {
  // frameSamples thường = AUDIO_FRAME_LEN
  for (size_t i = 0; i < frameSamples; ++i) {
    preRing[preWriteIndex++] = frame[i];
    if (preWriteIndex >= preRingSize) {
      preWriteIndex = 0;
      preRingReady = true;
    }
  }
}

void saveTriggeredClipAndPublish() {
  // Tạo tên file theo thời gian
  String iso = RTC_getISO8601();
  char fname[64];
  snprintf(fname, sizeof(fname), "/event_%s.wav", iso.c_str()); // ISO contains ':' but LittleFS may not like colon
  // replace ':' with '-' for filename
  for (char *p = fname; *p; ++p) if (*p == ':') *p = '-';

  // Build a contiguous buffer (pre-roll oldest -> newest)
  size_t totalSamples = preRingSize + postCaptured;
  int32_t *all32 = (int32_t*)malloc(sizeof(int32_t) * totalSamples);
  if (!all32) {
    Serial.println("[MAIN] malloc failed for all32");
    return;
  }
  // copy preRing starting from preWriteIndex (oldest)
  size_t idx = preWriteIndex;
  for (size_t i = 0; i < preRingSize; ++i) {
    all32[i] = preRing[idx++];
    if (idx >= preRingSize) idx = 0;
  }
  // append postBuf (captured post frames)
  for (size_t i = 0; i < postCaptured; ++i) {
    all32[preRingSize + i] = postBuf[i];
  }

  // Save WAV to LittleFS
  Recorder_saveWavFromInt32(all32, totalSamples, fname);

  // Publish metadata JSON via MQTT (timestamp + filename + rms)
  iso = RTC_getISO8601();
  char jsonBuf[256];
  snprintf(jsonBuf, sizeof(jsonBuf),
           "{\"device\":\"%s\",\"time\":\"%s\",\"file\":\"%s\",\"samples\":%u}",
           DEVICE_ID, iso.c_str(), fname, (unsigned)totalSamples);
  MqttTask_publishEvent(jsonBuf);

  // Publish audio binary on topic "devices/<id>/audio/<filename>"
  // We'll open file and publish in chunks
#ifndef NO_MQTT
  File f = LittleFS.open(fname, FILE_READ);
  if (f) {
    const size_t CHUNK = 1024;
    uint8_t chunkBuf[CHUNK];
    char topicSuffix[64];
    snprintf(topicSuffix, sizeof(topicSuffix), "%s", fname + 1); // skip leading '/'
    while (f.available()) {
      size_t r = f.read(chunkBuf, CHUNK);
      MqttTask_publishAudioBinary(chunkBuf, r, topicSuffix);
      delay(50); // polite pacing
    }
    f.close();
  } else {
    Serial.printf("[MAIN] cannot open file %s\n", fname);
  }
#else
  Serial.println("[MAIN] MQTT disabled, skip publish audio");
#endif

  free(all32);
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n[MAIN] Start IoT Audio Node");

  // init FS
  Recorder_init();

  // init RTC
  RTC_init();

  // init audio
  AudioTask_init();

  // init mqtt
  MqttTask_init();

  // allocate preRing & postBuf
  preRing = (int32_t*)malloc(sizeof(int32_t) * preRingSize);
  memset(preRing, 0, sizeof(int32_t) * preRingSize);
  postBuf = (int32_t*)malloc(sizeof(int32_t) * postNeeded);
  postCaptured = 0;

  baselineStartMs = millis();
  baselineFrames = 0;
}

void loop() {
  // Maintain MQTT loop
  MqttTask_loop();

  // Read one frame blocking
  static int32_t frame[AUDIO_FRAME_LEN];
  size_t samples = 0;
  if (!AudioTask_readFrame(frame, samples)) {
    delay(10);
    return;
  }
  // push into pre-roll circular buffer (for later saving)
  pushPreRing(frame, samples);

  // Update baseline RMS for first RMS_BASELINE_MS
  float rms = AudioFeature_computeRMS(frame, samples);
  if ((millis() - baselineStartMs) < RMS_BASELINE_MS) {
    baselineFrames++;
    baselineRms = ((baselineRms * (baselineFrames - 1)) + rms) / baselineFrames;
    Serial.printf("[MAIN] baseline collecting rms=%.1f (avg=%.1f)\n", rms, baselineRms);
    return; // not detecting until baseline ready
  }

  // Detection: if rms > baseline * factor, count consecutive frames
  static int consec = 0;
  if (rms > (baselineRms * DETECT_THRESHOLD_FACTOR)) {
    consec++;
  } else {
    consec = 0;
  }

  if (!triggered && consec >= DETECT_CONSENSUS_FRAMES) {
    // trigger start: begin capturing post-roll
    triggered = true;
    triggerTimeMs = millis();
    postCaptured = 0;
    Serial.printf("[MAIN] Triggered! rms=%.1f baseline=%.1f\n", rms, baselineRms);
  }

  if (triggered) {
    // append current frame to postBuf (we need postNeeded samples total)
    // we append sample-by-sample (postCaptured index)
    size_t canCopy = std::min((size_t)samples, postNeeded - postCaptured);
    // copy samples into postBuf starting at postCaptured
    // if postCaptured counts samples
    memcpy(postBuf + postCaptured, frame, canCopy * sizeof(int32_t));
    postCaptured += canCopy;
    // if collected enough post samples -> finalize
    if (postCaptured >= postNeeded) {
      // finalize: save and publish
      Serial.println("[MAIN] Post-roll complete, saving clip...");
      saveTriggeredClipAndPublish();
      // reset trigger
      triggered = false;
      consec = 0;
      // optionally, after trigger, reset baseline slowly (not implemented)
    }
  }

  // small yield
  delay(1);
}
