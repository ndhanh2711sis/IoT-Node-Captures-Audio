#pragma once
#include "FE.h"

namespace AI {
  // Khởi tạo mô hình (tạm thời chỉ giả lập)
  inline bool init(const char* /*modelPath*/) { return true; }

  // Trả điểm anomaly 0..1 từ đặc trưng (stub)
  float inferScore(const FETensor& x);
}
