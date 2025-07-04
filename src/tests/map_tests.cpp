
#include <iostream>

#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include "../utils/utils.h"

#define MAP_DEBUG 1
#include "../mtl_map/mtl_map.ipp"

#include <unordered_map>

#define ENTRIES_N (10)

bool map_test_multi(){
    // Init map
    auto test_map = mtl_map<uint8_t, uint8_t>();
    test_assert(test_map.init());
    
    // Init key-value buffers
    bucket * entries = new bucket[ENTRIES_N];
    key * keys = new key[ENTRIES_N];
    
    for (size_t idx = 0; idx < ENTRIES_N; idx++) {
        key * k = (key *) &entries[idx];
        auto v = k++;
        *k = idx;
        *v = idx * 2;
        keys[idx] = idx;
    }
    
    // Test multi insert
    test_assert(test_map.insert_multi(entries, &entries[ENTRIES_N]));
    
    // Test multi lookup
    value * out_values = new value[ENTRIES_N];
    test_assert(test_map.lookup_multi(keys, &keys[ENTRIES_N], out_values));
    
    // Test key-value match
    for (size_t i = 0; i < ENTRIES_N; i++){
        key * k = (key *) &entries[i];
        auto v = k++;
        test_assert(out_values[i] == *v);
    }
    
    delete [] entries;
    delete [] keys;
    delete [] out_values;
    
    return true;
}

bool map_test_single() {
    auto test_map = mtl_map<uint8_t, uint8_t>();
    test_assert(test_map.init());
    for (size_t i = 0; i < ENTRIES_N; i++) {
        test_assert(test_map.insert(i+1, i+1))
        test_assert(test_map.lookup(i+1) == i+1)
    }
    return true;
}

int main(int argc, const char * argv[]) {
    test_run(map_test_multi);
    test_run(map_test_single);
}

