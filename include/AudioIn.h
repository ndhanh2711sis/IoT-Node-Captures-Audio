#pragma once
#include <Arduino.h>

struct AudioChunk {
  int16_t* data;
  size_t   samples;
};

typedef void (*AudioChunkCB)(const AudioChunk&);

namespace AudioIn {
  // sr: 16000, chunkMs: 20 ms mặc định
  bool begin(uint32_t sr = 16000, size_t chunkMs = 20, AudioChunkCB cb = nullptr);
  // Gọi thường xuyên trong loop()
  void loop();
}
