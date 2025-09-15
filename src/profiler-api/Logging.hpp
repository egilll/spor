#pragma once
#include <iostream>
#include <sstream>
#include <string_view>
#include <type_traits>

// #define profiler_DEBUG_PRINT 1

#ifdef profiler_DEBUG_PRINT

template <typename T>
auto safeArg(const T &arg) {
    if constexpr (std::is_same_v<T, const char *> || std::is_same_v<T, char *>) {
        return arg ? arg : "NULL";
    } else {
        return arg;
    }
}

template <typename... Args>
void debugLog(std::string_view func, const Args &...args) {
    std::ostringstream oss;
    oss << "profiler: " << func;
    if constexpr (sizeof...(args) > 0) {
        oss << " | args: ";
        ((oss << safeArg(args) << ", "), ...);
    }
    std::cout << oss.str() << "\n" << std::flush;
}

#define DEBUG_FUNCTION(...) debugLog(__func__, ##__VA_ARGS__)

#else

#define DEBUG_FUNCTION(...)                                                                                            \
    do {                                                                                                               \
    } while (0)

#endif