#pragma once
#include <iostream>

#define debug(expr) \
std::cout << __FILE__ << ":" << __LINE__ << " " << expr << std::endl;

#define test_assert(expr) \
if (!(expr)) { \
debug( #expr " assert failed") \
exit(1);\
}
