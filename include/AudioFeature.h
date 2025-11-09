#pragma once
#include <Arduino.h>

/**
 * AudioFeature.h
 * Các hàm xử lý tín hiệu cơ bản:
 * - computeRMS: tính RMS của frame (đưa 24-bit về dạng thích hợp)
 * - computePeak: tính peak absolute (có thể dùng khi alert)
 */
float AudioFeature_computeRMS(int32_t *buf, size_t samples);
int32_t AudioFeature_computePeak(int32_t *buf, size_t samples);
