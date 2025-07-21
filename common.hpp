#pragma once
#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"
#include "pico/stdio.h"

#include <stdint.h>

#include "FreeRTOSConfig.h"

#define GPIO_AP 16

#define FLASH_MAGIC 0xdeadbeef /* Moo */

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

#define print_and_flush(str, ...)                                              \
  printf(str, ##__VA_ARGS__);                                                  \
  stdio_flush()

#define packed __attribute__((__packed__))

#define AIRCR_OFF 0x0ED0C
#define AIRCR_REG (*((volatile u32 *)(PPB_BASE + AIRCR_OFF)))
#define AIRCR_RESET 0x5FA0004

#define FREERTOS_PRIORITY_MINIMUM 0
#define FREERTOS_PRIORITY_MAXIMUM (configMAX_PRIORITIES - 1)

// Priorities for each task
// Please note that these numbers are completely arbitrary
// Don't fuck with this
#define FREERTOS_PRIORITY_NTP FREERTOS_PRIORITY_MINIMUM + 1
#define FREERTOS_PRIORITY_PORTAL FREERTOS_PRIORITY_MINIMUM
#define FREERTOS_PRIORITY_INIT FREERTOS_PRIORITY_MAXIMUM

static inline void soft_reset(void) {
  sleep_ms(1);
  print_and_flush("Resetting...\n");
  AIRCR_REG = AIRCR_RESET;
  asm volatile("b .");
}

static inline const u32 system_clock_get() {
  return clock_get_hz(clk_sys) / 1000 / 1000;
}

static inline const u32 usb_clock_get() {
  return clock_get_hz(clk_usb) / 1000 / 1000;
}