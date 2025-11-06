#include "FE.h"
#include <stdlib.h>
#include <math.h>

bool FE::computeLogMel(const int16_t* pcm, size_t samples, int sr, FETensor& out) {
  (void)sr; // chưa dùng trong stub
  if (!pcm || samples == 0) return false;

  out.nMel    = 40;
  out.nFrames = 32;
  const size_t N = (size_t)out.nMel * out.nFrames;

  out.data = (float*)ps_malloc(N * sizeof(float));
  if (!out.data) return false;

  // RMS thô → đổ đều vào tensor để thông đường
  double acc = 0.0;
  for (size_t i = 0; i < samples; ++i) {
    float v = (float)pcm[i] / 32768.0f;
    acc += (double)v * (double)v;
  }
  float rms = sqrtf((float)(acc / (samples ? samples : 1)));

  for (size_t i = 0; i < N; ++i) out.data[i] = rms;
  return true;
}

void FE::free(FETensor& t) {
  if (t.data) { ::free(t.data); t.data = nullptr; }
  t.nMel = 0; t.nFrames = 0;
}
