#pragma once
#include <iostream>

#define debug(expr) \
std::cout << __FILE__ << ":" << __LINE__ << " " << expr << std::endl;

#define test_assert(expr) \
if (!(expr)) { \
debug( #expr " assert failed") \
exit(1);\
}

void debug_memory(uint8_t * region, size_t length) {
    for (size_t i = 0; i < length; i++) {
        std::cout << std::hex << (int)region[i] << " ";
    }
    std::cout << std::endl;
}
