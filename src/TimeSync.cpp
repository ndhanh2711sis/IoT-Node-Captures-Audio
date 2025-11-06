#include "TimeSync.h"
#include <WiFi.h>
#include <time.h>
#include <sys/time.h>

namespace {
  // epoch_ms = bootEpochMs + millis()
  int64_t g_bootEpochMs = 0;
  bool    g_synced = false;

  // chờ NTP tối đa vài giây, không khóa cứng
  bool waitForNtp(uint32_t timeoutMs = 8000) {
    uint32_t t0 = millis();
    time_t now = 0;
    do {
      time(&now);
      if (now > 1609459200) return true; // > 2021-01-01 là coi như có NTP
      delay(200);
    } while (millis() - t0 < timeoutMs);
    return false;
  }
}

void TimeSync::begin(const char* ssid, const char* pass) {
  // Wi-Fi STA nếu có cấu hình
  if (ssid && ssid[0]) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass ? pass : "");
    uint32_t t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < 10000) {
      delay(200);
    }
  }

  // NTP (UTC)
  configTime(0, 0, "pool.ntp.org", "time.google.com", "time.nist.gov");

  if (waitForNtp(8000)) {
    struct timeval tv{};
    gettimeofday(&tv, nullptr);
    int64_t epochMs = (int64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
    g_bootEpochMs = epochMs - (int64_t)millis();
    g_synced = true;
  } else {
    g_synced = false;
  }
}

uint64_t TimeSync::nowMs() {
  return millis();
}

bool TimeSync::isSynced() { return g_synced; }

void TimeSync::toISO8601(uint64_t bootMs, char* out, size_t outSize) {
  if (!out || outSize < 25) return; // "YYYY-MM-DDTHH:MM:SS.mmmZ" ≈ 24–25
  int64_t epochMs = g_synced ? (g_bootEpochMs + (int64_t)bootMs)
                             : (int64_t)bootMs; // fallback

  time_t sec = (time_t)(epochMs / 1000);
  int ms = (int)(epochMs % 1000);
  struct tm tmUTC{};
#if defined(_WIN32)
  gmtime_s(&tmUTC, &sec);
#else
  gmtime_r(&sec, &tmUTC);
#endif
  char buf[32];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &tmUTC);
  snprintf(out, outSize, "%s.%03dZ", buf, ms);
}
