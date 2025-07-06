#include <chrono>
#include <random>
#include <unordered_map>

#include "../src/utils/utils.h"
#include "metal-ml.h"

std::shared_ptr<MetalDevice> MTL_DEVICE;




template <typename K, typename V, size_t S> void cpu_bench() {
    using key = mtl_map<K, V>::key;
    using value = mtl_map<K, V>::value;
    pair<key, value> *entries = new pair<key, value>[S];
    key *keys = new key[S];

    std::vector<K> keys_source;
    for (int i = 0; i < S; i++) {
        keys_source.push_back(i + 1);
    }

    uint64_t seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(keys_source.begin(), keys_source.end(),
                 std::default_random_engine(seed));

    for (size_t idx = 0; idx < S; idx++) {
        entries[idx].inner.k = keys_source[idx];
        entries[idx].inner.v = rand();
        keys[idx] = keys_source[idx];
    }

    volatile value *out_values = new value[S];

    benchmark_init();
    
    register_config(unordered_map_insert);
    register_config(unordered_map_lookup);
    register_config(mtl_map_insert);
    register_config(mtl_map_lookup)

    // CPU
    std::unordered_map<K, V> other_map;

    time_this_n(unordered_map_insert,
        INIT_BLOCK(
                   other_map.clear();
                   for (size_t idx = 0; idx < S; idx++) {
                       out_values[idx] = other_map[keys[idx]];
                   }
        ),
        {
        for (size_t idx = 0; idx < S; idx++) {
            other_map.insert({entries[idx].inner.k, entries[idx].inner.v});
        }
    }, 10);
    
    time_this_n(unordered_map_lookup,
        INIT_BLOCK(
                   other_map.clear();
        ),
        {
        for (size_t idx = 0; idx < S; idx++) {
            out_values[idx] = other_map[keys[idx]];
        }
    }, 10);

    // GPU
    time_this_n(
        mtl_map_insert,
        INIT_BLOCK(
                   mtl_map<K, V> test_map(MTL_DEVICE);
                   test_map.reserve(S);
                   test_map.init();
        ),
        {
            test_map.insert_multi(entries, &entries[S]);
        },
        10);
    
    std::memset((void*)out_values, 0, sizeof(value) * S);
    // GPU
    time_this_n(
        mtl_map_lookup,
        INIT_BLOCK(
                   mtl_map<K, V> test_map(MTL_DEVICE);
                   test_map.reserve(S);
                   test_map.init();
                   test_map.insert_multi(entries, &entries[S]);
        ),
        {
            test_map.lookup_multi(keys, &keys[S], (value *)out_values);
        },
        10);
    for (size_t i = 0; i < S; i++) {
        if (out_values[i] != entries[i].inner.v) {
            debug_error("Benchmark invalid");
        }
    }
    
    bench_reset();
}

int main(int argc, const char *argv[]) {
    MTL_DEVICE = std::make_shared<MetalDevice>();
    MTL_DEVICE->init_lib();
    bench_run((cpu_bench<uint16_t, uint16_t, 65536>()));
    bench_run((cpu_bench<uint32_t, uint32_t, 5000000>()));
}
