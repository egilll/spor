#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>

#include "spor-common/TargetPointer.hpp"

#include "utils/StringWire.hpp"
#include "MessagesFreertos.hpp"
// Using a minimalist streaming scheme; writers/readers are provided by
// target/host code without a shared serialization library.

struct ZoneBeginData {
    uint32_t ptr;
};

struct ZoneEndData {
    uint32_t ptr;
};

struct ZoneValueData {
    uint64_t value;
};

struct ZoneColorData {
    uint8_t r, g, b;
};

struct PlotData {
    int64_t value;
};

struct PlotConfigData {
    int32_t type;
    uint8_t step;
    uint8_t fill;
    uint32_t color;
};

// Removed MessageData; we only carry text for MessageTextMessage.

struct AllocData {
    TargetPointer ptr;
    uint32_t size;
};

struct FreeData {
    TargetPointer ptr;
};

struct SystemInfoData {
    uint32_t clock_frequency_mhz;
};

struct InterruptConfigData {
    uint16_t irq_number;
    uint16_t irq_priority_level;
};

struct InterruptEnterData {
    uint16_t irq_number;
    uint32_t enabled_flags;
};

struct InterruptExitData {
    uint16_t irq_number;
};

struct FunctionTraceEnterData {
    uint32_t fn;
};

struct FunctionTraceExitData {
    uint32_t fn;
};

struct FlowBeginData {
    uint32_t flow_id;
};

struct FlowEndData {
    uint32_t flow_id;
};

struct PointerAnnounceMessage {
    TargetPointer symbolPointer;
    TargetPointer heapPointer;
};

struct PointerSetNameMessage {
    TargetPointer ptr;
    TextField name;
};

struct ZoneTextMessage {
    TextField text;
};

struct PlotMessage {
    PlotData data;
    TextField name;
};

struct PlotConfigMessage {
    PlotConfigData data;
    TextField name;
};

#ifdef SPOR_HOST
struct MessageTextMessage {
    std::string text;
};
#else
struct MessageTextMessage {
    const char *ptr;
    uint32_t len;
};
#endif

struct AllocMessage {
    AllocData data;
    TextField name;
};

struct FreeMessage {
    FreeData data;
    TextField name;
};

struct DeclarePointerTypeMessage {
    TargetPointer ptr;
    TargetPointer typeInfo;
};

struct FlowBeginMessage {
    TargetPointer ptr;
};

struct FlowEndMessage {
    TargetPointer ptr;
};

struct InterruptConfigMessage {
    InterruptConfigData data;
    TextField irq_name;
    const char *const *flag_names;
};

using Message = ToVariant_t<
    TypeList<
        ZoneBeginData,
        ZoneEndData,
        ZoneTextMessage,
        ZoneValueData,
        ZoneColorData,
        PlotMessage,
        PlotConfigMessage,
        MessageTextMessage,
        AllocMessage,
        FreeMessage,
        FunctionTraceEnterData,
        FunctionTraceExitData,
        DeclarePointerTypeMessage,
        FlowBeginMessage,
        FlowEndMessage,
        SystemInfoData,
        InterruptConfigMessage,
        InterruptEnterData,
        InterruptExitData,
        PointerAnnounceMessage,
        PointerSetNameMessage>,
    FreertosMessages>;

