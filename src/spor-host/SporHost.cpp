#include "SporHost.hpp"

#include <chrono>
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
    if (!msg.name.empty()) {
        profiler::PerfettoApi::Plot(msg.name, msg.data.value);
    }
}

void SporHost::HandleMessage(const PlotConfigMessage &msg) {
    if (!msg.name.empty()) {
        profiler::PerfettoApi::PlotConfig(
            msg.name, msg.data.type, msg.data.step != 0, msg.data.fill != 0, msg.data.color
        );
    }
}

void SporHost::HandleMessage(const MessageTextMessage &msg) {
    if (!msg.text.empty()) {
        profiler::PerfettoApi::Message(msg.text, true);
    }
}

void SporHost::HandleMessage(const AllocMessage &msg) {
    std::string nameStr;
    std::string_view name;
    if (!msg.name.empty()) {
        nameStr = msg.name;
        name = nameStr;
    }
    profiler::PerfettoApi::Alloc(
        reinterpret_cast<const void *>(static_cast<uintptr_t>(msg.data.ptr)), msg.data.size, name
    );
}

void SporHost::HandleMessage(const FreeMessage &msg) {
    std::string nameStr;
    std::string_view name;
    if (!msg.name.empty()) {
        nameStr = msg.name;
        name = nameStr;
    }
    profiler::PerfettoApi::Free(reinterpret_cast<const void *>(static_cast<uintptr_t>(msg.data.ptr)), name);
}

void SporHost::HandleMessage(const FreertosTaskCreatedMessage &msg) {
    if (!msg.name.empty()) {
        auto name = msg.name;
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
    if (!msg.irq_name.empty()) {
        description += msg.irq_name;
    } else {
        description += std::to_string(msg.data.irq_number);
    }
    description += " configured";
    profiler::PerfettoApi::Message(description);
}

void SporHost::HandleMessage(const InterruptEnterData &msg) {
    // if (msg.irq_number == 59) {
    //     abort();
    // }
    //  // Resolve IRQ name from device info and ensure it is registered as a thread
    //  std::string irqName = "IRQ_" + std::to_string(msg.irq_number);
    //  auto it = deviceInfo.irq_table.find(static_cast<DeviceInfo::IrqNumber>(msg.irq_number));
    //  if (it != deviceInfo.irq_table.end()) {
    //      std::cout << irqName << " > " << std::string(it->second);
    //      irqName = std::string(it->second);
    //  }

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
    // Convert target cycle counter to trace time using configured CPU frequency.
    // Be resilient to wrap, resets, or long gaps by comparing to host wall clock
    // and clamping excessive deltas (treated as missing data).

    using namespace std::chrono;

    static bool initialized = false;
    static uint32_t lastCycles = 0;
    static uint64_t lastTraceNs = 0; // relative trace time (offset added in GetTime())
    static uint64_t lastHostNs = 0;  // host wall clock snapshot

    const uint64_t hostNowNs = duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();

    if (!initialized) {
        lastCycles = cycles;
        lastHostNs = hostNowNs;
        lastTraceNs = 0;
        profiler::SetTime(lastTraceNs);
        initialized = true;
        return;
    }

    // Compute cycle delta with wrap-around semantics.
    uint32_t diffCycles = cycles - lastCycles; // uint32 wraps naturally

    // Convert cycles -> nanoseconds using the configured frequency.
    uint64_t freqHz = cpuFrequencyHz.load();
    if (freqHz == 0) {
        // Avoid division by zero, keep previous time.
        profiler::SetTime(lastTraceNs);
        lastCycles = cycles;
        lastHostNs = hostNowNs;
        return;
    }

    uint64_t deltaFromCyclesNs = (static_cast<uint64_t>(diffCycles) * 1000000000ull) / freqHz;
    uint64_t hostDeltaNs = hostNowNs - lastHostNs;

    // Treat very large deltas as missing data and follow wall clock instead.
    // Heuristics:
    // - Absolute clamp: ignore cycle-based jumps > absClampNs
    // - Ratio clamp: if target delta >> host elapsed, assume gap
    const uint64_t absClampNs = 200000000ull; // 200 ms
    const uint64_t ratioMultiplier = 8;       // allow up to 8x host elapsed
    bool looksLikeGap = (deltaFromCyclesNs > absClampNs) ||
                        (hostDeltaNs > 0 && deltaFromCyclesNs > hostDeltaNs * ratioMultiplier + 1000000ull);

    uint64_t usedDeltaNs = looksLikeGap ? hostDeltaNs : deltaFromCyclesNs;

    lastTraceNs += usedDeltaNs;
    profiler::SetTime(lastTraceNs);

    lastCycles = cycles;
    lastHostNs = hostNowNs;
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

    auto str = msg.name;
    pointerNames[ptr] = str;

    std::cout << "Pointer name set: 0x" << std::hex << ptr << " = " << str << std::endl;

    // TODO make automatically derive name from pointerNames
    profiler::PerfettoApi::lockables[ptr].name = str;
}

#include "SporHostFreertos.cxx"
