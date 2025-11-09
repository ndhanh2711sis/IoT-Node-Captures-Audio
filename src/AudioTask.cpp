#include "AudioTask.h"
#include "Config.h"
#include "driver/i2s.h"
#include <esp_err.h>
#include <Arduino.h>

/*
 * AudioTask.cpp
 * - Khởi tạo I2S RX để đọc mic MEMS (ICS-43434).
 * - ICS-43434: 24-bit left-justified trong khung 32-bit -> chúng ta lưu 32-bit và >>8 để về "16-bit-like" khi xử lý.
 */

static i2s_port_t I2S_PORT = I2S_NUM_0;

void AudioTask_init() {
  i2s_config_t i2s_config = {};
  i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX);
  i2s_config.sample_rate = AUDIO_SR;
  i2s_config.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT;
  i2s_config.channel_format = MIC_LEFT ? I2S_CHANNEL_FMT_ONLY_LEFT : I2S_CHANNEL_FMT_ONLY_RIGHT;
  i2s_config.communication_format = I2S_COMM_FORMAT_STAND_I2S;
  i2s_config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
  i2s_config.dma_buf_count = 4;
  i2s_config.dma_buf_len = AUDIO_FRAME_LEN;
  i2s_config.use_apll = false;
  i2s_config.tx_desc_auto_clear = true;
  i2s_config.fixed_mclk = 0;

  i2s_pin_config_t pin_config = {};
  pin_config.mck_io_num = I2S_PIN_NO_CHANGE;
  pin_config.bck_io_num = (gpio_num_t)I2S_BCK;
  pin_config.ws_io_num  = (gpio_num_t)I2S_WS;
  pin_config.data_out_num = I2S_PIN_NO_CHANGE;
  pin_config.data_in_num  = (gpio_num_t)I2S_SD;

  esp_err_t e;
  e = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  if (e != ESP_OK) {
    Serial.printf("[AudioTask] i2s_driver_install ERR %d\n", e);
  }
  e = i2s_set_pin(I2S_PORT, &pin_config);
  if (e != ESP_OK) {
    Serial.printf("[AudioTask] i2s_set_pin ERR %d\n", e);
  }
  e = i2s_zero_dma_buffer(I2S_PORT);
  if (e != ESP_OK) {
    Serial.printf("[AudioTask] i2s_zero_dma_buffer ERR %d\n", e);
  }

  Serial.printf("[AudioTask] I2S ready SR=%d frame=%d BCK=%d WS=%d SD=%d\n",
                AUDIO_SR, AUDIO_FRAME_LEN, I2S_BCK, I2S_WS, I2S_SD);
}

bool AudioTask_readFrame(int32_t *buf, size_t &samples) {
  size_t nbytes = 0;
  esp_err_t e = i2s_read(I2S_PORT, (void*)buf,
                         sizeof(int32_t) * AUDIO_FRAME_LEN,
                         &nbytes, portMAX_DELAY);
  if (e != ESP_OK) return false;
  samples = nbytes / sizeof(int32_t);
  return true;
}
