#define _LIBCPP_HAS_NO_INCOMPLETE_FORMAT
#include <format>
#include <iostream>
#include <stdlib.h>

#define DEBUG_LEVEL 1
#define LEVEL_DEBUG 2
#define LEVEL_INFO 1

// Utils

#define PASTE(x, y) x##y
#define PASTE2(x, y) PASTE(x, y)

// COLORS

#define RST "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"

#define FRED(x) KRED x RST
#define FGRN(x) KGRN x RST
#define FYEL(x) KYEL x RST
#define FBLU(x) KBLU x RST
#define FMAG(x) KMAG x RST
#define FCYN(x) KCYN x RST
#define FWHT(x) KWHT x RST

#define BOLD(x) "\x1B[1m" x RST
#define UNDL(x) "\x1B[4m" x RST

// COLORS

#define require(assert, label)                                                 \
    if (!(assert)) {                                                           \
        debug(FRED("Assertion failed : " #assert));                            \
        goto label;                                                            \
    }

#define require_exit(assert, ret_value)                                        \
    if (!(assert)) {                                                           \
        debug(FRED("Assertion failed : " #assert));                            \
        return ret_value;                                                      \
    }

#define mtl_release(buffer)                                                    \
    if (buffer)                                                                \
        buffer->release();

#if DEBUG_LEVEL >= LEVEL_DEBUG
#define debug_no_line(...)                                                     \
    std::cout << "[ DEBUG ] " __FILE__ << ":" << __LINE__ << " "               \
              << std::format(__VA_ARGS__);
#else
#define debug_no_line(...)
#endif

#if DEBUG_LEVEL >= LEVEL_DEBUG
#define debug(...)                                                             \
    debug_no_line(__VA_ARGS__);                                                \
    std::cout << std::endl;
#else
#define debug(...)
#endif

#if DEBUG_LEVEL >= LEVEL_INFO
#define info(...)                                                              \
    std::cout << FYEL("[ INFO ] ") << __FILE__ << ":" << __LINE__ << " "       \
              << std::format(__VA_ARGS__) << std::endl;
#else
#define info(...)
#endif

#define debug_error(...)                                                       \
    std::cout << FRED("[ ERROR ] ") << __FILE__ << ":" << __LINE__ << " "      \
              << std::format(__VA_ARGS__) << std::endl;

// TEST
#define test_debug(...)                                                        \
    std::cout << FBLU("[ TEST ] ")       \
              << std::format(__VA_ARGS__) << std::endl;

#define test_assert(expr)                                                      \
    if (!(expr)) {                                                             \
        debug(#expr " assert failed") return false;                            \
    }

#define test_run_must_pass(func)                                               \
    test_debug(BOLD("Running test " #func)) if (func()) {                      \
        test_debug(FGRN("Test " #func " passed as expected. \n"));             \
    }                                                                          \
    else {                                                                     \
        test_debug(FRED("Test " #func " unexpectedly failed. \n"));            \
    }

#define test_run_must_fail(func)                                               \
    test_debug(BOLD("Running test " #func)) if (!func()) {                     \
        test_debug(FGRN("Test " #func " failed as expected. \n"));             \
    }                                                                          \
    else {                                                                     \
        test_debug(FRED("Test " #func " unexpectedly passed. \n"));            \
    }

#define test_run_n(expr, n)                                                    \
    test_debug(BOLD("Performing test " #expr " - {} runs"), n);                \
    for (int run = 0; run < n; run++) {                                        \
        test_debug("Run {}", run);                                             \
        test_run_must_pass(expr);                                              \
    }

// BENCHMARK

#define bench_debug(...)                                                       \
    std::cout << FMAG("[ BENCHMARK ] ")  \
              << std::format(__VA_ARGS__) << std::endl;

#define benchmark_init()                                                       \
    std::unordered_map<std::string, std::vector<size_t>> __configs;            \
    auto __bench_start = std::chrono::system_clock::now();                     \
    auto __bench_stop = std::chrono::system_clock::now();                      \
    auto __elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(    \
        __bench_stop - __bench_start);                                         \
    std::string __test_name;

#define bench_reset() __configs.clear()

#define bench_run(expr)                                                        \
    bench_debug(BOLD(FMAG("Starting benchmark" #expr "\n")));                  \
    expr;

#define register_config(name)                                                  \
    std::string __config_##name(#name);                                        \
    __configs[__config_##name];

#define measure_expr(expr)                                                     \
    __bench_start = std::chrono::system_clock::now();                          \
    expr;                                                                      \
    __bench_stop = std::chrono::system_clock::now();

#define time_this(name, expr)                                                  \
    measure_expr(expr) __elapsed =                                             \
        std::chrono::duration_cast<std::chrono::milliseconds>(__bench_stop -   \
                                                              __bench_start);  \
    __configs[__config_##name].push_back(__elapsed.count());                   \
    bench_debug("Running [" #name "] config : {} ms", __elapsed.count());

#define dump_mean(name)                                                        \
    {                                                                          \
        float sum = std::accumulate(__configs[__config_##name].begin(),        \
                                    __configs[__config_##name].end(), 0.0);    \
        float mean = sum / __configs[__config_##name].size();                  \
        bench_debug(FBLU(BOLD("[" #name "] - {} runs - mean : {} ms\n")),      \
                    __configs[__config_##name].size(), mean);                  \
    }

#define time_this_n(name, init, expr, n)                                       \
    bench_debug(BOLD(FBLU("[" #name "] - {} runs")), n);                       \
    for (int run = 0; run < n; run++) {                                        \
        init;                                                                  \
        time_this(name, expr);                                                 \
    }                                                                          \
    dump_mean(name)

#define dump_results(name)                                                     \
    for (int i = 0; i < __configs[__config_##name].size(); i++) {              \
        bench_debug(#name " - {} : {} ms", i, __configs[__config_##name][i]);  \
    }

#define INIT_BLOCK(...) __VA_ARGS__

// Utils methods

void debug_memory(uint8_t *region, size_t length);
