#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>

#include "spor-common/TargetPointer.hpp"

// bool IsSymbolInROM(const char *ptr);
#include "Channels.hpp"
#include "MessagesFreertos.hpp"
#include "utils/String.hpp"

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

struct MessageData {
    uint32_t color;
    uint32_t length;
};

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
    StringOrSymbol name;

    constexpr static auto serialize(auto &archive, auto &self) {
        return archive(self.ptr, self.name);
    }
};

struct ZoneTextMessage {
    StringOrSymbol text;

    constexpr static auto serialize(auto &archive, auto &self) {
        return archive(self.text);
    }
};

struct PlotMessage {
    PlotData data;
    StringOrSymbol name;

    constexpr static auto serialize(auto &archive, auto &self) {
        return archive(self.data, self.name);
    }
};

struct PlotConfigMessage {
    PlotConfigData data;
    StringOrSymbol name;

    constexpr static auto serialize(auto &archive, auto &self) {
        return archive(self.data, self.name);
    }
};

struct MessageTextMessage {
    MessageData data;
    StringOrSymbol text;

    constexpr static auto serialize(auto &archive, auto &self) {
        return archive(self.data, self.text);
    }
};

struct AllocMessage {
    AllocData data;
    StringOrSymbol name;

    constexpr static auto serialize(auto &archive, auto &self) {
        return archive(self.data, self.name);
    }
};

struct FreeMessage {
    FreeData data;
    StringOrSymbol name;

    constexpr static auto serialize(auto &archive, auto &self) {
        return archive(self.data, self.name);
    }
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
    StringOrSymbol irq_name;
    const char *const *flag_names;

    constexpr static auto serialize(auto &archive, auto &self) {
        return archive(self.data, self.irq_name);
    }
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
