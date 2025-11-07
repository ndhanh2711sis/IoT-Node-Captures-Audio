#pragma once
#include <Arduino.h>
#include <stdlib.h>

inline void* audio_alloc(size_t n) {
  // nếu có PSRAM thì dùng ps_malloc, nếu không thì dùng malloc
  return (psramFound() ? ps_malloc(n) : malloc(n));
}
