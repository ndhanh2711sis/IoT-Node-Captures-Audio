#pragma once
#include <Arduino.h>

/**
 * AudioTask.h
 *  - Hàm AudioTask_init(): khởi tạo I2S.
 *  - Hàm AudioTask_readFrame(buf, samples): đọc 1 frame (AUDIO_FRAME_LEN) vào buf.
 *
 * Buf dùng int32_t per sample (I2S 32-bit slot). ICS-43434 trả 24-bit left-justified.
 */
void AudioTask_init();
bool AudioTask_readFrame(int32_t *buf, size_t &samples);
