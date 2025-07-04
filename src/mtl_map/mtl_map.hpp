#include "../common/metal.hpp"

template<
        typename K,
        typename V,
        typename Enable = std::enable_if_t<__is_valid_key_value_size<K, V>::value, void>
>
class mtl_map {
    MetalDevice _mtl_device;

    // Buffers
    typedef enum {
        BUF_MAIN = 0,
        BUF_INSERT,
        BUF_LOOKUP_IN,
        BUF_LOOKUP_OUT,
        _BUF_MAX
    } _mtl_map_buffer;
    
    bool _init();
    
public:
    bool init();
    bool insert(key k, value v);
    bool insert_multi(bucket * begin, bucket * end);
    bool lookup_multi(key * begin, key * end, value *out);
    value lookup(key k);
    
    ~mtl_map() = default;
    mtl_map(mtl_map& other) = delete;
    mtl_map& operator=(mtl_map& other) = delete;
};

typedef enum _mtl_map_kernel {
    KER_MULTI_INSERT = 0,
    KER_MULTI_LOOKUP,
    _KER_MAX
} _mtl_map_kernel;
