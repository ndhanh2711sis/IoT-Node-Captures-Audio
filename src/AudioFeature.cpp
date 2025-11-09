#include "AudioFeature.h"
#include "Config.h"
#include <math.h>

/*
 * AudioFeature:
 *  - ICS-43434: 24-bit left-justified in 32-bit -> shift right 8 to approx 24->16-ish
 */

float AudioFeature_computeRMS(int32_t *buf, size_t samples) {
  double acc = 0.0;
  for (size_t i = 0; i < samples; ++i) {
    int32_t s = buf[i] >> 8; // lấy 24-bit -> ~16 scale
    acc += (double)s * (double)s;
  }
  double rms = sqrt(acc / (double)samples);
  return (float)rms;
}

int32_t AudioFeature_computePeak(int32_t *buf, size_t samples) {
  int32_t peak = 0;
  for (size_t i = 0; i < samples; ++i) {
    int32_t s = buf[i] >> 8;
    int32_t a = s < 0 ? -s : s;
    if (a > peak) peak = a;
  }
  return peak;
}
