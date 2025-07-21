#pragma once
#include "common.hpp"

static inline f32 powfi(f32 base, u32 exp) {
  f32 res = 1.0f;
  while (exp > 0) {
    if (exp & 1)
      res *= base;
    base *= base;
    exp >>= 1;
  }
  return res;
}

static inline f32 easing(f32 x) {
  return x < 0.5 ? 16.0 * powfi(x, 5) : 1 - powfi(-2.0 * x + 2.0, 5) / 2.0;
}