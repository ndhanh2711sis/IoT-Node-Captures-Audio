#pragma once
/**
 * Config.h
 * Trung tâm cấu hình dự án.
 * Chỉnh các tham số: WiFi, MQTT, audio, pre/post roll...
 */

// ----------- WiFi / MQTT (CHỈNH THEO MÔI TRƯỜNG) ------------
#define WIFI_SSID     "your_ssid"
#define WIFI_PASSWORD "your_password"

#define MQTT_BROKER    "192.168.1.10"   // broker ip/host
#define MQTT_PORT      1883
#define MQTT_USER      ""               // nếu có
#define MQTT_PASS      ""

#define DEVICE_ID      "node-audio-001" // id thiết bị (topic)

// ----------- Audio / I2S -----------
#define AUDIO_SR         32000     // sample rate (Hz)
#define AUDIO_FRAME_LEN  1024      // mẫu/frame (32ms @32kHz)
#ifndef I2S_BCK
  #define I2S_BCK 5
#endif
#ifndef I2S_WS
  #define I2S_WS 6
#endif
#ifndef I2S_SD
  #define I2S_SD 4
#endif
#ifndef MIC_LEFT
  #define MIC_LEFT 1
#endif

// ----------- Pre-roll / Post-roll (số mẫu) -----------
#define PREROLL_SECONDS   2       // lưu 2s trước khi trigger
#define POSTROLL_SECONDS  2       // lưu 2s sau trigger

// Pre/post convert thành số mẫu (AUDIO_SR * seconds)
#define PREROLL_SAMPLES  (AUDIO_SR * PREROLL_SECONDS)
#define POSTROLL_SAMPLES (AUDIO_SR * POSTROLL_SECONDS)

// để tiện, frame size = AUDIO_FRAME_LEN, nên ring buffer lưu theo mẫu 32-bit, kích thước PREROLL_SAMPLES

// ----------- Detection parameters -----------
#define RMS_BASELINE_MS      5000   // thời gian (ms) để tính baseline noise ban đầu
#define DETECT_THRESHOLD_FACTOR 4.0 // trigger khi RMS > baseline * factor
#define DETECT_CONSENSUS_FRAMES 3  // cần 3 frame liên tiếp vượt threshold để xác nhận
