#pragma once

using key = uint16_t;
using value = uint16_t;
using bucket = uint32_t;

#define MAP_ENTRY_N (10)
#define MAP_SIZE (MAP_ENTRY_N * sizeof(bucket))

#define SENTINEL (0x0)

#define MAP_DEBUG 0

