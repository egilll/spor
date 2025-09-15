#include "TimeUtils.hpp"

#include <chrono>

namespace profiler {

static uint64_t time_ns = 0;

void SetTime(uint64_t nanoseconds) {
    time_ns = nanoseconds;
}

uint64_t GetTime() {
    // Todo: Need to zero-initialise trace instead of this
    static uint64_t initTimestamp =
        std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch())
            .count();
    return initTimestamp + time_ns;
}

}