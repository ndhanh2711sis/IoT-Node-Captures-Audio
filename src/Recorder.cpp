#include "Recorder.h"
#include "Config.h"
#include <LittleFS.h>
#include <Arduino.h>

static const char *TAG = "Recorder";

bool Recorder_init() {
  if (!LittleFS.begin()) {
    Serial.println("[Recorder] LittleFS mount failed");
    return false;
  }
  Serial.println("[Recorder] LittleFS mounted");
  return true;
}

static bool writeWavHeader(File &f, uint32_t pcmBytes) {
  uint32_t sampleRate = AUDIO_SR;
  uint16_t channels = 1;
  uint16_t bitDepth = 16;
  uint32_t byteRate = sampleRate * channels * bitDepth / 8;
  uint16_t blockAlign = channels * bitDepth / 8;

  // RIFF
  f.write((const uint8_t*)"RIFF", 4);
  uint32_t chunkSize = 36 + pcmBytes;
  f.write((const uint8_t*)&chunkSize, 4);
  f.write((const uint8_t*)"WAVE", 4);

  // fmt subchunk
  f.write((const uint8_t*)"fmt ", 4);
  uint32_t subChunk1Size = 16;
  f.write((const uint8_t*)&subChunk1Size, 4);
  uint16_t audioFormat = 1; // PCM
  f.write((const uint8_t*)&audioFormat, 2);
  f.write((const uint8_t*)&channels, 2);
  f.write((const uint8_t*)&sampleRate, 4);
  f.write((const uint8_t*)&byteRate, 4);
  f.write((const uint8_t*)&blockAlign, 2);
  f.write((const uint8_t*)&bitDepth, 2);

  // data subchunk
  f.write((const uint8_t*)"data", 4);
  f.write((const uint8_t*)&pcmBytes, 4);
  return true;
}

bool Recorder_saveWavFromInt32(int32_t *buf32, size_t samples, const char *fname) {
  File f = LittleFS.open(fname, FILE_WRITE);
  if (!f) {
    Serial.printf("[Recorder] Open fail: %s\n", fname);
    return false;
  }
  uint32_t pcmBytes = samples * 2; // 16-bit mono
  if (!writeWavHeader(f, pcmBytes)) {
    f.close();
    return false;
  }
  for (size_t i = 0; i < samples; ++i) {
    int16_t s16 = (int16_t)(buf32[i] >> 8); // 24->16-ish
    f.write((const uint8_t*)&s16, 2);
  }
  f.close();
  Serial.printf("[Recorder] WAV saved: %s (%u bytes)\n", fname, pcmBytes + 44);
  return true;
}

bool Recorder_saveWavFromBuffers(int16_t *pcm16, size_t samples, const char *fname) {
  File f = LittleFS.open(fname, FILE_WRITE);
  if (!f) {
    Serial.printf("[Recorder] Open fail: %s\n", fname);
    return false;
  }
  uint32_t pcmBytes = samples * 2;
  if (!writeWavHeader(f, pcmBytes)) {
    f.close();
    return false;
  }
  f.write((const uint8_t*)pcm16, pcmBytes);
  f.close();
  Serial.printf("[Recorder] WAV saved: %s (%u bytes)\n", fname, pcmBytes + 44);
  return true;
}
