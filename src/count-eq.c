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

// Interface for all implementations.

// Stores any implementation-specific data.
typedef struct count_eq_env count_eq_env;

// Build an environment based on what byte we're looking for.
static inline count_eq_env setup_env (uint8_t const byte);

// Retrieve our block size (in bytes).
static inline size_t retrieve_block_size (count_eq_env const* env);

// Retrieve the count.
static inline size_t retrieve_count (count_eq_env const* env);

// Load and count a block.
static inline void count_block (uint8_t const* const src,
                                count_eq_env* env);

#if (__i386__ && __SSE2__)
#include <emmintrin.h>
// SSE2-only implementation. This is only going to be used on x86 (not 64), so
// it _can't_ have AVX.

struct count_eq_env {
  __m128i matches;
  __m128i counts;
};

__attribute__((pure))
static inline count_eq_env setup_env (uint8_t const byte) {
  return (count_eq_env){ .matches = _mm_set1_epi8(byte),
                         .counts = _mm_setzero_si128() };
}

__attribute__((pure, leaf))
static inline size_t retrieve_block_size (count_eq_env const* env) {
  // This suppresses the 'unused' warning, without enforcing reading from env.
  // See: https://stackoverflow.com/a/4851173/2629787
  (void)(sizeof((env), 0));
  return 128;
}

__attribute__((leaf))
static inline size_t retrieve_count (count_eq_env const* env) {
  uint64_t results[2];
  _mm_storeu_si128((__m128i*)results, env->counts);
  return results[0] + results[1];
}

static inline void count_block (uint8_t const* const src,
                                count_eq_env* env) {
  __m128i const* big_ptr = (__m128i const*)src;
  // Load eight blocks, compare with target. This gives 0xFF on a match, and 0x00
  // otherwise, per lane.
  // This is a manual 8x unroll.
  __m128i const results[8] = {
    _mm_cmpeq_epi8(_mm_loadu_si128(big_ptr), env->matches),
    _mm_cmpeq_epi8(_mm_loadu_si128(big_ptr + 1), env->matches),
    _mm_cmpeq_epi8(_mm_loadu_si128(big_ptr + 2), env->matches),
    _mm_cmpeq_epi8(_mm_loadu_si128(big_ptr + 3), env->matches),
    _mm_cmpeq_epi8(_mm_loadu_si128(big_ptr + 4), env->matches),
    _mm_cmpeq_epi8(_mm_loadu_si128(big_ptr + 5), env->matches),
    _mm_cmpeq_epi8(_mm_loadu_si128(big_ptr + 6), env->matches),
    _mm_cmpeq_epi8(_mm_loadu_si128(big_ptr + 7), env->matches)
 };
  // Since a match is 0xFF, which is -1, we can add our blocks together to get a
  // per-lane count of occurrences, except negative.
  __m128i const summed = _mm_add_epi8(_mm_add_epi8(_mm_add_epi8(results[0], results[1]),
                                                   _mm_add_epi8(results[2], results[3])),
                                      _mm_add_epi8(_mm_add_epi8(results[4], results[5]),
                                                   _mm_add_epi8(results[6], results[7])));
  // By taking the sum of absolute differences with 0x00 in all lanes, we end up
  // with two 64-bit sums, counting the number of occurences in the 'left' 8
  // lanes and the 'right' 8 lanes respectively. We then accumulate those.
  env->counts = _mm_add_epi64(env->counts, _mm_sad_epu8(summed, _mm_setzero_si128()));
}
#else
// Fallback implementation.

// Fill every 8-byte 'lane' with the same value.
__attribute__((pure, leaf))
static inline uint64_t broadcast(uint8_t const byte) {
  return byte * 0x0101010101010101ULL;
}

struct count_eq_env {
  uint64_t matches;
  uint64_t mask;
  size_t count;
};

__attribute__((pure))
static inline count_eq_env setup_env (uint8_t const byte) {
  return (count_eq_env){ .matches = broadcast(byte), 
                         .mask = broadcast(0x7F), 
                         .count = 0 };
}

__attribute__((pure, leaf))
static inline size_t retrieve_block_size (count_eq_env const* env) {
  // This suppresses the 'unused' warning, without enforcing reading from env.
  // See: https://stackoverflow.com/a/4851173/2629787
  (void)(sizeof((env), 0));
  return 64;
}

__attribute__((leaf))
static inline size_t retrieve_count (count_eq_env const* env) {
  return env->count;
}

// Load a 64-bit word, then set every byte which matches to 0x80, while setting
// the others to 0x00.
__attribute__((leaf))
static inline uint64_t load_and_set (uint64_t const* const big_ptr,
                                     size_t const i,
                                     uint64_t const matches,
                                     uint64_t const mask) {
  uint64_t const input = big_ptr[i] ^ matches;
  uint64_t const tmp = (input & mask) + mask;
  return (~(tmp | input | mask) >> i);
}

// We use the method described in "Bit Twiddling Hacks".
// Source: https://graphics.stanford.edu/~seander/bithacks.html#ZeroInWord
static inline void count_block (uint8_t const* const src,
                                count_eq_env* env) {
  uint64_t const* const big_ptr = (uint64_t const* const)src;
  uint64_t result = 0;
  // Manual 8x loop unroll
  result |= load_and_set(big_ptr, 0, env->matches, env->mask);
  result |= load_and_set(big_ptr, 1, env->matches, env->mask);
  result |= load_and_set(big_ptr, 2, env->matches, env->mask);
  result |= load_and_set(big_ptr, 3, env->matches, env->mask);
  result |= load_and_set(big_ptr, 4, env->matches, env->mask);
  result |= load_and_set(big_ptr, 5, env->matches, env->mask);
  result |= load_and_set(big_ptr, 6, env->matches, env->mask);
  result |= load_and_set(big_ptr, 7, env->matches, env->mask);
  env->count += __builtin_popcountll(result);
}
#endif

// Function implementation (fairly generic).
size_t diablo_count_eq(uint8_t const* const src,
                       size_t const off,
                       size_t const len,
                       uint8_t const byte) {
  size_t count = 0;
  count_eq_env env = setup_env(byte);
  size_t const block_size = retrieve_block_size(&env);
  size_t const big_strides = len / block_size;
  size_t const small_strides = len % block_size;
  uint8_t const* ptr = (uint8_t const*)&(src[off]);
  if (big_strides != 0) {
    for (size_t i = 0; i < big_strides; i++) {
      count_block(ptr, &env);
      ptr += block_size;
    }
    count += retrieve_count(&env); 
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
