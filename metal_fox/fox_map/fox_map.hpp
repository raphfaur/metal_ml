#pragma once
#include "../metal_config.hpp"
#include "fox_map_config.hpp"


class fox_map {
    MTL::Device * _mtl_device = nullptr;
    MTL::CommandQueue * _map_cmd_q = nullptr;
    MTL::ComputePipelineState * _pipeline_s = nullptr;
    MTL::Library * _lib = nullptr;
    
    // Buffers
    MTL::Buffer * _main_buffer = nullptr;
    MTL::Buffer * _insert_buffer = nullptr;
    MTL::Buffer * _lookup_in_buffer = nullptr;
    MTL::Buffer * _lookup_out_buffer = nullptr;
    
    bool _kernel_init();
    bool _init();
public:
    bool init();
    bool insert(key k, value v);
    bool insert_multi(bucket * begin, bucket * end);
    bool lookup_multi(key * begin, key * end);
    value lookup(key k);
    
    ~fox_map() = default;
};
