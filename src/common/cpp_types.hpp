template <typename T> struct type_name {};

template <> struct type_name<uint8_t> {
    static constexpr const char *name = "uint8_t";
};
template <> struct type_name<uint16_t> {
    static constexpr const char *name = "uint16_t";
};
template <> struct type_name<uint32_t> {
    static constexpr const char *name = "uint32_t";
};
