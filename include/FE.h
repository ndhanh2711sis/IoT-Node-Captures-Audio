#pragma once
#include <Arduino.h>

struct FETensor {
  float* data;
  int    nMel;
  int    nFrames;
};

namespace FE {
  // Tạo đặc trưng (stub, để pipeline chạy)
  bool computeLogMel(const int16_t* pcm, size_t samples, int sr, FETensor& out);
  // Giải phóng bộ nhớ tensor
  void free(FETensor& t);
}
