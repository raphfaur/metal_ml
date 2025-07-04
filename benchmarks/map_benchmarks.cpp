#include <chrono>
#include <random>
#include <unordered_map>

#include "metal-ml.h"
#include "../src/utils/utils.h"

template <typename K, typename V, size_t S> void cpu_bench() {
    using key = mtl_map<K, V, S>::key;
    using value = mtl_map<K, V, S>::value;
    pair<key, value> *entries = new pair<key, value>[S];
    key *keys = new key[S];

    std::vector<V> keys_source;
    for (int i = 0; i < S; i++) {
        keys_source.push_back(i);
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

    // CPU BENCH
    std::unordered_map<uint16_t, uint16_t> other_map;

    debug("Starting CPU insertion");
    auto cpu_start = std::chrono::system_clock::now();
    for (size_t idx = 0; idx < S; idx++) {
        other_map.insert({entries[idx].inner.k, entries[idx].inner.v});
    }
    for (size_t idx = 0; idx < S; idx++) {
        out_values[idx] = other_map[keys[idx]];
    }

    auto cpu_end = std::chrono::system_clock::now();
    debug("Done with CPU insertion");
    auto elapsed_cpu = std::chrono::duration_cast<std::chrono::milliseconds>(
        cpu_end - cpu_start);
    debug("CPU : {} ms", elapsed_cpu);

    // GPU BENCH
    mtl_map<K, V, S> test_map;
    test_map.init();
    debug("Starting GPU insertion");
    auto gpu_start = std::chrono::system_clock::now();
    test_map.insert_multi(entries, &entries[S]);
    test_map.lookup_multi(keys, &keys[S], (value *)out_values);
    auto gpu_end = std::chrono::system_clock::now();
    debug("Done GPU insertion");
    auto elapsed_gpu = std::chrono::duration_cast<std::chrono::milliseconds>(
        gpu_end - gpu_start);
    debug("GPU : {} ms", elapsed_gpu);
}

int main(int argc, const char *argv[]) {
    cpu_bench<uint16_t, uint16_t, 65000>();
    cpu_bench<uint32_t, uint32_t, 10000000>();
}
