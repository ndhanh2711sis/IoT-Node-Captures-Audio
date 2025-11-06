#include "Storage.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

#pragma pack(push,1)
struct WAVHeader {
  char riff[4] = {'R','I','F','F'};
  uint32_t chunkSize;
  char wave[4] = {'W','A','V','E'};
  char fmt[4]  = {'f','m','t',' '};
  uint32_t subchunk1Size = 16;
  uint16_t audioFormat   = 1;   // PCM
  uint16_t numChannels   = 1;   // mono
  uint32_t sampleRate;
  uint32_t byteRate;
  uint16_t blockAlign;
  uint16_t bitsPerSample = 16;
  char data[4] = {'d','a','t','a'};
  uint32_t subchunk2Size;
};
#pragma pack(pop)

bool Storage::writeWav(const char* path, const int16_t* pcm, size_t samples, int sr) {
  File f = LittleFS.open(path, "wb");
  if (!f) return false;
  WAVHeader h;
  h.sampleRate = sr;
  h.byteRate = sr * h.numChannels * (h.bitsPerSample/8);
  h.blockAlign = h.numChannels * (h.bitsPerSample/8);
  h.subchunk2Size = samples * (h.bitsPerSample/8);
  h.chunkSize = 36 + h.subchunk2Size;

  if (f.write((const uint8_t*)&h, sizeof(h)) != sizeof(h)) { f.close(); return false; }
  size_t bytes = samples * sizeof(int16_t);
  size_t wrote = f.write((const uint8_t*)pcm, bytes);
  f.close();
  return wrote == bytes;
}

String Storage::buildMetaJson(const char* deviceId,
                              const char* tsStartISO,
                              const char* tsEndISO,
                              int sr, float score, const char* modelVer) {
  JsonDocument doc;                // ← thay vì StaticJsonDocument<512>
  doc["device_id"]      = deviceId;
  doc["timestamp_start"]= tsStartISO;
  doc["timestamp_end"]  = tsEndISO;
  doc["sr_hz"]          = sr;
  doc["score"]          = score;
  doc["model_ver"]      = modelVer;
  String out;
  serializeJson(doc, out);
  return out;
}