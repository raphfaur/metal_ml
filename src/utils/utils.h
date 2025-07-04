#include <format>
#include <iostream>
#include <stdlib.h>
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

#define debug_no_line(...)                                                     \
    std::cout << "[ DEBUG ] " __FILE__ << ":" << __LINE__ << " "               \
              << std::format(__VA_ARGS__);

#define debug(...)                                                             \
    debug_no_line(__VA_ARGS__);                                                \
    std::cout << std::endl;

#define info(label, expr)                                                      \
    std::cout << FYEL("[ INFO ] ") << __FILE__ << ":" << __LINE__ << " "       \
              << label << " - " << expr << std::endl;

#define test_debug(expr)                                                       \
    std::cout << FBLU("[ TEST ] ") << __FILE__ << ":" << __LINE__ << " "       \
              << expr << std::endl;

// TEST

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

// BENCHMARK

void debug_memory(uint8_t *region, size_t length);
