#include "SporHost.hpp"

#include <cstring>
#include <iostream>
#include <memory>

#include "PerfettoApi.hpp"
#include "symbol-resolver/SymbolResolver.hpp"

namespace {
std::unique_ptr<MessageDecoder> decoderInstance;
}

MessageDecoder *GetSporInstance() {
    if (!decoderInstance) {
        decoderInstance = std::make_unique<MessageDecoder>(SporHost::GetInstance());
    }
    return decoderInstance.get();
}

void SporHost::HandleMessage(const ZoneBeginData &msg) {
    // profiler::PerfettoApi::ZoneBegin(nullptr, nullptr, nullptr, 0, 0);
}

void SporHost::HandleMessage(const ZoneEndData &msg) {
    // profiler::PerfettoApi::ZoneEnd();
}

void SporHost::HandleMessage(const ZoneTextMessage &msg) {
    // if (msg.text.HasData() && msg.text.IsString()) {
    //     const auto &text = msg.text.AsString();
    //     profiler::PerfettoApi::SetZoneText(text.c_str(), text.length());
    // }
}

void SporHost::HandleMessage(const ZoneValueData &msg) {
    // profiler::PerfettoApi::SetZoneValue(static_cast<int64_t>(msg.value));
}

void SporHost::HandleMessage(const ZoneColorData &msg) {
    // uint32_t color =
    //     static_cast<uint32_t>(msg.r) | (static_cast<uint32_t>(msg.g) << 8) | (static_cast<uint32_t>(msg.b) << 16);
    // profiler::PerfettoApi::SetZoneColor(color);
}

void SporHost::HandleMessage(const PlotMessage &msg) {
    if (msg.name.HasData() && msg.name.IsString()) {
        profiler::PerfettoApi::Plot(msg.name.AsString(), msg.data.value);
    }
}

void SporHost::HandleMessage(const PlotConfigMessage &msg) {
    if (msg.name.HasData() && msg.name.IsString()) {
        profiler::PerfettoApi::PlotConfig(
            msg.name.AsString(), msg.data.type, msg.data.step != 0, msg.data.fill != 0, msg.data.color
        );
    }
}

void SporHost::HandleMessage(const MessageTextMessage &msg) {
    if (msg.text.HasData() && msg.text.IsString()) {
        const auto &text = msg.text.AsString();
        profiler::PerfettoApi::Message(text, msg.data.color);
    }
}

void SporHost::HandleMessage(const AllocMessage &msg) {
    std::string_view name;
    if (msg.name.HasData() && msg.name.IsString()) {
        name = msg.name.AsString();
    }
    profiler::PerfettoApi::Alloc(
        reinterpret_cast<const void *>(static_cast<uintptr_t>(msg.data.ptr)), msg.data.size, name
    );
}

void SporHost::HandleMessage(const FreeMessage &msg) {
    std::string_view name;
    if (msg.name.HasData() && msg.name.IsString()) {
        name = msg.name.AsString();
    }
    profiler::PerfettoApi::Free(reinterpret_cast<const void *>(static_cast<uintptr_t>(msg.data.ptr)), name);
}

void SporHost::HandleMessage(const FreertosTaskCreatedMessage &msg) {
    if (msg.name.HasData() && msg.name.IsString()) {
        const auto &name = msg.name.AsString();
        tasks[msg.handle] = {name, msg.handle};
        profiler::PerfettoApi::RegisterThread(msg.handle, name);
    }
}

void SporHost::HandleMessage(const FunctionTraceEnterData &msg) {
    State::FunctionEnter(msg.fn);
}

void SporHost::HandleMessage(const FunctionTraceExitData &msg) {
    State::FunctionExit(msg.fn);
}

void SporHost::HandleMessage(const DeclarePointerTypeMessage &msg) {
    auto typeInfoAddr = msg.typeInfo;
    if (typeInfoAddr) {
        auto symbolInfo = profiler::GetResolvedSymbolInfo(typeInfoAddr);
        if (!symbolInfo.name.empty()) {
            pointerTypes[msg.ptr] = symbolInfo.name;
        }
    }
}

