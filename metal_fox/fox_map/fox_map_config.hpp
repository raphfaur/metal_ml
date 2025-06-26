#pragma once

using key = uint16_t;
using value = uint16_t;
using bucket = uint32_t;

#define MAP_SIZE (0x100000)
#define MAP_ENTRY_N (MAP_SIZE / sizeof(bucket))
#define SENTINEL (0x0)

#define MAP_DEBUG 1
