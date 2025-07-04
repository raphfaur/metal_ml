#pragma once
#include "../utils/metal_config.hpp"
#include "config.hpp"
#include <iostream>
#include <format>
#include "../utils/utils.h"
#include "mtl_map.hpp"
#include "../common/metal.ipp"

const char * fox_map_kernels[_mtl_map_kernel::_KER_MAX] = {
    "multi_map_insert",
    "multi_map_lookup"
};

template<typename K, typename V, typename Enable>
bool mtl_map<K, V, Enable>::_init(){
    std::vector<std::string> kernels(fox_map_kernels, fox_map_kernels + _mtl_map_kernel::_KER_MAX);
    
    // Allocate main memory for the hash table
    require_exit(_mtl_device.request_buffer(_mtl_map_buffer::BUF_MAIN, MAP_SIZE), false);
    memset(_mtl_device.get_buffer(_mtl_map_buffer::BUF_MAIN)->contents(), SENTINEL, MAP_SIZE);
    
    _mtl_device.load_kernels(kernels);
    
    return true;
err:
    return false;
}

template<typename K, typename V, typename Enable>
bool mtl_map<K, V, Enable>::init() {
    return this->_init();
}

template<typename K, typename V, typename Enable>
bool mtl_map<K, V, Enable>::insert(key k, value v){
    if ((k == 0 && v == 0) ) return false;
    std::pair<key, value> b (k,v);
//    key in_bucket[2];
//    in_bucket[0] = k;
//    in_bucket[1] = v;
    return insert_multi((bucket *) &b, (bucket *) &b + 1);
err:
    return false;
}

template<typename K, typename V, typename Enable>
value mtl_map<K, V, Enable>::lookup(key k){
    value out = 0;
    require(lookup_multi(&k, &k + 1, &out), err);
    return out;
err:
    return false;
}

template<typename K, typename V, typename Enable>
bool mtl_map<K, V, Enable>::insert_multi(bucket *begin, bucket *end) {
    MTL::CommandBuffer * cmd_buf = _mtl_device.get_command_buffer();
    auto encoder = cmd_buf->computeCommandEncoder();

    auto multi_ins_pipeline = _mtl_device.get_pipeline(_mtl_map_kernel::KER_MULTI_INSERT);
    
    encoder->setComputePipelineState(multi_ins_pipeline);
    
    // Compute entry count
    size_t entry_n =  end - begin;
    
    // Set insert args
    require_exit(_mtl_device.request_buffer(_mtl_map_buffer::BUF_INSERT, begin, entry_n * sizeof(bucket)), false);
    encoder->setBuffer(_mtl_device.get_buffer(_mtl_map_buffer::BUF_INSERT), 0, 0);
    
    // Set main buffer
    encoder->setBuffer(_mtl_device.get_buffer(_mtl_map_buffer::BUF_MAIN), 0, 1);
    
    auto threadGroupSize = MTL::Size(multi_ins_pipeline->maxTotalThreadsPerThreadgroup(), 1, 1);
    auto gridSize = MTL::Size(multi_ins_pipeline->threadExecutionWidth() * entry_n, 1, 1);
    encoder->dispatchThreads(gridSize, threadGroupSize);
    encoder->endEncoding();
    
    cmd_buf->commit();
    cmd_buf->waitUntilCompleted();
    
#if MAP_DEBUG
    debug_memory((uint8_t*)_mtl_device.get_buffer(_mtl_map_buffers::BUF_MAIN)->contents(), 100);
#endif
    
    return true;
    
err:
    return false;
}

template<typename K, typename V, typename Enable>
bool mtl_map<K, V, Enable>::lookup_multi(key *begin, key *end, value *out) {
    MTL::CommandBuffer * cmd_buf = _mtl_device.get_command_buffer();
    auto encoder = cmd_buf->computeCommandEncoder();
    
    auto multi_lookup_pipeline = _mtl_device.get_pipeline(_mtl_map_kernel::KER_MULTI_LOOKUP);
    
    encoder->setComputePipelineState(multi_lookup_pipeline);
    
    // Compute key count
    size_t key_n = end - begin;
    
    // Set keys buffer
    require_exit(_mtl_device.request_buffer(_mtl_map_buffer::BUF_LOOKUP_IN, begin, key_n * sizeof(key)), false);
    encoder->setBuffer(_mtl_device.get_buffer(_mtl_map_buffer::BUF_LOOKUP_IN), 0, 0);
    
    // Set output buffer
    require_exit(_mtl_device.request_buffer(_mtl_map_buffer::BUF_LOOKUP_OUT, key_n * sizeof(value)), false);
    encoder->setBuffer(_mtl_device.get_buffer(_mtl_map_buffer::BUF_LOOKUP_OUT), 0, 1);
    
    // Set main buffer
    encoder->setBuffer(_mtl_device.get_buffer(_mtl_map_buffer::BUF_MAIN), 0, 2);
    
    auto threadGroupSize = MTL::Size(multi_lookup_pipeline->maxTotalThreadsPerThreadgroup(), 1, 1);
    auto gridSize = MTL::Size(multi_lookup_pipeline->threadExecutionWidth() * key_n, 1, 1);
    
    encoder->dispatchThreads(gridSize, threadGroupSize);
    encoder->endEncoding();
    
    cmd_buf->commit();
    debug("Waiting kernel completion");
    cmd_buf->waitUntilCompleted();
    debug("Kernel completed");
    
    memcpy(out, _mtl_device.get_buffer(_mtl_map_buffer::BUF_LOOKUP_OUT)->contents(), key_n * sizeof(value));
    
#if MAP_DEBUG
    debug_memory((uint8_t *) _mtl_device.get_buffer(_mtl_map_buffers::BUF_LOOKUP_OUT)->contents(), 10);
#endif
    
    return true;
    
err:
    return false;
}




