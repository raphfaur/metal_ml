#pragma once
#include "../common/device.hpp"

template<typename T, size_t D>
class mtl_kmean {
private:
    // Buffers
    typedef enum {
        BUF_DATA = 0,
        BUF_MEANS,
        BUF_INSERT,
        BUF_LOOKUP_IN,
        BUF_LOOKUP_OUT,
        _BUF_MAX
    } _mtl_map_buffer;
public:
    mtl_kmean();
    ~mtl_mean();
    mtl_kmean& operator=(mtl_kmean &other);
    mtl_kmean(mtl_kmean &other);
};
