#pragma once

#include "lwip/dns.h"
#include "lwip/sockets.h"
#include <stdio.h>

// Resolve google.com
// By far the most reliable test point
#define TEST_ADDR "google.com"

static inline void update_dns() {
  // Bail out and set DNS to known good one
  ip_addr_t google_dns;
  ipaddr_aton("8.8.8.8", &google_dns);
  dns_setserver(0, &google_dns);
}

static inline void dns_callback(const char *name, const ip_addr_t *ipaddr,
                                void *arg) {
  if (ipaddr) {
    print_and_flush("DNS async resolved: %s = %s\n", name, ipaddr_ntoa(ipaddr));
  } else {
    print_and_flush("DNS async failed: %s\n", name);
  }
}

static inline void test_dns_async() {
  ip_addr_t result;
  err_t err = dns_gethostbyname("google.com", &result, dns_callback, NULL);

  if (err == ERR_OK) {
    // IP was already cached
    print_and_flush("DNS (cached): google.com = %s\n", ipaddr_ntoa(&result));
  } else if (err == ERR_INPROGRESS) {
    print_and_flush("Async DNS lookup sent, waiting on callback...\n");
  } else {
    print_and_flush("Async DNS failed to start. Error code: %d\n", err);
  }
}

static inline char *get_dns_ip() {
  const ip_addr_t *server = dns_getserver(0);
  return ipaddr_ntoa(server);
}