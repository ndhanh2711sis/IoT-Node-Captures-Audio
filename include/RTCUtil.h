#pragma once
#include <Arduino.h>

/**
 * RTCUtil.h
 * - Khi NO_RTC=1: dùng stub (build cho env smoke, không cần thư viện RTClib).
 * - Khi NO_RTC không bật: dùng DS3231 qua RTClib (include nằm ở .cpp).
 */

#ifndef NO_RTC
bool RTC_init();           // Khởi tạo RTC DS3231
String RTC_getISO8601();   // "YYYY-MM-DDTHH:MM:SSZ"
#else
inline bool RTC_init() {
  Serial.println("[RTC] disabled (NO_RTC)"); 
  return true;
}
inline String RTC_getISO8601() { 
  return String("1970-01-01T00:00:00Z"); 
}
#endif
