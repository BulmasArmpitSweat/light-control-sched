#pragma once
#include "common.hpp"
#include "defines.hpp"
#include "ip.hpp"
#include "mem.hpp"
#include "wifi.hpp"

#include "pico/cyw43_arch.h"

#include "lwip/sockets.h"
#include "lwipopts.h"

#define DEF_PORT 80

struct form_key {
  const char *key;
  const char *value;
};

const char *root = "<!DOCTYPE html>\n"
                   "<html>\n"
                   "  <head>\n"
                   "    <meta charset=\"utf-8\">\n"
                   "    <title>Light Control Scheduler</title>\n"
                   "  </head>\n"
                   "  <body>\n"
                   "    <h1>Light Control Scheduler</h1>\n"
                   "    <p>Version: " VERSION_NUM "</p>\n"
                   "    <p>System Clock Frequency: %d MHz</p>\n"
                   "    <p>USB Clock Frequency: %d MHz</p>\n"
                   "    <p>Using DNS server: %s</p>\n"
                   "    <p>IP: %s</p>\n"
                   "    <p>Gateway: %s</p>\n"
                   "    <p>Netmask: %s</p>\n"
                   "    <p>Mac: %s</p>\n"
                   "    <hr>"
                   "    <form action=\"/configure\" method=\"GET\">\n"
                   "       <button type=\"submit\" name=\"menu\" "
                   "value=\"wifi\">Configure Wifi</button>\n"
                   "       <button type=\"submit\" name=\"menu\" "
                   "value=\"sched\">Configure Schedule</button>\n"
                   "       <button type=\"submit\" name=\"menu\" "
                   "value=\"reboot\">Reboot</button>\n"
                   "    </form>\n"
                   "  </body>\n"
                   "</html>\n";

const char *wifi =
    "<!DOCTYPE html>\n"
    "<html>\n"
    "  <style>\n"
    "       .info {\n"
    "           opacity=40%\n"
    "           font-size=50%\n"
    "       }\n"
    "  </style>\n"
    "  <head>\n"
    "    <meta charset=\"utf-8\">\n"
    "    <title>Wifi Configuration</title>\n"
    "  </head>\n"
    "  <body>\n"
    "    <h1>Available Networks</h1>\n"
    "    <ul>%s</ul>\n"
    "    <hr>\n"
    "    <h1>Wifi Configuration</h1>\n"
    "    <form action=\"/wifi\" method=\"POST\">\n"
    "       <label for=\"ssid\">SSID</label>\n"
    "       <input type=\"text\" id=\"ssid\" name=\"ssid\" value=\"%s\">\n"
    "       <label for=\"password\">Password</label>\n"
    "       <input type=\"password\" id=\"password\" name=\"password\" "
    "value=\"%s\">\n"
    "       <button type=\"submit\" name=\"btn\" value=\"save\">Save</button>\n"
    "    </form>\n"
    "  </body>\n"
    "</html>\n";

const char *reset = "<!DOCTYPE html>\n"
                    "<html>\n"
                    "  <head>\n"
                    "    <meta charset=\"utf-8\">\n"
                    "    <title>Wifi Configuration</title>\n"
                    "  </head>\n"
                    "  <body>\n"
                    "    <h2>Device will reboot in 5 seconds</h2>"
                    "    <p>It will then attempt to initialize with the last "
                    "saved configuration</p>\n"
                    "    <p>If it fails, this portal will become available "
                    "again to attempt a new configuration.</p>"
                    "  </body>\n"
                    "</html>\n";

const char *not_found = "<!DOCTYPE html>\n"
                        "<html>\n"
                        "  <head>\n"
                        "    <meta charset=\"utf-8\">\n"
                        "    <title>404 Not Found</title>\n"
                        "  </head>\n"
                        "  <body>\n"
                        "    <h1>404 Not Found</h1>\n"
                        "  </body>\n"
                        "</html>\n";

const char *bad_request = "<!DOCTYPE html>\n"
                          "<html>\n"
                          "  <head>\n"
                          "    <meta charset=\"utf-8\">\n"
                          "    <title>400 Bad Request</title>\n"
                          "  </head>\n"
                          "  <body>\n"
                          "    <h1>400 Bad Request</h1>\n"
                          "  </body>\n"
                          "</html>\n";

static inline u16 parse_form(struct string resp, struct form_key *keys,
                             u16 max_keys) {
  u16 count = 0;
  for (char *substr = resp.str; *substr && count < max_keys; substr = NULL) {
    char *key = substr;
    char *eq = strchr(substr, '=');

    if (!eq)
      break;
    *eq = '\0';

    char *val = eq + 1;
    char *amp = strchr(val, '&');
    if (amp)
      *amp = '\0';

    keys[count++] = (struct form_key){key, val};
    if (!amp)
      break;

    substr = amp + 1;
  }
  return count;
}

