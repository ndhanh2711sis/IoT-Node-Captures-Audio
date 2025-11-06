#pragma once
#include <Arduino.h>

namespace Storage {
  bool writeWav(const char* path, const int16_t* pcm, size_t samples, int sr);
  String buildMetaJson(const char* deviceId,
                       const char* tsStartISO,
                       const char* tsEndISO,
                       int sr, float score, const char* modelVer);
}
