#pragma once
#include "common.h"

#include "hardware/rtc.h"
#include "pico/stdlib.h"
#include "pico/types.h"

static inline bool rtc_is_init() {
  datetime_t *temp = {0};
  bool res = rtc_get_datetime(temp);
  return res;
}

static inline absolute_time_t time_since_boot_us() { return time_us_64(); }