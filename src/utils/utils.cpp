#include "utils.h"

void debug_memory(uint8_t *region, size_t length) {
    debug_no_line("[ ");
    for (size_t i = 0; i < length; i++) {
        std::cout << std::hex << (int)region[i] << " ";
    }
    std::cout << "]" << std::endl;
}
