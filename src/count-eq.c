/*
 * Copyright 2021 Koz Ross <koz.ross@retro-freedom.nz>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stddef.h>
#include "../include/diablo.h"

// Fill every 8-byte 'lane' with the same value.
static inline uint64_t broadcast(uint8_t const byte) {
  return byte * 0x0101010101010101ULL;
}

size_t diablo_count_eq(uint8_t const* const src,
                       size_t const off,
                       size_t const len,
                       uint8_t const byte) {
  size_t count = 0;
  // We step 64 bytes at a time.
  size_t const big_strides = len / 64;
  size_t const small_strides = len % 64;
  uint8_t const* ptr = (uint8_t const*)&(src[off]);
  // We use the method described in "Bit Twiddling Hacks".
  // Source: https://graphics.stanford.edu/~seander/bithacks.html#ZeroInWord
  uint64_t const matches = broadcast(byte);
  uint64_t const mask = broadcast(0x7F);
  for (size_t i = 0; i < big_strides; i++) {
    uint64_t const* big_ptr = (uint64_t const*)ptr;
    uint64_t result = 0;
    // Manual 8x loop unroll
    uint64_t input = *big_ptr ^ matches;
    uint64_t tmp = (input & mask) + mask;
    result |= ~(tmp | input | mask);
    input = *(big_ptr + 1) ^ matches;
    tmp = (input & mask) + mask;
    result |= (~(tmp | input | mask) >> 1);
    input = *(big_ptr + 2) ^ matches;
    tmp = (input & mask) + mask;
    result |= (~(tmp | input | mask) >> 2);
    input = *(big_ptr + 3) ^ matches;
    tmp = (input & mask) + mask;
    result |= (~(tmp | input | mask) >> 3);
    input = *(big_ptr + 4) ^ matches;
    tmp = (input & mask) + mask;
    result |= (~(tmp | input | mask) >> 4);
    input = *(big_ptr + 5) ^ matches;
    tmp = (input & mask) + mask;
    result |= (~(tmp | input | mask) >> 5);
    input = *(big_ptr + 6) ^ matches;
    tmp = (input & mask) + mask;
    result |= (~(tmp | input | mask) >> 6);
    input = *(big_ptr + 7) ^ matches;
    tmp = (input & mask) + mask;
    result |= (~(tmp | input | mask) >> 7);
    count += __builtin_popcountll(result);
    ptr += 64;
  }
  // Finish the rest slow.
  for (size_t i = 0; i < small_strides; i++) {
    if ((*ptr) == byte) {
      count++;
    }
    ptr++;
  }
  return count;
}
