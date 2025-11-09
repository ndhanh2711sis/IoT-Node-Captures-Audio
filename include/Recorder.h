#pragma once
#include <Arduino.h>

/**
 * Recorder.h
 * - Recorder_init(): mount LittleFS
 * - Recorder_saveClip(preBuf, preSamples, postBuf, postSamples, filename): ghi WAV 16-bit mono
 * - Recorder_saveRingAndPost(...): tiện ích lưu pre-roll ring + post frames
 *
 * Lưu ý: data buffer là int32_t (I2S raw), phải convert >>8 -> int16_t trước khi ghi WAV.
 */
bool Recorder_init();
bool Recorder_saveWavFromBuffers(int16_t *pcm16, size_t samples, const char *fname);
bool Recorder_saveWavFromInt32(int32_t *buf32, size_t samples, const char *fname);
