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

// Retrieve the count.
static inline size_t retrieve_count (count_eq_env const* env);

// Load and count a 64-byte block.
static inline void count_block (uint8_t const* const src,
                                count_eq_env* env);

// Fill every 8-byte 'lane' with the same value.
static inline uint64_t broadcast(uint8_t const byte) {
  return byte * 0x0101010101010101ULL;
}

// Fallback implementation.

struct count_eq_env {
  uint64_t matches;
  uint64_t mask;
  size_t count;
};

static inline count_eq_env setup_env (uint8_t const byte) {
  return (count_eq_env){ .matches = broadcast(byte), .mask = broadcast(0x7F), 
                         .count = 0 };
}

static inline size_t retrieve_count (count_eq_env const* env) {
  return env->count;
}

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

// Load and count a 64-byte block.
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

// Function implementation (fairly generic).
size_t diablo_count_eq(uint8_t const* const src,
                       size_t const off,
                       size_t const len,
                       uint8_t const byte) {
  size_t count = 0; 
  // We aim to step 64 bytes at a time.
  size_t const big_strides = len / 64;
  size_t const small_strides = len % 64;
  uint8_t const* ptr = (uint8_t const*)&(src[off]);
  if (big_strides != 0) {
    // We use the method described in "Bit Twiddling Hacks".
    // Source: https://graphics.stanford.edu/~seander/bithacks.html#ZeroInWord
    count_eq_env env = setup_env(byte);
    for (size_t i = 0; i < big_strides; i++) {
      count_block(ptr, &env);
      ptr += 64;
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
