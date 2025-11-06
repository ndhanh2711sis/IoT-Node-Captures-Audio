#include "AI.h"
#include <Arduino.h>

// Stub: tính trung bình tuyệt đối của tensor làm "độ bất thường"
float AI::inferScore(const FETensor& x) {
  if (!x.data || x.nMel <= 0 || x.nFrames <= 0) return 0.0f;
  size_t N = (size_t)x.nMel * (size_t)x.nFrames;
  double acc = 0.0;
  for (size_t i = 0; i < N; ++i) {
    acc += fabs(x.data[i]);
  }
  double mean = acc / (double)N;

  // Chuẩn hóa thô về 0..1
  if (mean < 0) mean = 0;
  if (mean > 8.0) mean = 8.0;     // clamp để tránh bùng
  float score = (float)(mean / 8.0);
  if (score < 0) score = 0;
  if (score > 1) score = 1;
  return score;
}
