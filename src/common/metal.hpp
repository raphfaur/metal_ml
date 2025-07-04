#pragma once
#include "../utils/metal_config.hpp"

using __MTL_MAX_ATOMIC_TYPE = uint64_t;

template<typename K, typename V>
union pair {
    struct __attribute__((packed)) __pair {
        K k;
        V v;
    };
    __MTL_MAX_ATOMIC_TYPE raw;
};

template<typename K, typename V>
struct __is_valid_key_value_size {
    static const bool value = ( sizeof(pair<K,V>) <= sizeof(__MTL_MAX_ATOMIC_TYPE) );
};

class MetalDevice {
    MTL::Device * _mtl_device = nullptr;
    MTL::CommandQueue * _map_cmd_q = nullptr;
    MTL::Library * _lib = nullptr;
    
    // Internal but exposed
    std::vector<MTL::ComputePipelineState *> _pipelines;
    std::unordered_map<uint8_t,MTL::Buffer*> _buffers;
    
    bool _init();
    void _clear_buffer_index(uint8_t id);

public:
    bool request_buffer(uint8_t id, size_t size);
    bool request_buffer(uint8_t id, const void * begin, size_t size);
    MTL::ComputePipelineState * get_pipeline(size_t);
    MTL::Buffer * get_buffer(size_t);
    
    bool load_kernels(std::vector<std::string> kernels);
    MTL::CommandBuffer * get_command_buffer();
    MetalDevice();
    
    MetalDevice(MetalDevice& other) = delete;
    MetalDevice& operator=(MetalDevice& other) = delete;
    ~MetalDevice();
    
};

// General metal-related utils
#define define_mtl_buffer(name) \
size_t name = -1;
