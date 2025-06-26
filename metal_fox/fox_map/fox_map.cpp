#include "fox_map.hpp"
#include <iostream>
#include <format>

typedef enum fox_map_kernel {
    INSERT_K = 0,
    LOOKUP_K,
    MULTI_INSERT_K,
    MULTI_LOOKUP_K,
    K_COUNT
} fox_map_kernel_e;

const char * fox_map_kernels[fox_map_kernel_e::K_COUNT] = {
    "map_insert",
    "map_lookup",
    "multi_map_insert",
    "multi_map_lookup"
};

MTL::ComputePipelineState * fox_map_pipelines[fox_map_kernel_e::K_COUNT] = {nullptr};

bool fox_map::_kernel_init() {
    NS::Error * error;
    require(_mtl_device, err);
    for (int k = 0; k < fox_map_kernel_e::K_COUNT; k++) {
        auto k_name = NS::String::string(fox_map_kernels[k], NS::UTF8StringEncoding);
        auto computeFunction = _lib->newFunction(k_name);
        std::cout << k_name->utf8String() << std::endl;
        auto pipeline_state = _mtl_device->newComputePipelineState(computeFunction, &error);
        std::cout << "Created pipeline " << pipeline_state->label() << std::endl;
        fox_map_pipelines[k] = pipeline_state;
        require(!error, err);
    }
    return true;

err:
    return false;
}

bool fox_map::_init(){
    // Get GPU device
    _mtl_device = MTL::CreateSystemDefaultDevice();
    require(_mtl_device, err);
    
    // Create command queue
    _map_cmd_q = _mtl_device->newCommandQueue();
    _map_cmd_q->setLabel(NS::String::string("fox_map", NS::UTF8StringEncoding));
    
    // Create lib
    _lib = _mtl_device->newDefaultLibrary();
    
    // Allocate main memory for the hash table
    _main_buffer = _mtl_device->newBuffer(MAP_SIZE, MTL::ResourceStorageModeShared);
    // Set to sentinel value since it is empty
    memset(_main_buffer->contents(), SENTINEL, MAP_SIZE);
    require(_main_buffer, err);
    
    // Allocate insert buffer
    _insert_buffer = _mtl_device->newBuffer(sizeof(bucket), MTL::ResourceStorageModeShared);
    require(_insert_buffer, err);
    
    // Allocate lookup buffer
    _lookup_in_buffer = _mtl_device->newBuffer(sizeof(key), MTL::ResourceStorageModeShared);
    require(_lookup_in_buffer, err);
    _lookup_out_buffer = _mtl_device->newBuffer(sizeof(value), MTL::ResourceStorageModeShared);
    require(_lookup_out_buffer, err);
    
    // Init kernels
    require(_kernel_init(), err);
    
    return true;
err:
    return false;
}

bool fox_map::init() {
    return this->_init();
}

void debug_region(uint8_t * region, size_t length) {
    for (size_t i = 0; i < length; i++) {
        std::cout << std::hex << (int)region[i] << " ";
    }
    std::cout << std::endl;
}

bool fox_map::insert(key k, value v){
    if ((k == 0 && v == 0) ) return false;
    auto cmd_buf = _map_cmd_q->commandBuffer();
    auto encoder = cmd_buf->computeCommandEncoder();
    
    auto ins_pipeline = fox_map_pipelines[fox_map_kernel_e::INSERT_K];
    
    encoder->setComputePipelineState(ins_pipeline);
    
    // Set insert args
    auto buffer = (uint16_t * ) _insert_buffer->contents();
    buffer[0] = k;
    buffer[1] = v;
    encoder->setBuffer(_insert_buffer, 0, 0);
    
    // Set main buffer
    encoder->setBuffer(_main_buffer, 0, 1);
    
    auto threadGroupSize = MTL::Size(ins_pipeline->maxTotalThreadsPerThreadgroup(), 1, 1);
    auto gridSize = MTL::Size(32, 1, 1);
    
    encoder->dispatchThreads(gridSize, threadGroupSize);
    encoder->endEncoding();
    
    cmd_buf->commit();
    std::cout << "Waiting" << std::endl;
    cmd_buf->waitUntilCompleted();
    std::cout << "Done waiting" << std::endl;
    
#if MAP_DEBUG
    debug_region((uint8_t*)_main_buffer->contents(), 100);
#endif
    
    return true;
    
err:
    return false;
}

