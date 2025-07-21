#pragma once
#include "common.hpp"

#include "pico/stdlib.h"

#define MAX_TIME_SLICES 256

struct time_slice {
  absolute_time_t start;
  absolute_time_t end;
} packed;

struct schedule {
  u8 weekday;
  struct time_slice time_slices[MAX_TIME_SLICES];
} packed;