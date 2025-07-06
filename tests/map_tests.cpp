#include "../src/utils/utils.h"
#include "metal-ml.h"
#include <memory>
#include <random>
#include <unordered_map>

std::shared_ptr<MetalDevice> MTL_DEVICE;

template <typename K, typename V, size_t S> bool map_test_multi() {
    // Init map
    mtl_map<K, V> test_map(MTL_DEVICE);
    test_map.reserve(S);
    using key = mtl_map<K, V>::key;
    using value = mtl_map<K, V>::value;

    test_assert(test_map.init());

    // Init key-value buffers
    pair<key, value> *entries = new pair<key, value>[S];
    key *keys = new key[S];

    // Key shuffling
    std::vector<K> keys_source;
    for (int i = 0; i < S; i++) {
        keys_source.push_back(i+1);
    }
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(keys_source.begin(), keys_source.end(),
                 std::default_random_engine(seed));

    // Store key / value
    for (size_t idx = 0; idx < S; idx++) {
        entries[idx].inner.k = keys_source[idx];
        entries[idx].inner.v = rand();
        keys[idx] = keys_source[idx];
    }

    // Test multi insert
    test_debug("Inserting");
    test_assert(test_map.insert_multi(entries,  &entries[S]));
    test_debug("Inserted");

    // Test multi lookup
    value *out_values = new value[S];
    test_assert(test_map.lookup_multi(keys, &keys[S], out_values));

    // Test key-value match
    for (size_t i = 0; i < S; i++) {
        test_assert(out_values[i] == entries[i].inner.v);
    }

    delete[] entries;
    delete[] keys;
    delete[] out_values;

    return true;
}

template <typename K, typename V, size_t S> bool map_test_single() {
    auto test_map = mtl_map<K, V>(MTL_DEVICE);
    test_map.reserve(S);
    using key = mtl_map<K, V>::key;
    using value = mtl_map<K, V>::value;
    test_assert(test_map.init());
    for (int i = 0; i < S; i++) {
        test_assert(test_map.insert(pair<key, value>(i + 1, i + 1)));
        test_assert(test_map.lookup(i + 1) == i + 1)
    }
    return true;
}

int main(int argc, const char *argv[]) {
    MTL_DEVICE = std::make_shared<MetalDevice>();
    MTL_DEVICE->init_lib();
    

    // Test small K,V
    test_run_must_pass((map_test_multi<uint8_t, uint8_t, 255>));
    test_run_must_pass((map_test_multi<uint8_t, uint16_t, 255>));
    
    test_run_n((map_test_multi<uint16_t, uint16_t, 65535>), 20);
    test_run_must_pass((map_test_multi<uint16_t, uint16_t, 65535>));
    test_run_must_pass((map_test_multi<uint16_t, uint16_t, 100>));
    
    test_run_must_pass((map_test_multi<uint16_t, uint8_t, 65535>));
    test_run_must_pass((map_test_multi<uint16_t, uint8_t, 65535>));
    test_run_must_pass((map_test_multi<uint16_t, uint8_t, 100>));
    
    test_run_n((map_test_multi<uint32_t, uint32_t, 100000>), 20);
    test_run_must_pass((map_test_multi<uint32_t, uint32_t, 1000000>));
    test_run_must_pass((map_test_multi<uint32_t, uint32_t, __UINT32_MAX__ / 100>));
    test_run_must_pass((map_test_multi<uint32_t, uint32_t, 1>));
    test_run_must_pass((map_test_multi<uint32_t, uint32_t, 10>));
    
    test_run_must_pass((map_test_single<uint16_t, uint16_t, 1000>));
}
