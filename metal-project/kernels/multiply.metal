//
//  File.metal
//  metal-project
//
//  Created by Raphaël Faure on 23/06/2025.
//

#include <metal_stdlib>
using namespace metal;



kernel void multiply(device float *a [[buffer(0)]],
                     device float *b [[buffer(1)]],
                     device float *out [[buffer(2)]],
                     uint position [[thread_position_in_grid]],
                     uint simd_index [[thread_index_in_simdgroup]])
{
    
    out[position] = a[position] * b[position];
}





#define DATA_N 256

kernel void minimum(device atomic_float * global_min [[buffer(1)]],
                    device float * data [[buffer(0)]],
                    threadgroup float * gdata [[threadgroup(0)]],
                    uint sid [[simdgroup_index_in_threadgroup]],
                    uint tid [[thread_index_in_threadgroup]],
                    uint simd_groups_per_threadgroup [[simdgroups_per_threadgroup]],
                    uint simd_size [[threads_per_simdgroup]],
                    uint tcount [[threads_per_threadgroup]],
                    uint gid [[thread_position_in_grid]]){
    
    float simd_min;
    
    // First value is taken from input
    float value = data[gid];
    
    // Threadgroup reduction
    for (uint active_threads_n = tcount; active_threads_n > simd_size; active_threads_n /= simd_size ) {
        // SIMD level reduction
        // Reduce SIMD if SIMD is active in threadgroup
        if (tid < active_threads_n){
            simd_min = simd_sum(value);
            gdata[sid] = simd_min;
            threadgroup_barrier(mem_flags::mem_threadgroup);
            value = gdata[tid];
        }
    }
    
    // Last reduction in threadgroup
    float local_sum = simd_sum(value);
    if (tid == 0) {
        atomic_fetch_add_explicit(global_min, local_sum, memory_order_relaxed);
    }
}