static inline struct string wifi_scan_to_html(struct scan_result_list list) {
  const char *header = "<h2>Available Networks</h2>\n";
  u32 headerlen = strlen(header);
  struct string result = string_init(header, headerlen);
  result = string_append(result, "<dl>\n");

  u8 i;
  for (i = 0; i < list.num_networks; i++) {
    cyw43_ev_scan_result_t item = list.networks[i];
    result = string_append_fmt(result, "    <dt>%s</dt>\n", item.ssid);
    result = string_append_fmt(
        result, "    <dd class=\"info\">rssi: %4d</dd>\n", item.rssi);
    result = string_append_fmt(
        result, "    <dd class=\"info\">chan: %3d</dd>\n", item.channel);
    result = string_append_fmt(
        result,
        "    <dd class=\"info\">mac: %02x:%02x:%02x:%02x:%02x:%02x</dd>\n",
        item.bssid[0], item.bssid[1], item.bssid[2], item.bssid[3],
        item.bssid[4], item.bssid[5]);
    result = string_append_fmt(result, "    <dd class=\"info\">0x%02x</dd>\n",
                               item.auth_mode);
    result = string_append(result, "    <br>\n");
  }

  result = string_append(result, "</dl>\n");
  result = string_append(result, "<br>\n");

  struct time_pretty time_pretty = abs_to_time_pretty(list.time_to_scan);
  struct string time_pretty_str = time_pretty_fmt(time_pretty);

  result = string_append_fmt(result, "<p>Time to scan: %s</p>\n",
                             time_pretty_str.str);
  result = string_append(result, "<hr>");

  return result;
}

static inline i32 captive_portal_start() {
  i32 sock = lwip_socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    return -1;

  struct sockaddr_in serv_addr = {
      .sin_family = AF_INET,
      .sin_port = lwip_htons(DEF_PORT),
      .sin_addr = {.s_addr = lwip_htonl(INADDR_ANY)},
  };

  lwip_bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  lwip_listen(sock, 1);
  print_and_flush("Captive portal started on port %d\n", DEF_PORT);
  return sock;
}

static inline void captive_portal_send(i32 sock, u32 code, const char *type,
                                       struct string body) {
  struct string header = string_nodef();
  struct string content_type = string_nodef();
  struct string content_len = string_nodef();

  header = string_append_fmt(header, "HTTP/1.1 %d %s\r", code,
                             (code == 200   ? "OK"
                              : code == 400 ? "Bad Request"
                                            : "Not Found"));
  content_type = string_append_fmt(content_type, "Content-Type: %s\r", type);
  content_len =
      string_append_fmt(content_len, "Content-Length: %d\r", body.len);

  print_and_flush("Response: %s | %s | %s", header.str, content_type.str,
                  content_len.str);

  char buf[1024];
  u32 hlen = snprintf(buf, sizeof(buf), "%s\n%s\n%s\n\r\n", header.str,
                      content_type.str, content_len.len);

  string_free(header);
  string_free(content_type);
  string_free(content_len);

  lwip_send(sock, buf, hlen, 0);
  lwip_send(sock, body.str, body.len, 0);
}

static bool reset_root = true;

// TODO
static inline void captive_portal_sched(i32 sock) {
  reset_root = true;
  return;
}

// TODO
static inline void captive_portal_wifi(i32 sock) {
  reset_root = true;
  return;
}

static inline void captive_portal_reset(i32 sock) {
  struct string reset_string = string_init(reset, strlen(reset));
  captive_portal_send(sock, 200, "text/html", reset_string);
  string_free(reset_string);
}

static inline void captive_portal_root(i32 sock) {
  while (true) {
    if (reset_root) {
      reset_root = false;
      const u32 sysclk = system_clock_get();
      const u32 usbclk = usb_clock_get();
      const char *dns = get_dns_ip();
      const char *ip_addr = ip4addr_ntoa(netif_ip4_addr(netif_list));
      const char *gateway = ip4addr_ntoa(netif_ip4_gw(netif_list));
      const char *netmask = ip4addr_ntoa(netif_ip4_netmask(netif_list));
      char mac[18];
      mac[17] = '\0'; // just in case
      if (!get_mac(mac, strlen(mac))) {
        print_and_flush(
            "Cannot get MAC address - not critical, but not good either");
        strcpy(mac, "00:00:00:00:00:00");
      }

      struct string body = string_nodef();
      body = string_append_fmt(body, root, sysclk, usbclk, dns, ip_addr,
                               gateway, netmask, mac);

      captive_portal_send(sock, 200, "text/html", body);
      string_free(body);
    }

    i32 client = lwip_accept(sock, NULL, NULL);
    if (client < 0)
      continue;

    char buf[1024];
    i32 len = lwip_recv(client, buf, sizeof(buf), 0);
    lwip_close(client);

    print_and_flush("Received %d bytes\n", len);
    if (len <= 0)
      break;
    buf[len] = '\0';

    struct string bufstr = string_init(buf, strlen(buf));
    struct form_key keys[3];
    u16 count = parse_form(bufstr, keys, 3);
    string_free(bufstr);

    u16 i;
    for (i = 0; i < count; i++) {
      struct form_key curr_key = keys[i];
      if (strcmp(curr_key.key, "menu") == 0 &&
          strcmp(curr_key.value, "reboot") == 0) {
        captive_portal_reset(sock);
        sleep_ms(5000);
        break;
      }

      if (strcmp(curr_key.key, "menu") == 0 &&
          strcmp(curr_key.value, "wifi") == 0) {
        captive_portal_wifi(sock);
      }

      if (strcmp(curr_key.key, "menu") == 0 &&
          strcmp(curr_key.value, "sched") == 0) {
        captive_portal_sched(sock);
      }
    }
  }
}

static inline void captive_portal_stop(i32 sock) {
  lwip_close(sock);
  print_and_flush("Captive portal stopped\n");
}