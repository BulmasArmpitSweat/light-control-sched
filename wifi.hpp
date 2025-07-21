#pragma once
#include "common.hpp"
#include "init.hpp"
#include "mem.hpp"

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include <cstdio>

#define MAX_NETWORKS 20
#define RSSI_THRESHOLD -80

static constexpr const char *AP_SSID = "Pico-AP";
static constexpr const char *AP_PASSWORD = "Pico-AP";

static constexpr u8 AP_SSID_LEN = strlen(AP_SSID);
static constexpr u8 AP_PASS_LEN = strlen(AP_PASSWORD);

static constexpr const char *SSID = "Femboy Hooters";
static constexpr const char *PASSWORD = "g1mm3kr3ammm";

static constexpr u8 SSID_LEN = strlen(SSID);
static constexpr u8 PASS_LEN = strlen(PASSWORD);

struct scan_result_list {
  absolute_time_t time_to_scan;
  cyw43_ev_scan_result_t networks[MAX_NETWORKS];
  u8 num_networks;
};

struct scan_env {
  struct scan_result_list *list;
  u8 idx;
};

struct time_pretty {
  u8 minutes;
  u8 seconds;
  u16 milliseconds;
  u16 microseconds;
};

static inline bool wifi_is_connected(i32 mode) {
  return cyw43_wifi_link_status(&cyw43_state, mode) == CYW43_LINK_UP;
}

static inline bool wifi_init_sta() {
  // Initialise the Wi-Fi chip
  if (cyw43_arch_init()) {
    for (int i = 0; i < 5; i++) {
      cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
      sleep_ms(100);
      cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
      sleep_ms(100);
    }
    print_and_flush("Wi-Fi init failed - cannot continue\n");
    return false;
  }

  // Enable wifi station
  cyw43_arch_enable_sta_mode();

  int ret = cyw43_arch_wifi_connect_timeout_ms(SSID, PASSWORD,
                                               CYW43_AUTH_WPA2_AES_PSK, 10000);

  print_and_flush("Connecting to wifi...\n");
  if (ret == 0) {
    for (int i = 0; i < 3; i++) {
      cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
      sleep_ms(100);
      cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
      sleep_ms(100);
    }
  } else {
    for (int i = 0; i < 3; i++) {
      cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
      sleep_ms(500);
      cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
      sleep_ms(500);
    }
    print_and_flush("Wi-Fi connection failed - cannot continue\n");
    return false;
  }

  sleep_ms(1000); // Give DHCP time to complete
  return true;
}

static inline bool wifi_start_sta_mode() {
  if (wifi_is_connected(CYW43_ITF_AP))
    cyw43_arch_disable_ap_mode();
  cyw43_arch_enable_sta_mode();
  char *ip = ip4addr_ntoa(netif_ip4_addr(netif_list));
  print_and_flush("STA IP: %s\n", ip);
  return true;
}

static inline bool wifi_start_ap_broadcast() {
  if (wifi_is_connected(CYW43_ITF_STA))
    cyw43_arch_disable_sta_mode();
  cyw43_arch_enable_ap_mode(AP_SSID, AP_PASSWORD, CYW43_AUTH_WPA2_AES_PSK);
  char *ip = ip4addr_ntoa(netif_ip4_addr(netif_list));
  print_and_flush("AP IP: %s\n", ip);
  return true;
}

static inline bool get_mac(char* out, u32 len) {
  if (out == NULL || len != 18)
    return false;

  uint8_t mac[6];
  int r = cyw43_wifi_get_mac(&cyw43_state, CYW43_ITF_STA, mac);

  if (r == 0) {
    sprintf(out, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return true;
  }
  return false;
}

int last_scan_retval;

static inline int wifi_scan_cb(void *env, const cyw43_ev_scan_result_t *res) {
  struct scan_env *scan_env = (struct scan_env *)env;
  if (res && scan_env->idx < MAX_NETWORKS) {
    print_and_flush("ssid: %-32s rssi: %4d chan: %3d mac: "
                    "%02x:%02x:%02x:%02x:%02x:%02x sec: %u\n",
                    res->ssid, res->rssi, res->channel, res->bssid[0],
                    res->bssid[1], res->bssid[2], res->bssid[3], res->bssid[4],
                    res->bssid[5], res->auth_mode);

    if (res->rssi > RSSI_THRESHOLD) {
      last_scan_retval = 0;
      return 0;
    }

    u8 i;
    for (i = 0; i < scan_env->idx; i++) {
      if (strcmp((const char *)scan_env->list->networks[i].ssid,
                 (const char *)res->ssid) == 0) {
        last_scan_retval = 0;
        return 0;
      }
    }

    scan_env->list->networks[scan_env->idx++] = *res;
    last_scan_retval = 0;
    return 0;
  }
  last_scan_retval = 1;
  return 1;
}

static inline struct scan_result_list wifi_scan() {
  print_and_flush("Performing wifi scan...");
  struct scan_result_list list;

  absolute_time_t start = get_absolute_time();
  cyw43_wifi_scan_options_t opts = {0};
  struct scan_env scan_env = {&list, 0};
  while (true) {
    cyw43_wifi_scan(&cyw43_state, &opts, &scan_env, wifi_scan_cb);
    while (cyw43_wifi_scan_active(&cyw43_state))
      ;
    if (last_scan_retval == 1)
      break;
    sleep_ms(100); // Give wifi time to cool off I guess :shrug:
  }

  absolute_time_t end = get_absolute_time();
  absolute_time_t scan_time = end - start;

  list.time_to_scan = scan_time;
  list.num_networks = scan_env.idx;
  return list;
}

static inline struct time_pretty abs_to_time_pretty(absolute_time_t time) {
  struct time_pretty pretty;
  pretty.minutes = time / 60000000;
  pretty.seconds = (time / 1000000) % 60;
  pretty.milliseconds = (time / 1000) % 1000;
  pretty.microseconds = time % 1000;
  return pretty;
}

static inline struct string time_pretty_fmt(struct time_pretty time) {
  struct string result = string_nodef();

  if (time.minutes > 0)
    result = string_append_fmt(result, "Mins: %d, ", time.minutes);

  if (time.seconds > 0)
    result = string_append_fmt(result, "Secs: %d, ", time.seconds);

  if (time.milliseconds > 0)
    result = string_append_fmt(result, "Msecs: %d, ", time.milliseconds);

  if (time.microseconds > 0)
    result = string_append_fmt(result, "Usecs: %d", time.microseconds);

  return result;
}