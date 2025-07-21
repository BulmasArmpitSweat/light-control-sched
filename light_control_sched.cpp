#include "pico/cyw43_arch.h"
#include "pico/stdio.h"

#include "hardware/clocks.h"
#include "hardware/timer.h"
#include "hardware/xosc.h"

#include "FreeRTOS.h"
#include "task.h"

#include "common.hpp"
#include "defines.hpp"
#include "init.hpp"
#include "pwm.hpp"

// For more examples of timer use see
// https://github.com/raspberrypi/pico-examples/tree/master/timer For more
// examples of clocks use see
// https://github.com/raspberrypi/pico-examples/tree/master/clocks

repeating_timer_t light_timer;
repeating_timer_t watchdog_timer;

static bool watchdog_cb(repeating_timer_t *rt) {
  watchdog_update();
  return true;
}

static bool light_cb(repeating_timer_t *rt) {
  // TODO: Light control
  return true;
}

int main() {
  TaskHandle_t rtc_handle = NULL;
  TaskHandle_t watchdog_handle = NULL;
  TaskHandle_t wifi_init_handle = NULL;
  TaskHandle_t wifi_conn_handle = NULL;
  TaskHandle_t portal_check_handle = NULL;

  xosc_init();
  stdio_init_all();
  sleep_ms(2000); // Let USB serial settle

  print_and_flush("Light Control Scheduler - " VERSION_NUM "\n");

  print_and_flush("System Clock Frequency is %d MHz\n",
                  clock_get_hz(clk_sys) / 1000 / 1000);
  print_and_flush("USB Clock Frequency is %d MHz\n",
                  clock_get_hz(clk_usb) / 1000 / 1000);

  u32 rtcstatus = xTaskCreate(rtc_init_task, "RTC Init", 512, NULL,
                              FREERTOS_PRIORITY_INIT, &rtc_handle);

  u32 watchdogstatus =
      xTaskCreate(watchdog_init_task, "Watchdog Init", 512, NULL,
                  FREERTOS_PRIORITY_INIT, &watchdog_handle);

  u32 wifiinitstatus = xTaskCreate(wifi_init_task, "WiFi Init", 512, NULL,
                               FREERTOS_PRIORITY_INIT, &wifi_init_handle);
  add_repeating_timer_ms(1000, light_cb, NULL, &light_timer);
  add_repeating_timer_ms(100, watchdog_cb, NULL, &watchdog_timer);

  vTaskStartScheduler(); // Should never return

  cyw43_arch_deinit();
  xosc_dormant();
  asm volatile("b ."); // Hard fallback loop if an interrupt wakes us up again
}