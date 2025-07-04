
#include <iostream>

#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include "../utils/utils.h"
#include "../mtl_map/mtl_map.ipp"

#include <unordered_map>

#define ENTRIES_N (65000)

bool map_test_multi(){
    // Init map
    mtl_map<uint16_t, uint16_t, ENTRIES_N> test_map;
    using key = mtl_map<uint16_t, uint16_t, ENTRIES_N>::key;
    using value = mtl_map<uint16_t, uint16_t, ENTRIES_N>::value;
    
    test_assert(test_map.init());
    
    // Init key-value buffers
    pair<key, value> * entries = new pair<key, value>[ENTRIES_N];
    key * keys = new key[ENTRIES_N];
    
    for (size_t idx = 0; idx < ENTRIES_N; idx++) {
        entries[idx].inner.k = idx;
        entries[idx].inner.v = idx + 1;
        keys[idx] = idx;
    }
//    debug_memory((uint8_t *) entries, ENTRIES_N * 4);
    // Test multi insert
    test_debug("Inserting");
    test_assert(test_map.insert_multi(entries, &entries[ENTRIES_N]));
    test_debug("Inserted");
    // Test multi lookup
    value * out_values = new value[ENTRIES_N];
    test_assert(test_map.lookup_multi(keys, &keys[ENTRIES_N], out_values));
//    debug_memory((uint8_t *) out_values, ENTRIES_N);
    
    // Test key-value match
    for (size_t i = 0; i < ENTRIES_N; i++){
        test_assert(out_values[i] == i+1);
    }
    
    delete [] entries;
    delete [] keys;
    delete [] out_values;
    
    return true;
}

bool map_test_single() {
    auto test_map = mtl_map<uint16_t, uint16_t, ENTRIES_N>();
    using key = mtl_map<uint16_t, uint16_t, ENTRIES_N>::key;
    using value = mtl_map<uint16_t, uint16_t, ENTRIES_N>::value;
    test_assert(test_map.init());
    for (int i = 0; i < ENTRIES_N; i++) {
        test_assert(test_map.insert(pair<key, value>(i+1, i+1)));
        test_assert(test_map.lookup(i+1) == i+1)
    }
    return true;
}

int main(int argc, const char * argv[]) {
    test_run(map_test_multi);
//    test_run(map_test_single);
}