enum class MessageType : uint8_t {
    ZoneBeginData = 0,
    ZoneEndData,
    ZoneTextMessage,
    ZoneValueData,
    ZoneColorData,
    PlotMessage,
    PlotConfigMessage,
    MessageTextMessage,
    AllocMessage,
    FreeMessage,
    FunctionTraceEnterData,
    FunctionTraceExitData,
    DeclarePointerTypeMessage,
    FlowBeginMessage,
    FlowEndMessage,
    SystemInfoData,
    InterruptConfigMessage,
    InterruptEnterData,
    InterruptExitData,
    PointerAnnounceMessage,
    PointerSetNameMessage,
    FreertosTaskCreatedMessage,
    FreertosTaskSwitchedInMessage,
    FreertosTaskSwitchedOutMessage,
    FreertosTaskReadiedMessage,
    FreertosQueueCreatedMessage,
    FreertosQueuePeekMessage,
    FreertosQueueSendMessage,
    FreertosQueueReceiveMessage,
    FreertosTaskNotifyMessage,
    FreertosTaskNotifyReceivedMessage,
    FreertosTaskDeletedMessage,
    FreertosTaskPrioritySetMessage,
    FreertosTaskSuspendMessage,
    FreertosTaskResumeMessage,
    FreertosTimerCreatedMessage,
    FreertosTimerCommandMessage,
    FreertosTimerExpiredMessage,
    FreertosEventGroupCreatedMessage,
    FreertosEventGroupDeletedMessage,
    FreertosEventGroupSyncMessage,
    FreertosEventGroupWaitBitsMessage,
    FreertosEventGroupClearBitsMessage,
    FreertosEventGroupSetBitsMessage,
    FreertosStreamBufferCreatedMessage,
    FreertosStreamBufferDeletedMessage,
    FreertosStreamBufferSendMessage,
    FreertosStreamBufferReceiveMessage,
    FreertosStreamBufferResetMessage,
    FreertosIsrEnterMessage,
    FreertosIsrExitMessage,
    FreertosIsrExitToSchedulerMessage,
    FreertosQueueDeletedMessage,
    FreertosQueueRegistryMessage,
    FreertosQueueCreateFailedMessage,
    FreertosQueueSendFailedMessage,
    FreertosQueueReceiveFailedMessage,
    FreertosQueuePeekFailedMessage,
    FreertosTimerCreateFailedMessage,
    FreertosTimerCommandReceivedMessage,
    FreertosStreamBufferCreateFailedMessage,
    FreertosStreamBufferSendFailedMessage,
    FreertosStreamBufferReceiveFailedMessage,
    FreertosEventGroupCreateFailedMessage,
    FreertosTaskDelayMessage,
    FreertosTaskDelayUntilMessage,
};

template <typename T>
struct MessageTag;

#define SPOR_MAP_MSG(TAG, TYPE) \
    template <> struct MessageTag<TYPE> { static constexpr MessageType value = MessageType::TAG; };

SPOR_MAP_MSG(ZoneBeginData, ZoneBeginData)
SPOR_MAP_MSG(ZoneEndData, ZoneEndData)
SPOR_MAP_MSG(ZoneTextMessage, ZoneTextMessage)
SPOR_MAP_MSG(ZoneValueData, ZoneValueData)
SPOR_MAP_MSG(ZoneColorData, ZoneColorData)
SPOR_MAP_MSG(PlotMessage, PlotMessage)
SPOR_MAP_MSG(PlotConfigMessage, PlotConfigMessage)
SPOR_MAP_MSG(MessageTextMessage, MessageTextMessage)
SPOR_MAP_MSG(AllocMessage, AllocMessage)
SPOR_MAP_MSG(FreeMessage, FreeMessage)
SPOR_MAP_MSG(FunctionTraceEnterData, FunctionTraceEnterData)
SPOR_MAP_MSG(FunctionTraceExitData, FunctionTraceExitData)
SPOR_MAP_MSG(DeclarePointerTypeMessage, DeclarePointerTypeMessage)
SPOR_MAP_MSG(FlowBeginMessage, FlowBeginMessage)
SPOR_MAP_MSG(FlowEndMessage, FlowEndMessage)
SPOR_MAP_MSG(SystemInfoData, SystemInfoData)
SPOR_MAP_MSG(InterruptConfigMessage, InterruptConfigMessage)
SPOR_MAP_MSG(InterruptEnterData, InterruptEnterData)
SPOR_MAP_MSG(InterruptExitData, InterruptExitData)
SPOR_MAP_MSG(PointerAnnounceMessage, PointerAnnounceMessage)
SPOR_MAP_MSG(PointerSetNameMessage, PointerSetNameMessage)

