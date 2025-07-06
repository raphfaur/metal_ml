#pragma once
#include "../common/cpp_types.hpp"
#include "../utils/metal_config.hpp"
#include "../utils/utils.h"
#include "config.hpp"
#include "mtl_map.hpp"
#include <chrono>
#include <format>
#include <iostream>

const char *fox_map_kernels[_mtl_map_kernel::_KER_MAX] = {"map_insert",
                                                          "map_lookup"};

template <typename K, typename V, typename Enable>
mtl_map<K, V, Enable>::mtl_map(std::shared_ptr<MetalDevice> device) {
    _mtl_device = device;
}

template <typename K, typename V, typename Enable>
bool mtl_map<K, V, Enable>::_init() {
    std::vector<std::string> templated_kernels;
    for (size_t i = 0; i < _mtl_map_kernel::_KER_MAX; i++) {
        templated_kernels.push_back(
            std::format("{}_{}_{}", std::string(fox_map_kernels[i]),
                        type_name<K>::name, type_name<V>::name));
    }

    // Allocate main memory for the hash table
    require_exit(
        _mtl_device->request_buffer(_mtl_map_buffer::BUF_MAIN, _memory_map_size),
        false);
    memset(_mtl_device->get_buffer(_mtl_map_buffer::BUF_MAIN)->contents(),
           SENTINEL, _memory_map_size);

    // Map size config
    require_exit(
        _mtl_device->request_buffer(_mtl_map_buffer::BUF_SIZE, sizeof(uint64_t)),
        false);
    uint64_t *mapped_size = static_cast<uint64_t *>(
        _mtl_device->get_buffer(_mtl_map_buffer::BUF_SIZE)->contents());
    *mapped_size = _size;

    _mtl_device->load_kernels(templated_kernels);

    return true;
err:
    return false;
}

template <typename K, typename V, typename Enable>
bool mtl_map<K, V, Enable>::init() {
    return this->_init();
}

template <typename K, typename V, typename Enable>
bool mtl_map<K, V, Enable>::insert(bucket b) {
    if ((b.inner.k == 0 && b.inner.v == 0))
        return false;
    return insert_multi(&b, &b + 1);
err:
    return false;
}

template <typename K, typename V, typename Enable>
mtl_map<K, V, Enable>::value mtl_map<K, V, Enable>::lookup(key k) {
    value out = 0;
    require(lookup_multi(&k, &k + 1, &out), err);
    return out;
err:
    return false;
}

template <typename K, typename V, typename Enable>
bool mtl_map<K, V, Enable>::insert_multi(bucket *begin, bucket *end) {
    MTL::CommandBuffer *cmd_buf = _mtl_device->get_command_buffer();
    auto encoder = cmd_buf->computeCommandEncoder();

    auto multi_ins_pipeline =
        _mtl_device->get_pipeline(_mtl_map_kernel::KER_MULTI_INSERT);

    encoder->setComputePipelineState(multi_ins_pipeline);

    // Compute entry count
    size_t entry_n = end - begin;

    // Set insert args
    require_exit(_mtl_device->request_buffer(_mtl_map_buffer::BUF_INSERT, begin,
                                            entry_n * sizeof(bucket)),
                 false);
    encoder->setBuffer(_mtl_device->get_buffer(_mtl_map_buffer::BUF_INSERT), 0,
                       0);
    // #if MAP_DEBUG
    //     debug_memory((uint8_t*)_mtl_device->get_buffer(_mtl_map_buffer::BUF_INSERT)->contents(),
    //     entry_n * sizeof(bucket));
    // #endif

    // Set main buffer
    encoder->setBuffer(_mtl_device->get_buffer(_mtl_map_buffer::BUF_MAIN), 0, 1);
    encoder->setBuffer(_mtl_device->get_buffer(_mtl_map_buffer::BUF_SIZE), 0, 2);
    // #if MAP_DEBUG
    //     debug_memory((uint8_t*)_mtl_device->get_buffer(_mtl_map_buffer::BUF_SIZE)->contents(),
    //     sizeof(uint64_t));
    // #endif

    auto threadGroupSize =
        MTL::Size(multi_ins_pipeline->maxTotalThreadsPerThreadgroup(), 1, 1);
    auto gridSize =
        MTL::Size(multi_ins_pipeline->threadExecutionWidth() * entry_n, 1, 1);
    encoder->dispatchThreads(gridSize, threadGroupSize);
    encoder->endEncoding();

    auto gpu_start = std::chrono::system_clock::now();
    cmd_buf->commit();
    cmd_buf->waitUntilCompleted();
    auto gpu_end = std::chrono::system_clock::now();
    debug("Done waiting");
    auto elapsed_gpu = std::chrono::duration_cast<std::chrono::milliseconds>(
        gpu_end - gpu_start);
    debug("Elapsed time : {}", elapsed_gpu);

#if MAP_DEBUG
    debug_memory((uint8_t *)_mtl_device->get_buffer(_mtl_map_buffer::BUF_MAIN)
                     ->contents(),
                 _size * sizeof(bucket));
#endif

    return true;

err:
    return false;
}

