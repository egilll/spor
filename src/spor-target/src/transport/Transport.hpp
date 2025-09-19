#pragma once

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <span>

#include SPOR_SYSTEM_HEADER
#include "IrqLockGuard.hpp"
#include "spor-common/Messages.hpp"
#include "Utils.hpp"
#include "transport/RTT.hpp"

namespace spor {

// Use a dedicated RTT up-buffer for SPOR traffic.
#ifndef SPOR_RTT_UP_BUFFER_INDEX
#define SPOR_RTT_UP_BUFFER_INDEX 0
#endif

struct RttStreamWriter {
    static inline void put_u8(uint8_t v) { SEGGER_RTT_PutChar(SPOR_RTT_UP_BUFFER_INDEX, static_cast<char>(v)); }
    static inline void put_u16(uint16_t v) {
        put_u8(static_cast<uint8_t>(v & 0xFF));
        put_u8(static_cast<uint8_t>((v >> 8) & 0xFF));
    }
    static inline void put_u32(uint32_t v) {
        put_u8(static_cast<uint8_t>(v & 0xFF));
        put_u8(static_cast<uint8_t>((v >> 8) & 0xFF));
        put_u8(static_cast<uint8_t>((v >> 16) & 0xFF));
        put_u8(static_cast<uint8_t>((v >> 24) & 0xFF));
    }
    static inline void put_u64(uint64_t v) {
        put_u32(static_cast<uint32_t>(v & 0xFFFFFFFFULL));
        put_u32(static_cast<uint32_t>((v >> 32) & 0xFFFFFFFFULL));
    }
    static inline void put_s32(int32_t v) { put_u32(static_cast<uint32_t>(v)); }
    static inline void put_s64(int64_t v) { put_u64(static_cast<uint64_t>(v)); }
    static inline void put_stringz(const char *s) {
        if (!s) {
            put_u8(0);
            return;
        }
        while (*s) {
            SEGGER_RTT_PutChar(SPOR_RTT_UP_BUFFER_INDEX, *s++);
        }
        SEGGER_RTT_PutChar(SPOR_RTT_UP_BUFFER_INDEX, 0);
    }
};


class Transport {
public:
    static bool NO_INSTRUMENT isReady();
};

template <typename T>
void NO_INSTRUMENT Send(const T &message) {
    if (!Transport::isReady())
        return;

    const IrqLockGuard lock{};
    constexpr uint8_t messageTypeIndex = static_cast<uint8_t>(MessageTag<T>::value);
    RttStreamWriter w{};
    // [type]
    w.put_u8(messageTypeIndex);
    // [cycles as u32, LE]
    w.put_u32(DWT->CYCCNT);
    // [payload fields]
    spor::wire::write(w, message);
}

}
