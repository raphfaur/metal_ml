
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

#include <unordered_map>

#define ENTRIES_N (100000)

bool map_test_multi(){
    auto test_map = fox_map();
    test_assert(test_map.init());
    
    bucket * entries = new bucket[ENTRIES_N];
    key * keys = new key[ENTRIES_N];
    
    for (size_t idx = 0; idx < ENTRIES_N; idx++) {
        key * k = (key *) &entries[idx];
        auto v = k++;
        *k = idx;
        *v = idx + 1;
        keys[idx] = idx;
    }
    
    debug_memory((uint8_t *) entries, 10);
    debug_memory((uint8_t * ) keys, 10);
    test_map.insert_multi(entries, &entries[ENTRIES_N]);
    test_map.lookup_multi(keys, &keys[ENTRIES_N]);
    
    
    std::unordered_map<uint16_t, uint16_t> other_map;
    std::cout << "Starting insertion" << std::endl;
    for (size_t idx = 0; idx < ENTRIES_N; idx++) {
        key * k = (key *) &entries[idx];
        auto v = k++;
        other_map.insert({*k, *v});
    }
    std::cout << "Done insertion" << std::endl;
    
    return true;
}

bool map_test_single() {
    auto test_map = fox_map();
    test_assert(test_map.init());
    for (size_t i = 0; i < 1000; i++) {
        test_assert(test_map.insert(i+1, i+1))
        debug(i);
    }
    
    return true;
}

int main(int argc, const char * argv[]) {
    map_test_multi();
}

