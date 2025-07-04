#include <random>
#include <unordered_map>

#include "metal-ml.h"
#include "../src/utils/utils.h"

template <typename K, typename V, size_t S> bool map_test_multi() {
    // Init map
    mtl_map<K, V, S> test_map;
    using key = mtl_map<K, V, S>::key;
    using value = mtl_map<K, V, S>::value;

    test_assert(test_map.init());

    // Init key-value buffers
    pair<key, value> *entries = new pair<key, value>[S];
    key *keys = new key[S];

    // Key shuffling
    std::vector<V> keys_source;
    for (int i = 0; i < S; i++) {
        keys_source.push_back(i);
    }
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(keys_source.begin(), keys_source.end(),
                 std::default_random_engine(seed));

    // Store key / value
    for (size_t idx = 0; idx < S; idx++) {
        entries[idx].inner.k = keys_source[idx];
        entries[idx].inner.v = idx + 1;
        keys[idx] = keys_source[idx];
    }

    // Test multi insert
    test_debug("Inserting");
    test_assert(test_map.insert_multi(entries, &entries[S]));
    test_debug("Inserted");

    // Test multi lookup
    value *out_values = new value[S];
    test_assert(test_map.lookup_multi(keys, &keys[S], out_values));

    // Test key-value match
    for (size_t i = 0; i < S; i++) {
        test_assert(out_values[i] == i + 1);
    }

    delete[] entries;
    delete[] keys;
    delete[] out_values;

    return true;
}

template <typename K, typename V, size_t S> bool map_test_single() {
    auto test_map = mtl_map<K, V, S>();
    using key = mtl_map<K, V, S>::key;
    using value = mtl_map<K, V, S>::value;
    test_assert(test_map.init());
    for (int i = 0; i < S; i++) {
        test_assert(test_map.insert(pair<key, value>(i + 1, i + 1)));
        test_assert(test_map.lookup(i + 1) == i + 1)
    }
    return true;
}

int main(int argc, const char *argv[]) {
    test_run_must_pass((map_test_multi<uint16_t, uint16_t, 65000>));
    test_run_must_pass((map_test_multi<uint16_t, uint8_t, 65000>));
    test_run_must_fail((map_test_multi<uint16_t, uint16_t, 70000>));
    test_run_must_pass((map_test_multi<uint32_t, uint32_t, 10000000>));
    test_run_must_pass((map_test_multi<uint32_t, uint32_t, 1>));
    test_run_must_pass((map_test_multi<uint32_t, uint32_t, 10>));
    test_run_must_pass((map_test_single<uint16_t, uint16_t, 1000>));
}