SPOR_MAP_MSG(FreertosTaskCreatedMessage, FreertosTaskCreatedMessage)
SPOR_MAP_MSG(FreertosTaskSwitchedInMessage, FreertosTaskSwitchedInMessage)
SPOR_MAP_MSG(FreertosTaskSwitchedOutMessage, FreertosTaskSwitchedOutMessage)
SPOR_MAP_MSG(FreertosTaskReadiedMessage, FreertosTaskReadiedMessage)
SPOR_MAP_MSG(FreertosQueueCreatedMessage, FreertosQueueCreatedMessage)
SPOR_MAP_MSG(FreertosQueuePeekMessage, FreertosQueuePeekMessage)
SPOR_MAP_MSG(FreertosQueueSendMessage, FreertosQueueSendMessage)
SPOR_MAP_MSG(FreertosQueueReceiveMessage, FreertosQueueReceiveMessage)
SPOR_MAP_MSG(FreertosTaskNotifyMessage, FreertosTaskNotifyMessage)
SPOR_MAP_MSG(FreertosTaskNotifyReceivedMessage, FreertosTaskNotifyReceivedMessage)
SPOR_MAP_MSG(FreertosTaskDeletedMessage, FreertosTaskDeletedMessage)
SPOR_MAP_MSG(FreertosTaskPrioritySetMessage, FreertosTaskPrioritySetMessage)
SPOR_MAP_MSG(FreertosTaskSuspendMessage, FreertosTaskSuspendMessage)
SPOR_MAP_MSG(FreertosTaskResumeMessage, FreertosTaskResumeMessage)
SPOR_MAP_MSG(FreertosTimerCreatedMessage, FreertosTimerCreatedMessage)
SPOR_MAP_MSG(FreertosTimerCommandMessage, FreertosTimerCommandMessage)
SPOR_MAP_MSG(FreertosTimerExpiredMessage, FreertosTimerExpiredMessage)
SPOR_MAP_MSG(FreertosEventGroupCreatedMessage, FreertosEventGroupCreatedMessage)
SPOR_MAP_MSG(FreertosEventGroupDeletedMessage, FreertosEventGroupDeletedMessage)
SPOR_MAP_MSG(FreertosEventGroupSyncMessage, FreertosEventGroupSyncMessage)
SPOR_MAP_MSG(FreertosEventGroupWaitBitsMessage, FreertosEventGroupWaitBitsMessage)
SPOR_MAP_MSG(FreertosEventGroupClearBitsMessage, FreertosEventGroupClearBitsMessage)
SPOR_MAP_MSG(FreertosEventGroupSetBitsMessage, FreertosEventGroupSetBitsMessage)
SPOR_MAP_MSG(FreertosStreamBufferCreatedMessage, FreertosStreamBufferCreatedMessage)
SPOR_MAP_MSG(FreertosStreamBufferDeletedMessage, FreertosStreamBufferDeletedMessage)
SPOR_MAP_MSG(FreertosStreamBufferSendMessage, FreertosStreamBufferSendMessage)
SPOR_MAP_MSG(FreertosStreamBufferReceiveMessage, FreertosStreamBufferReceiveMessage)
SPOR_MAP_MSG(FreertosStreamBufferResetMessage, FreertosStreamBufferResetMessage)
SPOR_MAP_MSG(FreertosIsrEnterMessage, FreertosIsrEnterMessage)
SPOR_MAP_MSG(FreertosIsrExitMessage, FreertosIsrExitMessage)
SPOR_MAP_MSG(FreertosIsrExitToSchedulerMessage, FreertosIsrExitToSchedulerMessage)
SPOR_MAP_MSG(FreertosQueueDeletedMessage, FreertosQueueDeletedMessage)
SPOR_MAP_MSG(FreertosQueueRegistryMessage, FreertosQueueRegistryMessage)
SPOR_MAP_MSG(FreertosQueueCreateFailedMessage, FreertosQueueCreateFailedMessage)
SPOR_MAP_MSG(FreertosQueueSendFailedMessage, FreertosQueueSendFailedMessage)
SPOR_MAP_MSG(FreertosQueueReceiveFailedMessage, FreertosQueueReceiveFailedMessage)
SPOR_MAP_MSG(FreertosQueuePeekFailedMessage, FreertosQueuePeekFailedMessage)
SPOR_MAP_MSG(FreertosTimerCreateFailedMessage, FreertosTimerCreateFailedMessage)
SPOR_MAP_MSG(FreertosTimerCommandReceivedMessage, FreertosTimerCommandReceivedMessage)
SPOR_MAP_MSG(FreertosStreamBufferCreateFailedMessage, FreertosStreamBufferCreateFailedMessage)
SPOR_MAP_MSG(FreertosStreamBufferSendFailedMessage, FreertosStreamBufferSendFailedMessage)
SPOR_MAP_MSG(FreertosStreamBufferReceiveFailedMessage, FreertosStreamBufferReceiveFailedMessage)
SPOR_MAP_MSG(FreertosEventGroupCreateFailedMessage, FreertosEventGroupCreateFailedMessage)
SPOR_MAP_MSG(FreertosTaskDelayMessage, FreertosTaskDelayMessage)
SPOR_MAP_MSG(FreertosTaskDelayUntilMessage, FreertosTaskDelayUntilMessage)

