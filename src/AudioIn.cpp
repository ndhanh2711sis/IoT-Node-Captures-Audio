#include "AudioIn.h"
#include <LittleFS.h>
#include "Memory.h"
using namespace AudioIn;

static AudioChunkCB g_cb = nullptr;
static uint32_t g_sr = 16000;
static size_t  g_chunkMs = 20;
static size_t  g_chunkSamples = 320; // cập nhật theo sr/chunkMs

#if defined(BUILD_PROFILE_HW)
  #include "driver/i2s.h"
  static bool g_hwReady = false;
#else
  // MOCK
  static File g_f;
  static uint64_t g_lastMicros = 0;
  static const char* MOCK_PATH = "/mock.wav";
  static uint32_t g_bytesData = 0;
  static uint32_t g_bytesRead = 0;

  // rất đơn giản: bỏ qua header 44 byte nếu thấy "RIFF"
  static bool openMock(uint32_t sr) {
    if (!LittleFS.exists(MOCK_PATH)) return false;
    g_f = LittleFS.open(MOCK_PATH, "rb");
    if (!g_f) return false;
    uint8_t hdr[44];
    int n = g_f.read(hdr, 44);
    if (n < 44) { g_f.close(); return false; }
    if (memcmp(hdr, "RIFF", 4)==0 && memcmp(hdr+8, "WAVE", 4)==0) {
      g_bytesData = *reinterpret_cast<uint32_t*>(&hdr[40]);
    } else {
      // không phải WAV -> đọc như raw
      g_f.seek(0, SeekSet);
      g_bytesData = g_f.size();
    }
    g_bytesRead = 0;
    g_lastMicros = micros();
    return true;
  }
#endif

bool AudioIn::begin(uint32_t sr, size_t chunkMs, AudioChunkCB cb) {
  g_sr = sr;
  g_chunkMs = chunkMs;
  g_chunkSamples = (g_sr * g_chunkMs) / 1000;
  g_cb = cb;

#if defined(BUILD_PROFILE_HW)
  // I2S RX mono 16k/16-bit
  i2s_config_t cfg = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = (int)g_sr,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = (int)g_chunkSamples,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0,
    .mclk_multiple = I2S_MCLK_MULTIPLE_DEFAULT,
    .bits_per_chan = I2S_BITS_PER_CHAN_16BIT
  };
  // CHỈNH CHÂN THEO PHẦN CỨNG CỦA BẠN
  i2s_pin_config_t pin = {
    .bck_io_num = 13,     // BCLK
    .ws_io_num  = 14,     // LRCLK/WS
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num  = 12    // DOUT từ mic
  };
  esp_err_t e;
  e = i2s_driver_install(I2S_NUM_0, &cfg, 0, nullptr);
  if (e != ESP_OK) return false;
  e = i2s_set_pin(I2S_NUM_0, &pin);
  if (e != ESP_OK) return false;
  e = i2s_zero_dma_buffer(I2S_NUM_0);
  if (e != ESP_OK) return false;
  g_hwReady = true;
  return true;
#else
  return openMock(sr);
#endif
}

void AudioIn::loop() {
  if (!g_cb) return;

#if defined(BUILD_PROFILE_HW)
  if (!g_hwReady) return;
  size_t needBytes = g_chunkSamples * sizeof(int16_t);
  int16_t* buf = (int16_t*)audio_alloc(needBytes);
  if (!buf) return;
  size_t br = 0;
  esp_err_t e = i2s_read(I2S_NUM_0, buf, needBytes, &br, portMAX_DELAY);
  if (e == ESP_OK && br == needBytes) {
    AudioChunk ch{buf, g_chunkSamples};
    g_cb(ch);
  }
  free(buf);
#else
  if (!g_f) return;
  // Pace theo thời gian thực:  chunkMs
  if (micros() - g_lastMicros < (uint64_t)g_chunkMs * 1000) return;
  g_lastMicros += (uint64_t)g_chunkMs * 1000;

  size_t needBytes = g_chunkSamples * sizeof(int16_t);
  int16_t* buf = (int16_t*)audio_alloc(needBytes);
  if (!buf) return;

  int n = g_f.read((uint8_t*)buf, needBytes);
  if (n < (int)needBytes) {
    // loop lại từ đầu phần data
    g_f.seek(44, SeekSet);
    g_bytesRead = 0;
    int n2 = g_f.read((uint8_t*)buf + n, needBytes - n);
    if (n2 <= 0) { free(buf); return; }
    n += n2;
  }
  g_bytesRead += n;
  AudioChunk ch{buf, g_chunkSamples};
  g_cb(ch);
  free(buf);
#endif
}
