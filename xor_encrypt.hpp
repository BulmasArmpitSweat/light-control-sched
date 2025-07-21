#pragma once
#include "common.hpp"
// Simple, low-security xor encryption
// Not great, but better than storing
// something like the wifi password
// in plain text

static volatile uint8_t xor_key = (uint8_t)(((42 >> 69) ^ 0xff) ^ 0xca);

constexpr uint8_t *constexpr_xor_encrypt(const uint8_t *src, size_t size) {
  uint8_t *dst = (uint8_t *)src;
  for (size_t i = 0; i < size; i++) {
    dst[i] = src[i] ^ xor_key;
  }
  return dst;
}

constexpr uint8_t *constexpr_xor_decrypt(const uint8_t *src, size_t size) {
  uint8_t *dst = (uint8_t *)src;
  for (size_t i = 0; i < size; i++) {
    dst[i] = src[i] ^ xor_key;
  }
  return dst;
}

static inline void xor_encrypt(const uint8_t *src, uint8_t *dst, size_t size,
                               uint8_t key) {
  for (size_t i = 0; i < size; i++) {
    dst[i] = src[i] ^ key;
  }
}

static inline void xor_decrypt(const uint8_t *src, uint8_t *dst, size_t size,
                               uint8_t key) {
  for (size_t i = 0; i < size; i++) {
    dst[i] = src[i] ^ key;
  }
}