#undef SPOR_MAP_MSG

// Wire-encoding helpers for writer-side (target).
namespace spor::wire {

template <typename W>
inline void write(W &w, const TextField &t) {
#ifdef SPOR_HOST
    // Host side sends/handles only zero-terminated strings
    w.put_stringz(t.c_str());
#else
    // Target side: TextField is const char*, always encode inline zero-terminated string
    const char *s = t ? t : "";
    w.put_stringz(s);
#endif
}

template <typename W> inline void write(W &w, const ZoneBeginData &v) { w.put_u32(v.ptr); }
template <typename W> inline void write(W &w, const ZoneEndData &v) { (void)v; }
template <typename W> inline void write(W &w, const ZoneTextMessage &v) { write(w, v.text); }
template <typename W> inline void write(W &w, const ZoneValueData &v) { w.put_u64(static_cast<uint64_t>(v.value)); }
template <typename W> inline void write(W &w, const ZoneColorData &v) {
    w.put_u8(v.r);
    w.put_u8(v.g);
    w.put_u8(v.b);
}
template <typename W> inline void write(W &w, const PlotMessage &v) { w.put_s64(v.data.value); write(w, v.name); }
template <typename W> inline void write(W &w, const PlotConfigMessage &v) {
    w.put_s32(v.data.type);
    w.put_u8(v.data.step);
    w.put_u8(v.data.fill);
    w.put_u32(v.data.color);
    write(w, v.name);
}
#ifdef SPOR_HOST
template <typename W> inline void write(W &w, const MessageTextMessage &v) { w.put_stringz(v.text.c_str()); }
#else
template <typename W> inline void write(W &w, const MessageTextMessage &v) {
    // Write 'len' bytes from ptr, then a terminating zero byte.
    for (uint32_t i = 0; i < v.len; ++i) {
        w.put_u8(static_cast<uint8_t>(v.ptr[i]));
    }
    w.put_u8(0);
}
#endif
template <typename W> inline void write(W &w, const AllocMessage &v) {
    w.put_u32(v.data.ptr);
    w.put_u32(v.data.size);
    write(w, v.name);
}
template <typename W> inline void write(W &w, const FreeMessage &v) { w.put_u32(v.data.ptr); write(w, v.name); }
template <typename W> inline void write(W &w, const FunctionTraceEnterData &v) { w.put_u32(v.fn); }
template <typename W> inline void write(W &w, const FunctionTraceExitData &v) { w.put_u32(v.fn); }
template <typename W> inline void write(W &w, const DeclarePointerTypeMessage &v) { w.put_u32(v.ptr); w.put_u32(v.typeInfo); }
template <typename W> inline void write(W &w, const FlowBeginMessage &v) { w.put_u32(v.ptr); }
template <typename W> inline void write(W &w, const FlowEndMessage &v) { w.put_u32(v.ptr); }
template <typename W> inline void write(W &w, const SystemInfoData &v) { w.put_u32(v.clock_frequency_mhz); }
template <typename W> inline void write(W &w, const InterruptConfigMessage &v) {
    w.put_u32(v.data.irq_number);
    w.put_u32(v.data.irq_priority_level);
    write(w, v.irq_name);
}
template <typename W> inline void write(W &w, const InterruptEnterData &v) {
    w.put_u32(v.irq_number);
    w.put_u32(v.enabled_flags);
}
template <typename W> inline void write(W &w, const InterruptExitData &v) { w.put_u32(v.irq_number); }
template <typename W> inline void write(W &w, const PointerAnnounceMessage &v) {
    w.put_u32(v.symbolPointer);
    w.put_u32(v.heapPointer);
}
template <typename W> inline void write(W &w, const PointerSetNameMessage &v) { w.put_u32(v.ptr); write(w, v.name); }

} // namespace spor::wire

