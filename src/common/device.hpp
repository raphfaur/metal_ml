#include "../utils/metal_config.hpp"
#include "../utils/utils.h"
#include <stdexcept>

// General metal-related utils
#define define_mtl_buffer(name) size_t name = -1;

class MetalDevice {
    MTL::Device *_mtl_device = nullptr;
    MTL::CommandQueue *_map_cmd_q = nullptr;
    MTL::Library *_lib = nullptr;

    // Internal but exposed
    std::vector<MTL::ComputePipelineState *> _pipelines;
    std::unordered_map<uint8_t, MTL::Buffer *> _buffers;

    bool _init();
    void _clear_buffer_index(uint8_t id);

  public:
    bool request_buffer(uint8_t id, size_t size);
    bool request_buffer(uint8_t id, const void *begin, size_t size);
    MTL::ComputePipelineState *get_pipeline(size_t);
    MTL::Buffer *get_buffer(size_t);

    bool load_kernels(std::vector<std::string> kernels);
    MTL::CommandBuffer *get_command_buffer();
    MetalDevice();

    MetalDevice(MetalDevice &other) = delete;
    MetalDevice &operator=(MetalDevice &other) = delete;
    ~MetalDevice();
};

MetalDevice::MetalDevice() {
    // Get GPU device
    _mtl_device = MTL::CreateSystemDefaultDevice();
    require(_mtl_device, err);

    // Create command queue
    _map_cmd_q = _mtl_device->newCommandQueue();
    _map_cmd_q->setLabel(
        NS::String::string("mtl_device", NS::UTF8StringEncoding));

    // Create lib
    _lib = _mtl_device->newDefaultLibrary();
    return;
err:
    throw std::length_error("Unable to get default GPU");
};

MetalDevice::~MetalDevice() {
    for (auto p = _pipelines.begin(); p < _pipelines.end(); p++) {
        (*p)->release();
    }
    for (auto b = _buffers.begin(); b != _buffers.end(); b++) {
        b->second->release();
    }
    mtl_release(_map_cmd_q);
    mtl_release(_lib);
    mtl_release(_mtl_device);
}

bool MetalDevice::_init() {
    // Create command queue
    _map_cmd_q = _mtl_device->newCommandQueue();
    _map_cmd_q->setLabel(NS::String::string("fox_map", NS::UTF8StringEncoding));

    // Create lib
    _lib = _mtl_device->newDefaultLibrary();

    return true;
}

bool MetalDevice::load_kernels(std::vector<std::string> kernels) {
    NS::Error *error;
    require(_mtl_device, err);
    for (auto k = kernels.begin(); k < kernels.end(); k++) {
        debug("Building kernel {}", *k);
        auto k_name = NS::String::string(k->data(), NS::UTF8StringEncoding);
        auto computeFunction = _lib->newFunction(k_name);
        require(computeFunction, err);
        info("Built kernel", k_name->utf8String());
        k_name->release();
        auto pipeline_state =
            _mtl_device->newComputePipelineState(computeFunction, &error);
        _pipelines.push_back(pipeline_state);
        require(!error, err);
    }
    return true;
err:
    return false;
}

MTL::Buffer *MetalDevice::get_buffer(size_t index) { return _buffers[index]; }

MTL::ComputePipelineState *MetalDevice::get_pipeline(size_t index) {
    return _pipelines[index];
}

void MetalDevice::_clear_buffer_index(uint8_t id) {
    auto buffer = _buffers.find(id);
    if (buffer != _buffers.end()) {
        //        debug("Realeasing buffer {}", id);
        buffer->second->release();
        _buffers.erase(buffer);
    }
}

bool MetalDevice::request_buffer(uint8_t id, size_t size) {
    _clear_buffer_index(id);
    auto buffer = _mtl_device->newBuffer(size, MTL::ResourceStorageModeShared);
    if (!buffer)
        return false;
    _buffers[id] = buffer;
    return true;
}

bool MetalDevice::request_buffer(uint8_t id, const void *begin, size_t size) {
    //    debug("Requesting buffer from address {}", begin);
    if (_buffers.find(id) != _buffers.end()) {
        return true;
    }
    _clear_buffer_index(id);
    auto buffer =
        _mtl_device->newBuffer(begin, size, MTL::ResourceStorageModeShared);
    if (!buffer)
        return false;
    _buffers[id] = buffer;
    return true;
}

MTL::CommandBuffer *MetalDevice::get_command_buffer() {
    require(_map_cmd_q, err);
    return (_map_cmd_q->commandBuffer());
err:
    return nullptr;
}
