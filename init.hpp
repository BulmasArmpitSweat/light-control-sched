#pragma once

#include "common.hpp"

#include "hardware/gpio.h"
#include "hardware/rtc.h"
#include "hardware/watchdog.h"

#include "lwip/sockets.h"

#include "lwip/apps/sntp.h"

#include "FreeRTOS.h"
#include "task.h"

#include "html.hpp"
#include "ip.hpp"
#include "wifi.hpp"

TaskHandle_t portalHandle = NULL;

static bool portal_running = false;

static void rtc_init_task(void *param) {
  print_and_flush("[Started] Init RTC\n");
  rtc_init();
  vTaskDelay(1000);
  print_and_flush("[Finished] Init RTC\n");
}

static void watchdog_check_task(void *param) {
  print_and_flush("[Started] Watchdog Check\n");
  if (watchdog_caused_reboot()) {
    print_and_flush("The watchdog caused a reboot\n");
    print_and_flush("This is likely because of a programming fuck-up\n");
  }
  print_and_flush("[Finished] Watchdog Check\n");
}

static void watchdog_init_task(void *param) {
  print_and_flush("[Started] Watchdog Init\n");
  watchdog_enable(WATCHDOG_TIMEOUT, 1);
  print_and_flush("[Finished] Watchdog Init\n");
}

static void DNS_init_task(void *param) {
  print_and_flush("[Started] Init DNS\n");
  update_dns();
  print_and_flush("Using DNS server: %s\n", get_dns_ip());
  test_dns_async();
  print_and_flush("[Finished] Init DNS\n");
}

static void captive_portal_task(void *param) {
  wifi_start_ap_broadcast();
  print_and_flush("AP mode enabled\n");
  int sock = captive_portal_start();
  captive_portal_root(sock);
  lwip_close(sock);
  captive_portal_stop(sock);
  wifi_start_sta_mode();
  portal_running = false;
  vTaskDelete(NULL);
  portalHandle = NULL;
}

static void captive_portal_button_task(void *param) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(1000);

  for (;;) {
    vTaskDelayUntil(&xLastWakeTime, period);
    if (!portal_running) {
      int button_status = gpio_get(GPIO_AP);
      if (button_status == 0) {
        portal_running = true;
        xTaskCreate(captive_portal_task, "Captive Portal", 1024, NULL, 1,
                    &portalHandle);
      }
    }
  }
}

static void wifi_init_task(void *param) {
  print_and_flush("[Started] Init Wifi\n");

  if (!wifi_init_sta()) {
    print_and_flush("Wifi connection failed - opening config captive portal\n");
    xTaskCreate(captive_portal_task, "Captive Portal", 1024, NULL, 1, NULL);
    return;
  }

  // Wait for IP
  while (netif_ip4_addr(netif_list) == NULL)
    vTaskDelay(100);

  const char *ip_addr = ip4addr_ntoa(netif_ip4_addr(netif_list));
  const char *gateway = ip4addr_ntoa(netif_ip4_gw(netif_list));
  const char *netmask = ip4addr_ntoa(netif_ip4_netmask(netif_list));

  char mac[18];
  mac[17] = '\0';
  if (!get_mac(mac, strlen(mac))) {
    print_and_flush(
        "Cannot get MAC address - not critical, but not good either");
    strcpy(mac, "00:00:00:00:00:00");
  }

  print_and_flush("IP: %s | GW: %s | NM: %s | MAC: %s", ip_addr, gateway,
                  netmask, mac);
  print_and_flush("[Finished] Init Wifi\n");
}

static inline void wifi_connection_task(void *param) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(1000);

  for (;;) {
    if (!portal_running) {
      if (!wifi_is_connected(CYW43_ITF_STA)) {
      printf("Wifi dropped out - attempting reconnection\n");
        int ret = cyw43_arch_wifi_connect_timeout_ms(
            SSID, PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000);
        if (!ret) {
          printf("Wifi reconnection failed - trying again in 1 second\n");
          continue; // Try again
        }
      }
    }
  }
}

static void ntp_init_task(void *param) {
  print_and_flush("[Started] Init NTP\n");
  print_and_flush("Init NTP\n");
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_init();
  vTaskDelay(1000); // Give the NTP server time to respond
  print_and_flush("[Finished] Init NTP\n");
}