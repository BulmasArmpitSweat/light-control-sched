#pragma once

#include "common.hpp"
#include "hardware/flash.h"
#include "mem.hpp"

#define FLASH_TARGET_OFFSET (256 * 1024)

extern u8 __flash_binary_end;
uintptr_t binary_end_aligned =
    ((uintptr_t)&__flash_binary_end + FLASH_SECTOR_SIZE - 1) &
    ~(FLASH_SECTOR_SIZE - 1);
static const uintptr_t FLASH_START = binary_end_aligned;

struct flash_packet {
  u32 magic;
  u32 size;
  u8 data[];
} packed;

static inline constexpr u32 align_pos(u32 pos) {
  return (pos + FLASH_SECTOR_SIZE - 1) & ~(FLASH_SECTOR_SIZE - 1);
}

static inline u32 get_packet_size(struct flash_packet *packet) {
  return sizeof(struct flash_packet) + packet->size;
}

static inline bool verify_packet_size(struct flash_packet *packet) {
  return get_packet_size(packet) <= FLASH_SECTOR_SIZE;
}

// Assumes pos is aligned to a sector using the align_pos function
static inline void write_packet(struct flash_packet *packet, u32 pos) {
  u32 size = get_packet_size(packet);
  u32 off = FLASH_START + pos;
  u8 temp[size];
  c_memcpy(temp, packet, size);
  flash_range_erase(off, FLASH_SECTOR_SIZE);
  flash_range_program(off, temp, size);
}

// Assumes pos is aligned to a sector using the align_pos function
static inline const struct flash_packet *read_packet(u32 pos) {
  const struct flash_packet *packet =
      (const struct flash_packet *)(FLASH_START + pos);
  if (packet->magic != FLASH_MAGIC)
    return NULL;
  return packet;
}