void SporHost::HandleMessage(const FlowBeginMessage &msg) {
    std::string name = "";
    auto it = pointerTypes.find(msg.ptr);
    if (it != pointerTypes.end()) {
        name = it->second;
    }
    profiler::PerfettoApi::FlowBegin(msg.ptr, name);
}

void SporHost::HandleMessage(const FlowEndMessage &msg) {
    profiler::PerfettoApi::FlowEnd(msg.ptr);
}

void SporHost::HandleMessage(const SystemInfoData &msg) {
    cpuFrequencyHz.store(static_cast<uint64_t>(msg.clock_frequency_mhz) * 1000000);
}

void SporHost::HandleMessage(const InterruptConfigMessage &msg) {
    std::string description = "IRQ ";
    if (msg.irq_name.HasData() && msg.irq_name.IsString()) {
        description += msg.irq_name.AsString();
    } else {
        description += std::to_string(msg.data.irq_number);
    }
    description += " configured";
    profiler::PerfettoApi::Message(description);
}

void SporHost::HandleMessage(const InterruptEnterData &msg) {
    if (msg.irq_number == 59) {
        abort();
    }
    // // Resolve IRQ name from device info and ensure it is registered as a thread
    // std::string irqName = "IRQ_" + std::to_string(msg.irq_number);
    // auto it = deviceInfo.irq_table.find(static_cast<DeviceInfo::IrqNumber>(msg.irq_number));
    // if (it != deviceInfo.irq_table.end()) {
    //     std::cout << irqName << " > " << std::string(it->second);
    //     irqName = std::string(it->second);
    // }

    // profiler::PerfettoApi::EnsureThreadRegistered(msg.irq_number, irqName.c_str());
    profiler::PerfettoApi::EnterIrq(msg.irq_number);
}

void SporHost::HandleMessage(const InterruptExitData &msg) {
    // Handle interrupt exit
    profiler::PerfettoApi::ExitIrq(msg.irq_number);
}

void SporHost::OnConsoleLog(const void *data, size_t length) {
    if (length > 0) {
        std::string message(static_cast<const char *>(data), length);
        profiler::PerfettoApi::Message(message, true);
    }
}

void SporHost::OnCycleCount(uint32_t cycles) {
    static uint32_t lastCycles = cycles;
    static uint64_t lastNanoseconds = 0;
    uint32_t diff = cycles - lastCycles;
    lastNanoseconds += diff * 5; // Todo
    profiler::SetTime(lastNanoseconds);
    lastCycles = cycles;

    // if (cycles < lastTimestamp) {
    //     std::cerr << "Timestamps are not monotonic" << std::endl;
    //     throw std::runtime_error("Timestamps are not monotonic");
    // }
    // profiler::SetTime((timestamp - initialTimestamp) * TIMESTAMP_TO_NANOSECOONDS);
    // lastTimestamp = timestamp;
}

void SporHost::HandleMessage(const PointerAnnounceMessage &msg) {
    uint32_t symbolAddr = msg.symbolPointer;
    uint32_t heapAddr = msg.heapPointer;

    auto symbolInfo = profiler::GetResolvedSymbolInfo(symbolAddr);
    if (!symbolInfo.name.empty()) {
        pointerNames[heapAddr] = symbolInfo.name;

        // Send a message to the trace system
        std::string message = "Pointer announced: " + symbolInfo.name + " at 0x" + std::to_string(heapAddr);
        // profiler::PerfettoApi::Message(message);

        std::cout << "Pointer name set: " << message << std::endl;
    }
}

void SporHost::HandleMessage(const PointerSetNameMessage &msg) {
    uint32_t ptr = msg.ptr;

    auto str = msg.name.GetString();
    pointerNames[ptr] = str;

    std::cout << "Pointer name set: " << std::hex << msg.name.AsSymbol() << " = " << str << std::endl;

    // TODO make automatically derive name from pointerNames
    profiler::PerfettoApi::lockables[ptr].name = str;
}

#include "SporHostFreertos.cxx"
