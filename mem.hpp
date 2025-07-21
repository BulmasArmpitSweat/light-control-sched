#pragma once

#include "common.hpp"
#include "pico/assert.h"
#include "pico/stdlib.h"

static inline void c_memcpy(void *__restrict dest, const void *__restrict src,
                            u32 n) {
  u32 i;
  for (i = 0; i < n; i++) {
    *((u8 *)dest + i) = *((u8 *)src + i);
  }
}

struct string {
  char *str;
  u32 len;
  bool init = false;
};

// A function to make code cleaner
static inline struct string string_nodef() {
  struct string res;
  res.len = 1;
  return res;
}

static inline struct string string_new(const u32 len) {
  struct string s;

  s.str = (char *)malloc(len);
  assert(s.str);
  s.len = len;

  return s;
}

static inline struct string string_init(const char *str, const u32 len) {
  struct string s;
  s.str = strdup(str);
  s.len = len;
  return s;
}

static inline void string_free(struct string str) {
  str.len = 0;
  free(str.str);
}

static inline struct string string_append(struct string str1,
                                          const char *str2) {
  u32 str2len = strlen(str2);
  if (!str1.init)
    return string_init(str2, str2len);
  // len is only 0 if str1 isn't valid
  assert(str1.len > 0);
  assert(str1.len + str2len > str1.len);

  u32 newlen = str1.len + str2len;
  struct string newstr = string_new(newlen);

  memcpy(newstr.str, str1.str, str1.len);
  memcpy(newstr.str + str1.len, str2, str2len);

  string_free(str1);
  return newstr;
}

static inline struct string string_prepend(struct string str1,
                                           const char *str2) {
  u32 str2len = strlen(str2);
  if (!str1.init)
    return string_init(str2, str2len);
  // len is only 0 if str1 isn't valid
  assert(str1.len > 0);
  assert(str1.len + str2len > str1.len);

  u32 newlen = str1.len + str2len;
  struct string newstr = string_new(newlen);

  memcpy(newstr.str, str2, str2len);
  memcpy(newstr.str + str2len, str1.str, str1.len);

  string_free(str1);
  return newstr;
}

static inline struct string string_fmt(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  u32 len = vsnprintf(NULL, 0, fmt, args);
  va_end(args);

  struct string newstr = string_new(len + 1);

  va_start(args, fmt);
  vsnprintf(newstr.str, len + 1, fmt, args);
  va_end(args);

  return newstr;
}

static inline struct string string_append_fmt(struct string str,
                                              const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  u32 len = vsnprintf(NULL, 0, fmt, args);
  va_end(args);

  char *append = (char *)malloc(len + 1);
  assert(append);

  va_start(args, fmt);
  vsnprintf(append, len + 1, fmt, args);
  va_end(args);

  struct string newstr = string_append(str, append); // Frees str
  free(append);
  return newstr;
}