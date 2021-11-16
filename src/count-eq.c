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

static inline size_t count_eq_rest (uint8_t const* const src, 
                                    size_t const len,
                                    uint8_t const byte) {
  size_t count = 0;
  uint8_t const* ptr = (uint8_t const*)src;
  for (size_t i = 0; i < len; i++) {
    if ((*ptr) == byte) {
      count++;
    }
    ptr++;
  }
  return count;
}

#if (__i386__ && __SSE2__)
// Pure SSE implementation, as i386 _cannot_ have AVX.

#include <emmintrin.h>

size_t diablo_count_eq (uint8_t const* const src,
                        size_t const off,
                        size_t const len,
                        uint8_t const byte) {
  size_t count = 0;
  size_t const big_strides = len / 64;
  size_t const small_strides = len % 64;
  uint8_t const* ptr = (uint8_t const*)&(src[off]);
  if (big_strides != 0) {
    __m128i const matches = _mm_set1_epi8(byte);
    __m128i counts = _mm_setzero_si128();
    for (size_t i = 0; i < big_strides; i++) {
      __m128i const* big_ptr = (__m128i const*)ptr;
      // Load and compare. This leaves 0xFF in a matching lane, and 0x00
      // otherwise.
      //
      // This is a manual 4x unroll.
      __m128i const results[4] = {
        _mm_cmpeq_epi8(matches, _mm_loadu_si128(big_ptr)),
        _mm_cmpeq_epi8(matches, _mm_loadu_si128(big_ptr + 1)),
        _mm_cmpeq_epi8(matches, _mm_loadu_si128(big_ptr + 2)),
        _mm_cmpeq_epi8(matches, _mm_loadu_si128(big_ptr + 3))
      };
      // Since 0xFF is -1, we can get a lane-by-lane match count (except
      // negative) by adding.
      __m128i const summed = _mm_add_epi8(_mm_add_epi8(results[0], results[1]),
                                          _mm_add_epi8(results[2], results[3]));
      // By taking the sum of absolute differences with 0x00 in each lane, we
      // get two 64-bit counts, which we accumulate.
      counts = _mm_add_epi64(counts, _mm_sad_epu8(summed, _mm_setzero_si128()));
      ptr += 64;
    }
    // Evacuate results and sum.
    uint64_t results[2];
    _mm_storeu_si128((__m128i*)results, counts);
    count += (results[0] + results[1]);
  }
  count += count_eq_rest(ptr, small_strides, byte);
  return count;
}
#else
// Fallback implementation
//
// We use the method described in "Bit Twiddling Hacks".
// Source: https://graphics.stanford.edu/~seander/bithacks.html#ZeroInWord

// Load a 64-bit word, then set every byte which matches to 0x80, while setting
// the others to 0x00.
static inline uint64_t load_and_set (uint64_t const* const big_ptr,
                                     size_t const i,
                                     uint64_t const matches,
                                     uint64_t const mask) {
  uint64_t const input = big_ptr[i] ^ matches;
  uint64_t const tmp = (input & mask) + mask;
  return (~(tmp | input | mask) >> i);
}


// Fill every 8-byte 'lane' with the same value.
static inline uint64_t broadcast(uint8_t const byte) {
  return byte * 0x0101010101010101ULL;
}

size_t diablo_count_eq (uint8_t const* const src,
                        size_t const off,
                        size_t const len,
                        uint8_t const byte) {
  size_t count = 0;
  size_t const big_strides = len / 64;
  size_t const small_strides = len % 64;
  uint8_t const* ptr = (uint8_t const*)&(src[off]);
  if (big_strides != 0) {
    uint64_t const matches = broadcast(byte);
    uint64_t const mask = broadcast(0x7F);
    for (size_t i = 0; i < big_strides; i++) {
      uint64_t const* const big_ptr = (uint64_t const* const)ptr;
      uint64_t result = 0;
      // Manual 8x loop unroll
      result |= load_and_set(big_ptr, 0, matches, mask);
      result |= load_and_set(big_ptr, 1, matches, mask);
      result |= load_and_set(big_ptr, 2, matches, mask);
      result |= load_and_set(big_ptr, 3, matches, mask);
      result |= load_and_set(big_ptr, 4, matches, mask);
      result |= load_and_set(big_ptr, 5, matches, mask);
      result |= load_and_set(big_ptr, 6, matches, mask);
      result |= load_and_set(big_ptr, 7, matches, mask);
      count += __builtin_popcountll(result);
      ptr += 64;
    }
  }
  count += count_eq_rest(ptr, small_strides, byte);
  return count;
}
#endif
