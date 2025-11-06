#pragma once
#include <Arduino.h>

struct AudioClip {
  int16_t* pcm;
  size_t   samples;
  uint32_t sr;
  uint64_t tsStartMs;
  uint64_t tsEndMs;
};

typedef void (*ClipCB)(const AudioClip&);

namespace VADSeg {
  bool begin(uint32_t sr, size_t preMs, size_t postMs, ClipCB cb);
  // gọi cho mỗi chunk từ AudioIn
  void onChunk(const int16_t* data, size_t samples, uint64_t nowMs);
  // tùy chọn: chỉnh ngưỡng
  void setThresholds(float minLevel, uint32_t minActMs, uint32_t minSilMs);
}
