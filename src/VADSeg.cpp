#include "VADSeg.h"

static uint32_t g_sr;
static size_t   g_preMs, g_postMs;
static ClipCB   g_cb = nullptr;

// Ring buffer ~10s mặc định (có thể thay đổi nếu muốn)
static int16_t* g_ring = nullptr;
static size_t   g_ringSamples = 0;
static size_t   g_head = 0;
static bool     g_inited = false;

enum State { IDLE=0, ACTIVE, POST };
static State g_state = IDLE;
static uint64_t g_evtStartMs = 0;
static uint64_t g_lastActiveMs = 0;

// VAD params
static float    g_minLevel = 0.02f; // RMS chuẩn hóa (0..1)
static uint32_t g_minActMs = 200;
static uint32_t g_minSilMs = 300;

static void ringInit(uint32_t sr, float seconds=10.0f) {
  g_ringSamples = (size_t)(sr * seconds);
  g_ring = (int16_t*)ps_malloc(g_ringSamples * sizeof(int16_t));
  g_head = 0;
}

static float computeRMS(const int16_t* x, size_t n) {
  // chuẩn hóa về [-1,1] theo 32768
  double acc = 0;
  for (size_t i=0;i<n;i++) {
    float v = x[i] / 32768.0f;
    acc += (double)v * (double)v;
  }
  acc /= (double)n;
  float rms = sqrt((float)acc);
  return rms;
}

static float computeZCR(const int16_t* x, size_t n) {
  int cnt = 0;
  int16_t prev = x[0];
  for (size_t i=1;i<n;i++) {
    if ((prev < 0 && x[i] >= 0) || (prev >= 0 && x[i] < 0)) cnt++;
    prev = x[i];
  }
  return (float)cnt / (float)n;
}

bool VADSeg::begin(uint32_t sr, size_t preMs, size_t postMs, ClipCB cb) {
  g_sr = sr; g_preMs = preMs; g_postMs = postMs; g_cb = cb;
  ringInit(sr, 10.0f);
  g_state = IDLE;
  g_evtStartMs = g_lastActiveMs = 0;
  g_inited = (g_ring != nullptr);
  return g_inited;
}

void VADSeg::setThresholds(float minLevel, uint32_t minActMs, uint32_t minSilMs) {
  g_minLevel = minLevel;
  g_minActMs = minActMs;
  g_minSilMs = minSilMs;
}

void VADSeg::onChunk(const int16_t* data, size_t samples, uint64_t nowMs) {
  if (!g_inited || !g_cb) return;

  // 1) Push chunk vào ring
  for (size_t i=0;i<samples;i++) {
    g_ring[g_head] = data[i];
    g_head = (g_head + 1) % g_ringSamples;
  }

  // 2) VAD đơn giản: RMS + ZCR
  float rms = computeRMS(data, samples);
  float zcr = computeZCR(data, samples);
  bool active = (rms >= g_minLevel) || (zcr > 0.10f); // ngưỡng ZCR phụ

  // 3) State machine
  static uint64_t activeAccum = 0;
  static uint64_t postAccum = 0;

  if (g_state == IDLE) {
    if (active) {
      activeAccum += (samples * 1000ULL) / g_sr;
      if (activeAccum >= g_minActMs) {
        g_state = ACTIVE;
        g_evtStartMs = nowMs - activeAccum;
      }
    } else {
      activeAccum = 0;
    }
  } else if (g_state == ACTIVE) {
    if (active) {
      g_lastActiveMs = nowMs;
      postAccum = 0;
    } else {
      // im lặng
      uint64_t silentMs = nowMs - g_lastActiveMs;
      if (silentMs >= g_minSilMs) {
        g_state = POST;
        postAccum = 0;
      }
    }
  } else if (g_state == POST) {
    postAccum += (samples * 1000ULL) / g_sr;
    if (postAccum >= g_postMs) {
      // tạo clip [pre + (từ g_evtStart) + post]
      size_t preSamp  = (g_preMs  * g_sr) / 1000;
      size_t postSamp = (g_postMs * g_sr) / 1000;
      // ước lượng total event length: từ g_evtStartMs đến nowMs
      uint64_t evtEndMs = nowMs;
      uint64_t evtDurMs = (evtEndMs > g_evtStartMs) ? (evtEndMs - g_evtStartMs) : 0;
      size_t evtSamp = (size_t)( (evtDurMs * g_sr) / 1000 );
      size_t totalSamp = preSamp + evtSamp + postSamp;
      if (totalSamp > g_ringSamples) totalSamp = g_ringSamples - 1;

      int16_t* clip = (int16_t*)ps_malloc(totalSamp * sizeof(int16_t));
      if (clip) {
        // sao chép từ ring: kết thúc ở g_head, lùi lại totalSamp
        size_t endIdx = (g_head + g_ringSamples - 1) % g_ringSamples;
        size_t startIdx = (endIdx + g_ringSamples + 1 - totalSamp) % g_ringSamples;
        for (size_t i=0;i<totalSamp;i++) {
          size_t idx = (startIdx + i) % g_ringSamples;
          clip[i] = g_ring[idx];
        }
        AudioClip ac { clip, totalSamp, g_sr, g_evtStartMs > g_preMs ? g_evtStartMs - g_preMs : g_evtStartMs, evtEndMs + g_postMs };
        g_cb(ac);
        free(clip);
      }
      // reset
      g_state = IDLE;
      activeAccum = 0;
      postAccum = 0;
      g_evtStartMs = 0;
      g_lastActiveMs = 0;
    }
  }
}
