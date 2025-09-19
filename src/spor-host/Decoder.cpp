#include "Decoder.hpp"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>

// #define SPOR_DECODER_DEBUG

// Using a simple fixed-width + stringz decoder for the new wire format
#include <type_traits>

struct Reader {
    const uint8_t *ptr;
    size_t size;
    size_t pos{0};

    Reader(const uint8_t *p, size_t n) : ptr(p), size(n) {}

    bool can_read(size_t n) const {
        return pos + n <= size;
    }
    bool read_u8(uint8_t &v) {
        if (!can_read(1))
            return false;
        v = ptr[pos++];
        return true;
    }
    bool read_u16(uint16_t &v) {
        if (!can_read(2))
            return false;
        v = static_cast<uint16_t>(ptr[pos]) | (static_cast<uint16_t>(ptr[pos + 1]) << 8);
        pos += 2;
        return true;
    }
    bool read_u32(uint32_t &v) {
        if (!can_read(4))
            return false;
        v = static_cast<uint32_t>(ptr[pos]) | (static_cast<uint32_t>(ptr[pos + 1]) << 8) |
            (static_cast<uint32_t>(ptr[pos + 2]) << 16) | (static_cast<uint32_t>(ptr[pos + 3]) << 24);
        pos += 4;
        return true;
    }
    bool read_u64(uint64_t &v) {
        uint32_t lo, hi;
        if (!read_u32(lo) || !read_u32(hi))
            return false;
        v = (static_cast<uint64_t>(hi) << 32) | lo;
        return true;
    }
    bool read_s32(int32_t &v) {
        uint32_t u;
        if (!read_u32(u))
            return false;
        v = static_cast<int32_t>(u);
        return true;
    }
    bool read_s64(int64_t &v) {
        uint64_t u;
        if (!read_u64(u))
            return false;
        v = static_cast<int64_t>(u);
        return true;
    }
    bool read_stringz(std::string &out) {
        // find zero terminator
        size_t i = pos;
        while (i < size && ptr[i] != 0)
            ++i;
        if (i == size)
            return false; // not complete
        out.assign(reinterpret_cast<const char *>(ptr + pos), i - pos);
        pos = i + 1; // skip terminator
        return true;
    }
};

// Generic field readers to reduce duplication
namespace {
template <typename T>
inline bool read_field(Reader &r, T &v) {
    if constexpr (std::is_same_v<T, uint8_t>) {
        return r.read_u8(v);
    } else if constexpr (std::is_same_v<T, uint16_t>) {
        return r.read_u16(v);
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        return r.read_u32(v);
    } else if constexpr (std::is_same_v<T, uint64_t>) {
        return r.read_u64(v);
    } else if constexpr (std::is_same_v<T, int32_t>) {
        return r.read_s32(v);
    } else if constexpr (std::is_same_v<T, int64_t>) {
        return r.read_s64(v);
    } else if constexpr (std::is_same_v<T, bool>) {
        uint8_t b{};
        if (!r.read_u8(b))
            return false;
        v = (b != 0);
        return true;
    } else if constexpr (std::is_enum_v<T>) {
        uint8_t u{};
        if (!r.read_u8(u))
            return false;
        v = static_cast<T>(u);
        return true;
    } else if constexpr (std::is_same_v<T, std::string>) {
        return r.read_stringz(v);
    } else {
        // Fallback for types that are typedefs/aliases of integral types
        using U = std::remove_cv_t<std::remove_reference_t<T>>;
        if constexpr (std::is_same_v<U, unsigned int> || std::is_same_v<U, uint32_t>) {
            return r.read_u32(reinterpret_cast<uint32_t &>(v));
        } else if constexpr (std::is_same_v<U, unsigned long long> || std::is_same_v<U, uint64_t>) {
            return r.read_u64(reinterpret_cast<uint64_t &>(v));
        } else {
            static_assert(!sizeof(T *), "Unsupported field type for read_field");
            return false;
        }
    }
}

template <typename... Ts>
inline bool read_fields(Reader &r, Ts &...vs) {
    return (read_field(r, vs) && ...);
}
} // namespace

MessageDecoder::MessageDecoder(IMessageHandler &handler) : handler_(handler) {}

MessageDecoder *MessageDecoder::GetInstance() {
    extern MessageDecoder *GetSporInstance();
    return GetSporInstance();
}