template <typename K, typename V, typename Enable>
bool mtl_map<K, V, Enable>::lookup_multi(key *begin, key *end, value *out) {
    MTL::CommandBuffer *cmd_buf = _mtl_device->get_command_buffer();
    auto encoder = cmd_buf->computeCommandEncoder();

    auto multi_lookup_pipeline =
        _mtl_device->get_pipeline(_mtl_map_kernel::KER_MULTI_LOOKUP);

    encoder->setComputePipelineState(multi_lookup_pipeline);

    // Compute key count
    size_t key_n = end - begin;

    // Set keys buffer
    require_exit(_mtl_device->request_buffer(_mtl_map_buffer::BUF_LOOKUP_IN,
                                            begin, key_n * sizeof(key)),
                 false);
    encoder->setBuffer(_mtl_device->get_buffer(_mtl_map_buffer::BUF_LOOKUP_IN),
                       0, 0);
    //    debug_memory((uint8_t *)
    //    _mtl_device->get_buffer(_mtl_map_buffer::BUF_LOOKUP_IN)->contents(),
    //    key_n * sizeof(key));

    // Set output buffer
    require_exit(_mtl_device->request_buffer(_mtl_map_buffer::BUF_LOOKUP_OUT,
                                            key_n * sizeof(value)),
                 false);
    encoder->setBuffer(_mtl_device->get_buffer(_mtl_map_buffer::BUF_LOOKUP_OUT),
                       0, 1);

    // Set main buffer
    encoder->setBuffer(_mtl_device->get_buffer(_mtl_map_buffer::BUF_MAIN), 0, 2);
    encoder->setBuffer(_mtl_device->get_buffer(_mtl_map_buffer::BUF_SIZE), 0, 3);

    auto threadGroupSize =
        MTL::Size(multi_lookup_pipeline->maxTotalThreadsPerThreadgroup(), 1, 1);
    auto gridSize =
        MTL::Size(multi_lookup_pipeline->threadExecutionWidth() * key_n, 1, 1);

    encoder->dispatchThreads(gridSize, threadGroupSize);
    encoder->endEncoding();

    cmd_buf->commit();
    cmd_buf->waitUntilCompleted();

    memcpy(out,
           _mtl_device->get_buffer(_mtl_map_buffer::BUF_LOOKUP_OUT)->contents(),
           key_n * sizeof(value));

#if MAP_DEBUG
    debug_memory(
        (uint8_t *)_mtl_device->get_buffer(_mtl_map_buffer::BUF_LOOKUP_OUT)
            ->contents(),
        10);
#endif

    return true;

err:
    return false;
}

template <typename K, typename V, typename Enable>
void mtl_map<K, V, Enable>::clear() {
    memset(_mtl_device->get_buffer(_mtl_map_buffer::BUF_MAIN)->contents(),
           SENTINEL, _memory_map_size);
}

template <typename K, typename V, typename Enable>
void mtl_map<K, V, Enable>::reserve(size_t size) {
    _size = MIN(size, pow(2, 8 * sizeof(key)));
    _memory_map_size = _size * sizeof(bucket);
}