value fox_map::lookup(key k){
    auto cmd_buf = _map_cmd_q->commandBuffer();
    auto encoder = cmd_buf->computeCommandEncoder();
    
    auto lup_pipeline = fox_map_pipelines[fox_map_kernel_e::LOOKUP_K];
    
    encoder->setComputePipelineState(lup_pipeline);
    
    // Set lookup key
    auto in_buff = (uint16_t * ) _lookup_in_buffer->contents();
    *in_buff = k;
    
    encoder->setBuffer(_lookup_in_buffer, 0, 0);
    encoder->setBuffer(_lookup_out_buffer, 0, 1);
    
    // Set main buffer
    encoder->setBuffer(_main_buffer, 0, 2);
    
    auto threadGroupSize = MTL::Size(lup_pipeline->maxTotalThreadsPerThreadgroup(), 1, 1);
    auto gridSize = MTL::Size(32, 1, 1);
    
    encoder->dispatchThreads(gridSize, threadGroupSize);
    encoder->endEncoding();
    
    cmd_buf->commit();
    cmd_buf->waitUntilCompleted();
    
    auto out_buff = (uint16_t * ) _lookup_out_buffer->contents();
    return *out_buff;
}

bool fox_map::insert_multi(bucket *begin, bucket *end) {
    auto cmd_buf = _map_cmd_q->commandBuffer();
    auto encoder = cmd_buf->computeCommandEncoder();
    
    auto multi_ins_pipeline = fox_map_pipelines[fox_map_kernel_e::MULTI_INSERT_K];
    
    encoder->setComputePipelineState(multi_ins_pipeline);
    
    // Compute entry count
    size_t entry_n = end - begin;
    
    // Set insert args
    auto entries_buffer = _mtl_device->newBuffer(begin, entry_n * sizeof(bucket), MTL::ResourceStorageModeShared);
    encoder->setBuffer(entries_buffer, 0, 0);
    
    // Set main buffer
    encoder->setBuffer(_main_buffer, 0, 1);
    
    auto threadGroupSize = MTL::Size(multi_ins_pipeline->maxTotalThreadsPerThreadgroup(), 1, 1);
    auto gridSize = MTL::Size(multi_ins_pipeline->threadExecutionWidth() * entry_n, 1, 1);
    
    encoder->dispatchThreads(gridSize, threadGroupSize);
    encoder->endEncoding();
    
    cmd_buf->commit();
    std::cout << "Waiting" << std::endl;
    cmd_buf->waitUntilCompleted();
    std::cout << "Done waiting" << std::endl;
    
#if MAP_DEBUG
    debug_region((uint8_t*)_main_buffer->contents(), 100);
#endif
    
    return true;
    
err:
    return false;
}

bool fox_map::lookup_multi(key *begin, key *end) {
    auto cmd_buf = _map_cmd_q->commandBuffer();
    auto encoder = cmd_buf->computeCommandEncoder();
    
    auto multi_lookup_pipeline = fox_map_pipelines[fox_map_kernel_e::MULTI_LOOKUP_K];
    
    encoder->setComputePipelineState(multi_lookup_pipeline);
    
    // Compute key count
    size_t key_n = end - begin;
    
    // Set keys buffer
    auto entries_buffer = _mtl_device->newBuffer(begin, key_n * sizeof(key), MTL::ResourceStorageModeShared);
    encoder->setBuffer(entries_buffer, 0, 0);
    
    // Set output buffer
    auto out_buffer = _mtl_device->newBuffer(begin, key_n * sizeof(key), MTL::ResourceStorageModeShared);
    encoder->setBuffer(out_buffer, 0, 1);
    
    // Set main buffer
    encoder->setBuffer(_main_buffer, 0, 2);
    
    auto threadGroupSize = MTL::Size(multi_lookup_pipeline->maxTotalThreadsPerThreadgroup(), 1, 1);
    auto gridSize = MTL::Size(multi_lookup_pipeline->threadExecutionWidth() * key_n, 1, 1);
    
    encoder->dispatchThreads(gridSize, threadGroupSize);
    encoder->endEncoding();
    
    cmd_buf->commit();
    std::cout << "Waiting" << std::endl;
    cmd_buf->waitUntilCompleted();
    std::cout << "Done waiting" << std::endl;
    
#if MAP_DEBUG
    debug_region((uint8_t *) out_buffer->contents(), 10);
#endif
    
    return true;
    
err:
    return false;
}
