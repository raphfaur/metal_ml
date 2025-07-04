using __MTL_MAX_ATOMIC_TYPE = uint32_t;

template <int size> struct __sized_unsigned {};
template <> struct __sized_unsigned<2> { typedef uint32_t type;};
template <> struct __sized_unsigned<3> { typedef uint32_t type;};
template <> struct __sized_unsigned<4> { typedef uint32_t type;};
template <> struct __sized_unsigned<5> { typedef uint64_t type;};
template <> struct __sized_unsigned<6> { typedef uint64_t type;};
template <> struct __sized_unsigned<7> { typedef uint64_t type;};
template <> struct __sized_unsigned<8> { typedef uint64_t type;};

template<typename K, typename V>
union pair {
    struct __attribute__((packed)) __pair {
        K k;
        V v;
        __pair(K k, V v) : k(k), v(v){};
    } inner ;
    
    using raw_t = typename __sized_unsigned<sizeof(__pair)>::type;
    raw_t raw;
    
    pair(K key, V value) : inner(key,value){};
    pair(raw_t raw_v) : raw(raw_v){};
    pair() : raw(0){};
};