// FreeRTOS message encoders
namespace spor::wire {

template <typename W> inline void write(W &w, const FreertosTaskCreatedMessage &v) {
    w.put_u32(v.handle);
    write(w, v.name);
    w.put_u32(v.priority);
}
template <typename W> inline void write(W &w, const FreertosTaskSwitchedInMessage &v) { w.put_u32(v.handle); }
template <typename W> inline void write(W &w, const FreertosTaskSwitchedOutMessage &v) {
    w.put_u32(v.handle);
    w.put_u8(static_cast<uint8_t>(v.switchReason));
    w.put_u32(v.blockedOnObject);
    w.put_u8(v.stillReady ? 1 : 0);
}
template <typename W> inline void write(W &w, const FreertosTaskReadiedMessage &v) { w.put_u32(v.handle); }
template <typename W> inline void write(W &w, const FreertosQueueCreatedMessage &v) {
    w.put_u32(v.handle);
    w.put_u32(v.capacity);
    w.put_u8(static_cast<uint8_t>(v.queueType));
}
template <typename W> inline void write(W &w, const FreertosQueuePeekMessage &v) {
    w.put_u32(v.handle);
    w.put_u32(v.updatedCount);
    w.put_u8(v.isFromISR ? 1 : 0);
}
template <typename W> inline void write(W &w, const FreertosQueueSendMessage &v) {
    w.put_u32(v.handle);
    w.put_u32(v.updatedCount);
    w.put_u8(static_cast<uint8_t>(v.queueType));
    w.put_u8(v.isFromISR ? 1 : 0);
}
template <typename W> inline void write(W &w, const FreertosQueueReceiveMessage &v) {
    w.put_u32(v.handle);
    w.put_u32(v.updatedCount);
    w.put_u8(static_cast<uint8_t>(v.queueType));
    w.put_u8(v.isFromISR ? 1 : 0);
}
template <typename W> inline void write(W &w, const FreertosQueueSendFailedMessage &v) {
    w.put_u32(v.handle);
    w.put_u8(static_cast<uint8_t>(v.queueType));
    w.put_u8(v.isFromISR ? 1 : 0);
}
template <typename W> inline void write(W &w, const FreertosQueueReceiveFailedMessage &v) {
    w.put_u32(v.handle);
    w.put_u8(static_cast<uint8_t>(v.queueType));
    w.put_u8(v.isFromISR ? 1 : 0);
}
template <typename W> inline void write(W &w, const FreertosQueuePeekFailedMessage &v) {
    w.put_u32(v.handle);
    w.put_u8(static_cast<uint8_t>(v.queueType));
    w.put_u8(v.isFromISR ? 1 : 0);
}
template <typename W> inline void write(W &w, const FreertosTaskNotifyMessage &v) {
    w.put_u32(v.handle);
    w.put_u32(v.index);
    w.put_u32(v.action);
    w.put_u32(v.updatedValue);
    w.put_u8(v.isFromISR ? 1 : 0);
}
template <typename W> inline void write(W &w, const FreertosTaskNotifyReceivedMessage &v) {
    w.put_u32(v.handle);
    w.put_u32(v.index);
    w.put_u32(v.updatedValue);
}
template <typename W> inline void write(W &w, const FreertosTaskDeletedMessage &v) { w.put_u32(v.handle); }
template <typename W> inline void write(W &w, const FreertosTaskPrioritySetMessage &v) {
    w.put_u32(v.handle);
    w.put_u32(v.oldPriority);
    w.put_u32(v.newPriority);
}
template <typename W> inline void write(W &w, const FreertosTaskSuspendMessage &v) { w.put_u32(v.handle); }
template <typename W> inline void write(W &w, const FreertosTaskResumeMessage &v) {
    w.put_u32(v.handle);
    w.put_u8(v.isFromISR ? 1 : 0);
}
template <typename W> inline void write(W &w, const FreertosTimerCreatedMessage &v) {
    w.put_u32(v.handle);
    w.put_u32(v.period);
    w.put_u8(v.autoReload ? 1 : 0);
}
template <typename W> inline void write(W &w, const FreertosTimerCommandMessage &v) {
    w.put_u32(v.handle);
    w.put_u32(v.commandId);
    w.put_u32(v.optionalValue);
    w.put_u8(v.isFromISR ? 1 : 0);
}
template <typename W> inline void write(W &w, const FreertosTimerExpiredMessage &v) { w.put_u32(v.handle); }
template <typename W> inline void write(W &w, const FreertosEventGroupCreatedMessage &v) { w.put_u32(v.handle); }
template <typename W> inline void write(W &w, const FreertosEventGroupDeletedMessage &v) { w.put_u32(v.handle); }
template <typename W> inline void write(W &w, const FreertosEventGroupSyncMessage &v) {
    w.put_u32(v.handle);
    w.put_u32(v.setBits);
    w.put_u32(v.waitBits);
    w.put_u32(v.resultBits);
}
template <typename W> inline void write(W &w, const FreertosEventGroupWaitBitsMessage &v) {
    w.put_u32(v.handle);
    w.put_u32(v.waitBits);
    w.put_u32(v.resultBits);
}
template <typename W> inline void write(W &w, const FreertosEventGroupClearBitsMessage &v) {
    w.put_u32(v.handle);
    w.put_u32(v.clearBits);
    w.put_u8(v.isFromISR ? 1 : 0);
}
template <typename W> inline void write(W &w, const FreertosEventGroupSetBitsMessage &v) {
    w.put_u32(v.handle);
    w.put_u32(v.setBits);
    w.put_u8(v.isFromISR ? 1 : 0);
}
template <typename W> inline void write(W &w, const FreertosStreamBufferCreatedMessage &v) {
    w.put_u32(v.handle);
    w.put_u32(v.size);
    w.put_u32(v.triggerLevel);
    w.put_u8(v.isMessageBuffer ? 1 : 0);
}
template <typename W> inline void write(W &w, const FreertosStreamBufferDeletedMessage &v) { w.put_u32(v.handle); }
template <typename W> inline void write(W &w, const FreertosStreamBufferSendMessage &v) {
    w.put_u32(v.handle);
    w.put_u32(v.bytesSent);
    w.put_u8(v.isFromISR ? 1 : 0);
}
template <typename W> inline void write(W &w, const FreertosStreamBufferReceiveMessage &v) {
    w.put_u32(v.handle);
    w.put_u32(v.bytesReceived);
    w.put_u8(v.isFromISR ? 1 : 0);
}
template <typename W> inline void write(W &w, const FreertosStreamBufferResetMessage &v) {
    w.put_u32(v.handle);
    w.put_u8(v.isFromISR ? 1 : 0);
}
template <typename W> inline void write(W &w, const FreertosIsrEnterMessage &v) { w.put_u32(v.isrNumber); }
template <typename W> inline void write(W &w, const FreertosIsrExitMessage &v) { w.put_u32(v.isrNumber); }
template <typename W> inline void write(W &w, const FreertosIsrExitToSchedulerMessage &v) { w.put_u32(v.isrNumber); }
template <typename W> inline void write(W &w, const FreertosQueueDeletedMessage &v) { w.put_u32(v.handle); }
template <typename W> inline void write(W &w, const FreertosQueueRegistryMessage &v) { w.put_u32(v.handle); write(w, v.name); }
template <typename W> inline void write(W &w, const FreertosQueueCreateFailedMessage &v) {
    w.put_u8(static_cast<uint8_t>(v.queueType));
}
template <typename W> inline void write(W &w, const FreertosTimerCreateFailedMessage &) {}
template <typename W> inline void write(W &w, const FreertosTimerCommandReceivedMessage &v) {
    w.put_u32(v.handle);
    w.put_u32(v.commandId);
    w.put_u32(v.optionalValue);
}
template <typename W> inline void write(W &w, const FreertosStreamBufferCreateFailedMessage &) {}
template <typename W> inline void write(W &w, const FreertosStreamBufferSendFailedMessage &v) {
    w.put_u32(v.handle);
    w.put_u8(v.isFromISR ? 1 : 0);
}
template <typename W> inline void write(W &w, const FreertosStreamBufferReceiveFailedMessage &v) {
    w.put_u32(v.handle);
    w.put_u8(v.isFromISR ? 1 : 0);
}
template <typename W> inline void write(W &w, const FreertosEventGroupCreateFailedMessage &) {}
template <typename W> inline void write(W &w, const FreertosTaskDelayMessage &v) { w.put_u32(v.handle); w.put_u32(v.ticksToDelay); }
template <typename W> inline void write(W &w, const FreertosTaskDelayUntilMessage &v) { w.put_u32(v.handle); w.put_u32(v.timeToWake); }

} // namespace spor::wire

// Also support bare AllocData/FreeData, which are sent directly in some paths
namespace spor::wire {

template <typename W> inline void write(W &w, const AllocData &v) { w.put_u32(v.ptr); w.put_u32(v.size); }
template <typename W> inline void write(W &w, const FreeData &v) { w.put_u32(v.ptr); }

} // namespace spor::wire
