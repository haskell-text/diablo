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
#ifndef DIABLO_H
#define DIABLO_H

#include <stdint.h>
#include <stdlib.h>

#define IS_BIG_ENDIAN (!*(unsigned char*)&(uint16_t){1})

// Counting
size_t count_eq(uint8_t const* const src,
                size_t const off,
                size_t const len,
                uint8_t const byte);

#endif /* DIABLO_H */
