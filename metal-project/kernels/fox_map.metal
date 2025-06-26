//
//  fox_map.metal
//  metal-project
//
//  Created by Raphaël Faure on 25/06/2025.
//

#include <metal_stdlib>
#include "../fox_map/fox_map_config.hpp"
using namespace metal;

// One bucket is a 8 bytes bucket divided in a [ key value ] layout, 4 bytes each
size_t hash(key k) {
    return k % MAP_ENTRY_N;
}

kernel void map_insert(device key * entry [[buffer(0)]],
                       device _atomic<bucket> * map_memory [[buffer(1)]],
                       uint tid [[ thread_index_in_simdgroup ]],
                       uint simd_size [[threads_per_simdgroup]]){
    key k = entry[0];
    value v = entry[1];
    size_t h = hash(k);
    
    device _atomic<bucket> * base_address = map_memory + h;
    
    uint offset = tid < MAP_ENTRY_N ? tid : 0;
    
    threadgroup bool SUCCESS = false;
    
    size_t i = 0;
    while (true) {
        auto content = atomic_load_explicit(map_memory + h + i * simd_size + offset , memory_order_relaxed);
        bool cond = content == SENTINEL;
        simd_vote vote = simd_ballot(cond);
        if (!vote.any()) {
            i += 1;
            continue;;
        } else {
            uint64_t vote_v = static_cast<uint64_t>(vote);
            uint first_empty = ctz(vote_v);
            
            // Build bucket
            bucket b = v << 16 | k ;
            
            // Compare and swap
            bucket desired = b;
            bucket sentinel {SENTINEL};
            if (tid == first_empty) {
                SUCCESS = atomic_compare_exchange_weak_explicit(base_address + i * simd_size + first_empty, &sentinel, desired, memory_order_relaxed, memory_order_relaxed);
            }
            if (SUCCESS) break;
        }
    }
}

kernel void map_lookup (device key * entry [[buffer(0)]],
                       device _atomic<bucket> * map_memory [[buffer(2)]],
                        device value * output [[buffer(1)]],
                       uint tid [[ thread_index_in_simdgroup ]],
                        uint simd_size [[threads_per_simdgroup]]){
    key k = *entry;
    
    size_t h = hash(k);
    
    device _atomic<bucket> * base_address = map_memory + h;
    
    uint offset = tid < MAP_ENTRY_N ? tid : 0;
    
    size_t i = 0;
    while (true) {
        auto content = atomic_load_explicit(base_address + i * simd_size + offset , memory_order_relaxed);
        bool cond = (uint16_t) content == k || content == SENTINEL;
        simd_vote vote = simd_ballot(cond);
        if (!vote.any()) {
            i += 1;
            continue;;
        } else {
            uint64_t vote_v = static_cast<uint64_t>(vote);
            uint first_empty = ctz(vote_v);
            if (first_empty == tid) *output = (uint16_t) (content >> 16);
            break;
        }
    }
    
}
