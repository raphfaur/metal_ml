//
//  fox_map.metal
//  metal-project
//
//  Created by Raphaël Faure on 25/06/2025.
//
#include <metal_stdlib>
#include "config.hpp"
#include "../common/types.hpp"

#define DECLARE_TEMPLATE_LOOKUP(K, V) \
template [[host_name("map_lookup_" #K "_" #V)]] kernel void multi_map_lookup( \
device K * keys, \
device V * values, \
device pair<K,V> * map_memory, \
device uint64_t * map_size [[buffer(2)]], \
uint sid, \
uint tid, \
uint tgid, \
uint tg_size_simd, \
uint simd_size);

#define DECLARE_TEMPLATE_INSERT(K,V) \
template [[host_name("map_insert_" #K "_" #V )]] kernel void multi_map_insert( \
device pair<K, V> * entries , \
device pair<K, V> * map_memory, \
device uint64_t * map_size [[buffer(2)]], \
uint sid [[simdgroup_index_in_threadgroup]], \
uint tid [[thread_index_in_simdgroup]], \
uint tgid [[threadgroup_position_in_grid]], \
uint tg_size_simd [[simdgroups_per_threadgroup]], \
uint simd_size [[threads_per_simdgroup]]);

using namespace metal;

template<typename K>
size_t hash(K key) {
    return key;
}

constant int simd_groups_per_threadgroups = 32;

template <typename K, typename V>
kernel void multi_map_insert(device pair<K, V> * entries [[buffer(0)]],
                             device pair<K, V> * map_memory [[buffer(1)]],
                             device uint64_t * map_size [[buffer(2)]],
                             uint sid [[simdgroup_index_in_threadgroup]],
                             uint tid [[thread_index_in_simdgroup]],
                             uint tgid [[threadgroup_position_in_grid]],
                             uint tg_size_simd [[threads_per_threadgroup]],
                             uint simd_size [[threads_per_simdgroup]]){
    
    // One simd group per entry (k-v grouped in bucket to ensure cache locality)
    
    uint entry_id = tgid * simd_groups_per_threadgroups + sid;
    
    //Sentinel
    pair<K, V> sentinel;
    
    // Get key and value from bucket
    pair<K, V> assigned_bucket = entries[entry_id];
    K k = assigned_bucket.inner.k;
    V v = assigned_bucket.inner.v;
    
    size_t h = hash(k);
    
    ushort SUCCESS = 0;
    
    size_t i = 0;
    while (true) {
        size_t index = ( h + i * simd_size + tid ) % *map_size;
        bool cond = false;
        
        // Check type size
        if constexpr( sizeof(pair<K, V>) > 4 ) {
            using atomic_content = device atomic<K> *;
            atomic_content address = (atomic_content) (map_memory + index);
            K content = atomic_load_explicit( address, memory_order_relaxed);
            cond = (content == sentinel.inner.k);
        } else {
            using atomic_content = device atomic<typename pair<K, V>::raw_t> *;
            atomic_content address = (atomic_content) (map_memory + index);
            typename pair<K, V>::raw_t content = atomic_load_explicit( address, memory_order_relaxed);
            pair<K, V> content_p;
            content_p.raw = content;
            cond = (content_p.inner.k == sentinel.inner.k);
        }
        
        simd_vote vote = simd_ballot(cond);
        if (!vote.any()) {
            i += 1;
            continue;
        } else {
            uint64_t vote_v = static_cast<uint64_t>(vote);
            
            uint first_empty = ctz(vote_v);
            size_t index_first_empty = ( h + i * simd_size + first_empty ) % *map_size;
            
            // Build bucket
            pair<K, V> b (k,v);
            
            // Compare and swap
            if (tid == first_empty) {
                if constexpr( sizeof(pair<K, V>) > 4 ) {
                    using atomic_content = device atomic<K> *;
                    atomic_content address = (atomic_content) (map_memory + index_first_empty);
                    // Check key
                    SUCCESS = atomic_compare_exchange_weak_explicit(address, &(sentinel.inner.k), b.inner.k, memory_order_relaxed, memory_order_relaxed) ? 1 : 0;
                    // Flush value
                    if (SUCCESS) {
                        *(map_memory + index_first_empty) = b;
                    }
                } else {
                    using atomic_content = device atomic<typename pair<K, V>::raw_t> *;
                    atomic_content address = (atomic_content) (map_memory + index_first_empty);
                    SUCCESS = atomic_compare_exchange_weak_explicit(address, &(sentinel.raw), b.raw, memory_order_relaxed, memory_order_relaxed) ? 1 : 0;
                }
                
            }
            if (simd_broadcast(SUCCESS, first_empty)) break;
        }
    }
    
}


template <typename K, typename V>
kernel void multi_map_lookup(device K * keys [[buffer(0)]],
                             device V * values [[buffer(1)]],
                             device pair<K,V> * map_memory [[buffer(2)]],
                             device uint64_t * map_size [[buffer(3)]],
                             uint sid [[simdgroup_index_in_threadgroup]],
                             uint tid [[thread_index_in_simdgroup]],
                             uint tgid [[threadgroup_position_in_grid]],
                             uint tg_size_simd [[simdgroups_per_threadgroup]],
                             uint simd_size [[threads_per_simdgroup]]){
    
    // One simd group per entry (k-v grouped in bucket to ensure cache locality)
    uint entry_id = tgid * simd_groups_per_threadgroups + sid;
    
    // Get key and value from bucket
    K assined_key = keys[entry_id];
    
    size_t h = hash(assined_key);
    
    size_t i = 0;
    while (true) {
        bool cond = false;
        size_t index = ( h + i * simd_size + tid ) % *map_size;
        V value;
        if constexpr( sizeof(pair<K, V>) > 4 ) {
            using atomic_content = device atomic<K> *;
            atomic_content address = (atomic_content) (map_memory + index);
            K content = atomic_load_explicit( address, memory_order_relaxed);
            cond = (content == assined_key || content == SENTINEL);
            value = (map_memory + index)->inner.v;
        } else {
            using atomic_content = device atomic<typename pair<K, V>::raw_t> *;
            atomic_content address = (atomic_content) (map_memory + index);
            typename pair<K, V>::raw_t content = atomic_load_explicit(address , memory_order_relaxed);
            pair<K, V> content_p;
            content_p.raw = content;
            cond = ( content_p.inner.k == assined_key || content_p.inner.k == SENTINEL );
            value = content_p.inner.v;
        }
        simd_vote vote = simd_ballot(cond);
        if (!vote.any()) {
            i += 1;
            continue;;
        } else {
            uint64_t vote_v = static_cast<uint64_t>(vote);
            uint first_empty = ctz(vote_v);
            if (first_empty == tid) values[entry_id] = value;
            break;
        }
    }
    
}

DECLARE_TEMPLATE_LOOKUP(uint8_t, uint8_t)
DECLARE_TEMPLATE_LOOKUP(uint16_t, uint16_t)
DECLARE_TEMPLATE_LOOKUP(uint16_t, uint8_t)
DECLARE_TEMPLATE_LOOKUP(uint32_t, uint32_t)

DECLARE_TEMPLATE_INSERT(uint8_t, uint8_t)
DECLARE_TEMPLATE_INSERT(uint16_t, uint16_t)
DECLARE_TEMPLATE_INSERT(uint16_t, uint8_t)
DECLARE_TEMPLATE_INSERT(uint32_t, uint32_t)

