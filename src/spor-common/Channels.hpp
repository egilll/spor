#pragma once
#include <cstdint>

enum class Channel : uint8_t {
    CONSOLE_LOG = 0,
    MESSAGE_TYPE,
    MESSAGE_DATA,
    // MESSAGE_START,
    // MESSAGE_END,
    CYCLE_COUNT,
    FUNCTION_ENTER,
    FUNCTION_EXIT,
    _NUM_CHANNELS,
};
static_assert(static_cast<int>(Channel::_NUM_CHANNELS) <= 32);