void MessageDecoder::ProcessBytes(std::span<const std::byte> data) {
    const auto *bytes = reinterpret_cast<const uint8_t *>(data.data());
    streamBuffer.insert(streamBuffer.end(), bytes, bytes + data.size());

    TryParse();
}

#define DEC_DEBUG(...)                                                                                                 \
    do {                                                                                                               \
    } while (0)

void MessageDecoder::TryParse() {
    size_t offset = 0;
    const size_t N = streamBuffer.size();
    while (offset < N) {
        Reader r(streamBuffer.data() + offset, N - offset);
        uint8_t type = 0;
        if (!r.read_u8(type))
            break;
        uint32_t cycles = 0;
        if (!r.read_u32(cycles))
            break;

        DEC_DEBUG(
            "try type=" << unsigned(type) << " (" << message_type_name(type) << ") cycles=" << cycles
                        << " avail=" << (r.size - r.pos)
        );

        // Decode per message type; if incomplete, stop and wait for more.
        auto try_decode = [&](uint8_t t, Reader &rr) -> bool {
            switch (t) {
            case static_cast<uint8_t>(MessageType::ZoneBeginData): {
                ZoneBeginData v{};
                return read_fields(rr, v.ptr) && (handler_.HandleMessage(v), true);
            }
            case static_cast<uint8_t>(MessageType::ZoneEndData): {
                ZoneEndData v{};
                v.ptr = 0;
                return (handler_.HandleMessage(v), true);
            }
            case static_cast<uint8_t>(MessageType::ZoneTextMessage): {
                ZoneTextMessage v{};
                return read_fields(rr, v.text) && (handler_.HandleMessage(v), true);
            }
            case static_cast<uint8_t>(MessageType::ZoneValueData): {
                ZoneValueData v{};
                return read_fields(rr, v.value) && (handler_.HandleMessage(v), true);
            }
            case static_cast<uint8_t>(MessageType::ZoneColorData): {
                ZoneColorData v{};
                uint8_t r8, g8, b8;
                if (!rr.read_u8(r8) || !rr.read_u8(g8) || !rr.read_u8(b8))
                    return false;
                v.r = r8;
                v.g = g8;
                v.b = b8;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::PlotMessage): {
                PlotMessage v{};
                if (!read_fields(rr, v.data.value, v.name))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::PlotConfigMessage): {
                PlotConfigMessage v{};
                if (!read_fields(rr, v.data.type, v.data.step, v.data.fill, v.data.color, v.name))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::MessageTextMessage): {
                MessageTextMessage v{};
                if (!read_fields(rr, v.text))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::AllocMessage): {
                AllocMessage v{};
                if (!read_fields(rr, v.data.ptr, v.data.size, v.name))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreeMessage): {
                FreeMessage v{};
                if (!read_fields(rr, v.data.ptr, v.name))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FunctionTraceEnterData): {
                FunctionTraceEnterData v{};
                return read_fields(rr, v.fn) && (handler_.HandleMessage(v), true);
            }
            case static_cast<uint8_t>(MessageType::FunctionTraceExitData): {
                FunctionTraceExitData v{};
                return read_fields(rr, v.fn) && (handler_.HandleMessage(v), true);
            }
            case static_cast<uint8_t>(MessageType::DeclarePointerTypeMessage): {
                DeclarePointerTypeMessage v{};
                return read_fields(rr, v.ptr, v.typeInfo) && (handler_.HandleMessage(v), true);
            }
            case static_cast<uint8_t>(MessageType::FlowBeginMessage): {
                FlowBeginMessage v{};
                return read_fields(rr, v.ptr) && (handler_.HandleMessage(v), true);
            }
            case static_cast<uint8_t>(MessageType::FlowEndMessage): {
                FlowEndMessage v{};
                return read_fields(rr, v.ptr) && (handler_.HandleMessage(v), true);
            }
            case static_cast<uint8_t>(MessageType::SystemInfoData): {
                SystemInfoData v{};
                return read_fields(rr, v.clock_frequency_mhz) && (handler_.HandleMessage(v), true);
            }
            case static_cast<uint8_t>(MessageType::InterruptConfigMessage): {
                InterruptConfigMessage v{};
                uint32_t irq32{}, prio32{};
                if (!read_fields(rr, irq32, prio32, v.irq_name))
                    return false;
                v.data.irq_number = static_cast<uint16_t>(irq32);
                v.data.irq_priority_level = static_cast<uint16_t>(prio32);
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::InterruptEnterData): {
                InterruptEnterData v{};
                uint32_t irq{};
                if (!read_fields(rr, irq, v.enabled_flags))
                    return false;
                v.irq_number = static_cast<uint16_t>(irq);
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::InterruptExitData): {
                InterruptExitData v{};
                uint32_t irq{};
                if (!read_fields(rr, irq))
                    return false;
                v.irq_number = static_cast<uint16_t>(irq);
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::PointerAnnounceMessage): {
                PointerAnnounceMessage v{};
                if (!read_fields(rr, v.symbolPointer, v.heapPointer))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::PointerSetNameMessage): {
                PointerSetNameMessage v{};
                if (!read_fields(rr, v.ptr, v.name))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            // FreeRTOS messages
            case static_cast<uint8_t>(MessageType::FreertosTaskCreatedMessage): {
                FreertosTaskCreatedMessage v{};
                if (!read_fields(rr, v.handle, v.name, v.priority))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosTaskSwitchedInMessage): {
                FreertosTaskSwitchedInMessage v{};
                if (!read_fields(rr, v.handle))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosTaskSwitchedOutMessage): {
                FreertosTaskSwitchedOutMessage v{};
                if (!read_fields(rr, v.handle, v.switchReason, v.blockedOnObject, v.stillReady))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosTaskReadiedMessage): {
                FreertosTaskReadiedMessage v{};
                if (!read_fields(rr, v.handle))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosQueueCreatedMessage): {
                FreertosQueueCreatedMessage v{};
                if (!read_fields(rr, v.handle, v.capacity, v.queueType))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosQueuePeekMessage): {
                FreertosQueuePeekMessage v{};
                if (!read_fields(rr, v.handle, v.updatedCount, v.isFromISR))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosQueueSendMessage): {
                FreertosQueueSendMessage v{};
                if (!read_fields(rr, v.handle, v.updatedCount, v.queueType, v.isFromISR))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosQueueReceiveMessage): {
                FreertosQueueReceiveMessage v{};
                if (!read_fields(rr, v.handle, v.updatedCount, v.queueType, v.isFromISR))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosTaskNotifyMessage): {
                FreertosTaskNotifyMessage v{};
                if (!read_fields(rr, v.handle, v.index, v.action, v.updatedValue, v.isFromISR))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosTaskNotifyReceivedMessage): {
                FreertosTaskNotifyReceivedMessage v{};
                if (!read_fields(rr, v.handle, v.index, v.updatedValue))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosTaskDeletedMessage): {
                FreertosTaskDeletedMessage v{};
                if (!read_fields(rr, v.handle))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosTaskPrioritySetMessage): {
                FreertosTaskPrioritySetMessage v{};
                if (!read_fields(rr, v.handle, v.oldPriority, v.newPriority))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosTaskSuspendMessage): {
                FreertosTaskSuspendMessage v{};
                if (!read_fields(rr, v.handle))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosTaskResumeMessage): {
                FreertosTaskResumeMessage v{};
                if (!read_fields(rr, v.handle, v.isFromISR))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosTimerCreatedMessage): {
                FreertosTimerCreatedMessage v{};
                if (!read_fields(rr, v.handle, v.period, v.autoReload))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosTimerCommandMessage): {
                FreertosTimerCommandMessage v{};
                if (!read_fields(rr, v.handle, v.commandId, v.optionalValue, v.isFromISR))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosTimerExpiredMessage): {
                FreertosTimerExpiredMessage v{};
                if (!read_fields(rr, v.handle))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosEventGroupCreatedMessage): {
                FreertosEventGroupCreatedMessage v{};
                if (!read_fields(rr, v.handle))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosEventGroupDeletedMessage): {
                FreertosEventGroupDeletedMessage v{};
                if (!read_fields(rr, v.handle))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosEventGroupSyncMessage): {
                FreertosEventGroupSyncMessage v{};
                if (!read_fields(rr, v.handle, v.setBits, v.waitBits, v.resultBits))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosEventGroupWaitBitsMessage): {
                FreertosEventGroupWaitBitsMessage v{};
                if (!read_fields(rr, v.handle, v.waitBits, v.resultBits))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosEventGroupClearBitsMessage): {
                FreertosEventGroupClearBitsMessage v{};
                if (!read_fields(rr, v.handle, v.clearBits, v.isFromISR))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosEventGroupSetBitsMessage): {
                FreertosEventGroupSetBitsMessage v{};
                if (!read_fields(rr, v.handle, v.setBits, v.isFromISR))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosStreamBufferCreatedMessage): {
                FreertosStreamBufferCreatedMessage v{};
                if (!read_fields(rr, v.handle, v.size, v.triggerLevel, v.isMessageBuffer))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosStreamBufferDeletedMessage): {
                FreertosStreamBufferDeletedMessage v{};
                if (!read_fields(rr, v.handle))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosStreamBufferSendMessage): {
                FreertosStreamBufferSendMessage v{};
                if (!read_fields(rr, v.handle, v.bytesSent, v.isFromISR))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosStreamBufferReceiveMessage): {
                FreertosStreamBufferReceiveMessage v{};
                if (!read_fields(rr, v.handle, v.bytesReceived, v.isFromISR))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosStreamBufferResetMessage): {
                FreertosStreamBufferResetMessage v{};
                if (!read_fields(rr, v.handle, v.isFromISR))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosIsrEnterMessage): {
                FreertosIsrEnterMessage v{};
                if (!read_fields(rr, v.isrNumber))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosIsrExitMessage): {
                FreertosIsrExitMessage v{};
                if (!read_fields(rr, v.isrNumber))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosIsrExitToSchedulerMessage): {
                FreertosIsrExitToSchedulerMessage v{};
                if (!read_fields(rr, v.isrNumber))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosQueueDeletedMessage): {
                FreertosQueueDeletedMessage v{};
                if (!read_fields(rr, v.handle))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosQueueRegistryMessage): {
                FreertosQueueRegistryMessage v{};
                if (!read_fields(rr, v.handle, v.name))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosQueueCreateFailedMessage): {
                FreertosQueueCreateFailedMessage v{};
                if (!read_fields(rr, v.queueType))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosQueueSendFailedMessage): {
                FreertosQueueSendFailedMessage v{};
                if (!read_fields(rr, v.handle, v.queueType, v.isFromISR))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosQueueReceiveFailedMessage): {
                FreertosQueueReceiveFailedMessage v{};
                if (!read_fields(rr, v.handle, v.queueType, v.isFromISR))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosQueuePeekFailedMessage): {
                FreertosQueuePeekFailedMessage v{};
                if (!read_fields(rr, v.handle, v.queueType, v.isFromISR))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosTimerCreateFailedMessage): {
                FreertosTimerCreateFailedMessage v{};
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosTimerCommandReceivedMessage): {
                FreertosTimerCommandReceivedMessage v{};
                if (!read_fields(rr, v.handle, v.commandId, v.optionalValue))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosStreamBufferCreateFailedMessage): {
                FreertosStreamBufferCreateFailedMessage v{};
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosStreamBufferSendFailedMessage): {
                FreertosStreamBufferSendFailedMessage v{};
                if (!read_fields(rr, v.handle, v.isFromISR))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosStreamBufferReceiveFailedMessage): {
                FreertosStreamBufferReceiveFailedMessage v{};
                if (!read_fields(rr, v.handle, v.isFromISR))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosEventGroupCreateFailedMessage): {
                FreertosEventGroupCreateFailedMessage v{};
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosTaskDelayMessage): {
                FreertosTaskDelayMessage v{};
                if (!read_fields(rr, v.handle, v.ticksToDelay))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            case static_cast<uint8_t>(MessageType::FreertosTaskDelayUntilMessage): {
                FreertosTaskDelayUntilMessage v{};
                if (!read_fields(rr, v.handle, v.timeToWake))
                    return false;
                handler_.HandleMessage(v);
                return true;
            }
            default:
                break;
            }
            return false;
        };
        // Transactional decode: try on a copy, commit on success
        Reader r_try = r;
        bool ok = try_decode(type, r_try);
        if (!ok) {
            // Incomplete or unknown message; wait for more data
            break;
        }
        // Commit consumed bytes
        r = r_try;

        // If we reach here, message decoded successfully.
        handler_.OnCycleCount(cycles);
        offset += r.pos;
        DEC_DEBUG("decoded type=" << unsigned(type) << " (" << message_type_name(type) << ") consumed=" << r.pos);
    }
    if (offset > 0) {
        streamBuffer.erase(streamBuffer.begin(), streamBuffer.begin() + static_cast<long>(offset));
    }
}
