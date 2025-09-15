#pragma once

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <span>

#include SPOR_SYSTEM_HEADER
#include "IrqLockGuard.hpp"
#include "orbcode/trace/itm.h"
#include "spor-common/Messages.hpp"
#include "Utils.hpp"
#include "zpp_bits.h"

namespace spor {

#ifndef spor_BUFFER_SIZE
#define spor_BUFFER_SIZE 100
#endif

template <typename T, typename Variant>
struct VariantIndex;

template <typename T, typename... Types>
struct VariantIndex<T, std::variant<Types...>> {
    static constexpr std::size_t value = []() {
        std::size_t index = 0;
        ((std::is_same_v<T, Types> ? false : (++index, true)) && ...);
        return index;
    }();
};

template <typename T>
constexpr uint8_t GetMessageIndex() {
    return static_cast<uint8_t>(VariantIndex<T, Message>::value);
}

class Transport {
public:
    static bool NO_INSTRUMENT isReady();
};

void NO_INSTRUMENT SendChannel(Channel channel, std::span<const std::byte> data);

/** Todo: Merge with below */
inline NO_INSTRUMENT void SendCycleCount() {
    // const IrqLockGuard lock{};
    ITMWrite32(static_cast<uint8_t>(Channel::CYCLE_COUNT), DWT->CYCCNT);
}

template <typename T>
void NO_INSTRUMENT Send(const T &message) {
    if (!Transport::isReady())
        return;

    const IrqLockGuard lock{};

    SendCycleCount();

    constexpr uint8_t messageTypeIndex = GetMessageIndex<T>();
    SendChannel(Channel::MESSAGE_TYPE, std::span{reinterpret_cast<const std::byte *>(&messageTypeIndex), 1});

    static std::array<std::byte, spor_BUFFER_SIZE> buffer;
    auto out = zpp::bits::out{buffer};
    auto result = out(message);
    if (zpp::bits::failure(result)) {
        return;
    }
    auto data = std::span<const std::byte>{buffer.data(), out.position()};
    SendChannel(Channel::MESSAGE_DATA, data);
}

}