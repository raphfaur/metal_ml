#include <memory>
#include "../common/device.hpp"
#include "../common/types.hpp"

template <typename K, typename V> struct __is_valid_key_value_size {
    static const bool value = (sizeof(K) <= sizeof(__MTL_MAX_ATOMIC_TYPE) &&
                               sizeof(V) <= sizeof(__MTL_MAX_ATOMIC_TYPE));
};

template <typename K, typename V,
          typename Enable =
              std::enable_if_t<__is_valid_key_value_size<K, V>::value, void>>
class mtl_map {
    // Types
  public:
    using bucket = pair<K, V>;
    using key = K;
    using value = V;

  private:
    size_t _memory_map_size = 0;
    size_t _size = 0;

    std::shared_ptr<MetalDevice> _mtl_device;
    
    // Buffers
    typedef enum {
        BUF_MAIN = 0,
        BUF_SIZE,
        BUF_INSERT,
        BUF_LOOKUP_IN,
        BUF_LOOKUP_OUT,
        _BUF_MAX
    } _mtl_map_buffer;

    bool _init();

  public:
    void reserve(size_t size);
    bool init();
    void clear();
    bool insert(pair<K, V> pair);
    bool insert_multi(bucket *begin, bucket *end);
    bool lookup_multi(key *begin, key *end, value *out);
    value lookup(key k);

    ~mtl_map() = default;
    mtl_map() = delete;
    mtl_map(std::shared_ptr<MetalDevice>);
    mtl_map(mtl_map &other) = delete;
    mtl_map &operator=(mtl_map &other) = delete;
};

typedef enum _mtl_map_kernel {
    KER_MULTI_INSERT = 0,
    KER_MULTI_LOOKUP,
    _KER_MAX
} _mtl_map_kernel;
