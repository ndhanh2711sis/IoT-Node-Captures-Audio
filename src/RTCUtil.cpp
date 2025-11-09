#ifndef NO_RTC

#include "RTCUtil.h"
#include <Wire.h>
#include <RTClib.h>   // DateTime nằm trong RTClib.h

static RTC_DS3231 rtc;

bool RTC_init() {
  Wire.begin();
  if (!rtc.begin()) {
    Serial.println("[RTC] DS3231 not found");
    return false;
  }
  if (rtc.lostPower()) {
    Serial.println("[RTC] RTC lost power, set to compile time");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  Serial.println("[RTC] RTC OK");
  return true;
}

String RTC_getISO8601() {
  DateTime now = rtc.now();
  char buf[32];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02dZ",
           now.year(), now.month(), now.day(),
           now.hour(), now.minute(), now.second());
  return String(buf);
}

#endif  // NO_RTC
