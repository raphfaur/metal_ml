
#include <iostream>

#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include "test_utils.hpp"

#define MAP_DEBUG 1
#include "../fox_map/fox_map.hpp"

bool map_test() {
    auto test_map = fox_map();
    test_assert(test_map.init());
    for (size_t i = 0; i < 1000; i++) {
        test_assert(test_map.insert(i+1, i+1))
        debug(i);
    }
    
    return true;
}

int main(int argc, const char * argv[]) {
    map_test();
}

