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
#include "../include/utils.h"

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
    for (ptrdiff_t j = 0; j < 8; j++) {
      uint64_t const input = (*(big_ptr + j)) ^ matches;
      uint64_t const tmp = (input & mask) + mask;
      result |= (~(tmp | input | mask) >> j);
    }
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
