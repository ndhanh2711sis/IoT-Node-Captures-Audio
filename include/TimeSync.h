#pragma once
#include <Arduino.h>

namespace TimeSync {
  // Kết nối Wi-Fi (nếu có) + NTP. Không bắt buộc phải thành công để chạy mock.
  // ssid/pass có thể là "" nếu bạn chưa cấu hình.
  void begin(const char* ssid, const char* pass);

  // Milliseconds từ lúc boot (proxy cho timestamp nội bộ pipeline)
  uint64_t nowMs();

  // Chuyển ms-tính-từ-boot → ISO-8601 UTC nếu đã sync NTP,
  // nếu chưa sync sẽ fallback theo millis() tương đối (vẫn hợp lệ để debug).
  void toISO8601(uint64_t bootMs, char* out, size_t outSize);

  // Trả về true nếu đã sync NTP (có mapping bootMs→epoch).
  bool isSynced();
}
