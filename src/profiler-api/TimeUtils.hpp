#pragma once

#include <cstdint>

namespace profiler {

void SetTime(uint64_t nanoseconds);
uint64_t GetTime();

}