#include "PerfettoApi.hpp"

#include <atomic>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <unistd.h>

#include "Logging.hpp"
#include "spor-devices/DeviceInfo.hpp"
#include "symbol-resolver/SymbolResolver.hpp"
#include "TimeUtils.hpp"

// PERFETTO_DECLARE_DATA_SOURCE_STATIC_MEMBERS(profiler::ThreadDataSource);

PERFETTO_DEFINE_CATEGORIES(
    perfetto::Category("scheduler").SetDescription("Thread scheduler events"),
    perfetto::Category("zones").SetDescription("Function zone events"),
    perfetto::Category("locks").SetDescription("Lock/mutex events"),
    perfetto::Category("memory").SetDescription("Memory allocation events"),
    perfetto::Category("irq").SetDescription("Interrupt events"),
    perfetto::Category("log").SetDescription("Log"),
);

PERFETTO_TRACK_EVENT_STATIC_STORAGE();

namespace profiler {

std::unordered_map<uint32_t, Lockable> PerfettoApi::lockables;
std::unordered_map<uint32_t, Thread> PerfettoApi::threads;
std::unique_ptr<SymbolResolver> PerfettoApi::symbolResolver = nullptr;
uint32_t PerfettoApi::currentThreadId = 0;

void InitializePerfetto() {
    perfetto::TracingInitArgs args;
    args.backends = perfetto::kInProcessBackend;
    perfetto::Tracing::Initialize(args);
    perfetto::TrackEvent::Register();

    perfetto::DataSourceDescriptor dsd;
    dsd.set_name("com.custom_data_source");
    ThreadDataSource::Register(dsd);
}

std::unique_ptr<perfetto::TracingSession> PerfettoApi::StartTracing() {
    InitializePerfetto();
    perfetto::TraceConfig cfg;

    auto *buffer = cfg.add_buffers();
    buffer->set_size_kb(100 * 1024);
    // buffer->set_fill_policy(perfetto::TraceConfig::BufferConfig::RING_BUFFER);

    {
        auto *ds_cfg = cfg.add_data_sources()->mutable_config();
        ds_cfg->set_name("com.custom_data_source");
    }
    {
        auto *ds_cfg = cfg.add_data_sources()->mutable_config();
        ds_cfg->set_name("track_event");
    }

    // cfg.set_output_path("output.pftrace");
    // cfg.set_write_into_file(true);

    // perfetto::protos::gen::TrackEventConfig te_cfg;
    // te_cfg.add_enabled_categories("*");
    // ds_cfg->set_track_event_config_raw(te_cfg.SerializeAsString());

    auto tracing_session = perfetto::Tracing::NewTrace();
    tracing_session->Setup(cfg);
    tracing_session->StartBlocking();
    return tracing_session;
}

void PerfettoApi::StopTracing(std::unique_ptr<perfetto::TracingSession> tracing_session) {
    // Flush all pending events
    // perfetto::TrackEvent::Flush();

    ThreadDataSource::Trace([](ThreadDataSource::TraceContext ctx) {
        ctx.Flush();
    });

    tracing_session->StopBlocking();
    std::vector<char> trace_data(tracing_session->ReadTraceBlocking());

    std::ofstream output;
    const char *filename = "/Users/egill/spor/data/trace.perfetto";
    output.open(filename, std::ios::out | std::ios::binary);
    output.write(&trace_data[0], static_cast<std::streamsize>(trace_data.size()));
    output.close();

    for (auto &thread : PerfettoApi::threads) {
        std::cout << thread.second.name << " " << thread.second.pid << std::endl;
    }

    PERFETTO_LOG("Trace written to %s", filename);
}

void PerfettoApi::CreateThread(uint32_t threadId, std::string_view threadName) {
    std::string name = !threadName.empty() ? std::string(threadName) : ("Thread_" + std::to_string(threadId));

    Thread thread(static_cast<int32_t>(threadId), std::move(name));
    threads.emplace(threadId, std::move(thread));
}

Thread *PerfettoApi::FindThread(uint32_t threadId) {
    auto it = threads.find(threadId);
    if (it != threads.end()) {
        return &it->second;
    }

    CreateThread(threadId, "");
    return &threads[threadId];
}

void PerfettoApi::UpdateThreadStatus(uint32_t threadId, const ThreadState &newState) {
    Thread *thread = FindThread(threadId);
    if (thread && thread->statusWrapper) {
        thread->statusWrapper->UpdateStatus(newState);
    }
}

void PerfettoApi::RegisterThread(uint32_t threadId, std::string_view threadName, int32_t groupHint) {
    DEBUG_FUNCTION(threadId, threadName, groupHint);

    auto it = threads.find(threadId);
    if (it == threads.end()) {
        CreateThread(threadId, threadName);
    }
}

void PerfettoApi::SwitchToThread(uint32_t threadId, std::string_view threadName) {
    DEBUG_FUNCTION(threadId, threadName);

    auto it = threads.find(threadId);
    if (it == threads.end()) {
        CreateThread(threadId, threadName);
    }

    Thread *prevThread = currentThreadId != 0 ? FindThread(currentThreadId) : nullptr;
    Thread *nextThread = FindThread(threadId);

    if (prevThread && prevThread->pid != static_cast<int32_t>(threadId)) {
        UpdateThreadStatus(currentThreadId, ThreadState{ThreadState::State::TASK_IDLE});
    }

    UpdateThreadStatus(threadId, ThreadState{ThreadState::State::TASK_RUNNING});
    currentThreadId = threadId;
}

void PerfettoApi::ZoneBegin(
    std::string_view name, std::string_view function, std::string_view file, uint32_t line, uint32_t color
) {
    // DEBUG_FUNCTION(name, function, file, line, color);
    //
    // Thread *thread = currentThreadId != 0 ? FindThread(currentThreadId) : nullptr;
    // if (thread) {
    //     auto callStackTrack = thread->GetOrCreateTrack(TrackType::CALL_STACK, "Call Stack");
    //     if (callStackTrack) {
    //         callStackTrack->StartSlice(std::string(name), GetTime());
    //     }
    // }
}

void PerfettoApi::ZoneEnd() {
    DEBUG_FUNCTION();

    // Thread *thread = currentThreadId != 0 ? FindThread(currentThreadId) : nullptr;
    // if (thread) {
    //     auto callStackTrack = thread->GetOrCreateTrack(TrackType::CALL_STACK, "Call Stack");
    //     if (callStackTrack && callStackTrack->GetActiveSliceCount() > 0) {
    //         callStackTrack->EndAllSlices();
    //     }
    // }
}

void PerfettoApi::Plot(std::string_view name, int64_t value) {
    DEBUG_FUNCTION(name, value);

    // static std::unordered_map<std::string, perfetto::CounterTrack> counterTracks;
    //
    // std::string nameStr(name);
    // auto it = counterTracks.find(nameStr);
    // if (it == counterTracks.end()) {
    //     perfetto::CounterTrack track(perfetto::DynamicString{nameStr});
    //     auto desc = track.Serialize();
    //     desc.set_name(nameStr);
    //     perfetto::TrackEvent::SetTrackDescriptor(track, desc);
    //     counterTracks.emplace(nameStr, track);
    //     it = counterTracks.find(nameStr);
    // }

    // TRACE_COUNTER("zones", it->second, GetTime(), value);
}

void PerfettoApi::PlotConfig(std::string_view name, int type, bool step, bool fill, uint32_t color) {
    DEBUG_FUNCTION(name, type, step, fill, color);
}

void PerfettoApi::Alloc(const void *ptr, size_t size, std::string_view name) {
    DEBUG_FUNCTION(ptr, size, name);
}

void PerfettoApi::Free(const void *ptr, std::string_view name) {
    DEBUG_FUNCTION(ptr, name);
}

void PerfettoApi::Message(std::string_view text, bool isConsole) {
    DEBUG_FUNCTION(text, color);

    Thread *thread = currentThreadId != 0 ? FindThread(currentThreadId) : nullptr;

    if (thread) {
        if (isConsole) {
            if (thread->messageTrack) {
                thread->messageTrack->Message(text);
            }
        } else {
            if (thread->extraInfoTrack) {
                thread->extraInfoTrack->Message(text);
            }
        }
    } else {
        // throw std::runtime_error("No root track found");
    }
}

void PerfettoApi::SetupLockableIfNeeded(uint32_t id) {
    DEBUG_FUNCTION(id);
    auto it = lockables.find(id);
    if (it != lockables.end()) {
        return;
    }

    std::string name = "Lock_" + std::to_string(id);
    if (symbolResolver) {
        const SymbolInfo *symbolInfo = symbolResolver->GetSymbolInfo(id);
        if (symbolInfo) {
            name = symbolInfo->name;
        }
    }

    lockables.emplace(id, Lockable{name, "Unknown", 0});
}

void PerfettoApi::LockableCreate(uint32_t id, std::string_view name, std::string_view type) {
    DEBUG_FUNCTION(id, name, type);
    SetupLockableIfNeeded(id);

    auto it = lockables.find(id);
    if (it != lockables.end()) {
        it->second.name = !name.empty() ? std::string(name) : ("Lock_" + std::to_string(id));
        it->second.type = !type.empty() ? std::string(type) : "Unknown";
    }
}

void PerfettoApi::LockableWait(uint32_t id) {
    DEBUG_FUNCTION(id);
    SetupLockableIfNeeded(id);

    auto it = lockables.find(id);
    if (it == lockables.end())
        return;

    if (currentThreadId != 0) {
        EnsureThreadRegistered(currentThreadId);
    }

    Thread *thread = currentThreadId != 0 ? FindThread(currentThreadId) : nullptr;
    if (thread) {
        auto lockTrack = thread->lockTrack;
        if (lockTrack) {
            std::string waitEvent = "Waiting for " + it->second.name;
            auto slice = lockTrack->StartSlice(waitEvent, GetTime());
            // Store the slice in the thread's lockable info for this specific lock
            thread->lockables[id].SetCurrentSlice(slice);
        }
    }
}

void PerfettoApi::LockableObtain(uint32_t id) {
    DEBUG_FUNCTION(id);
    SetupLockableIfNeeded(id);

    auto it = lockables.find(id);
    if (it == lockables.end())
        return;

    // Ensure current thread is registered
    if (currentThreadId != 0) {
        EnsureThreadRegistered(currentThreadId);
    }

    Thread *thread = currentThreadId != 0 ? FindThread(currentThreadId) : nullptr;
    if (thread) {
        auto lockTrack = thread->lockTrack;
        if (lockTrack) {
            // End the waiting slice if it exists
            auto &lockableInfo = thread->lockables[id];

            std::string heldEvent = "Holding " + it->second.name;
            auto slice = lockTrack->StartSlice(heldEvent, GetTime());
            lockableInfo.SetCurrentSlice(slice);
        }
    }

    it->second.count++;
}

void PerfettoApi::LockableRelease(uint32_t id) {
    DEBUG_FUNCTION(id);

    auto it = lockables.find(id);
    if (it == lockables.end())
        return;

    // Ensure current thread is registered
    if (currentThreadId != 0) {
        EnsureThreadRegistered(currentThreadId);
    }

    Thread *thread = currentThreadId != 0 ? FindThread(currentThreadId) : nullptr;
    if (thread) {
        auto &lockableInfo = thread->lockables[id];
        lockableInfo.ResetCurrentSlice();
    }

    if (it->second.count > 0) {
        it->second.count--;
    }
}

void PerfettoApi::ThreadWakeup(uint32_t threadId) {
    DEBUG_FUNCTION(threadId);

    UpdateThreadStatus(threadId, ThreadState{ThreadState::State::TASK_IDLE});
}

void PerfettoApi::EnterIrq(uint32_t irq) {
    DEBUG_FUNCTION(irq);

    EnsureIrqThreadRegistered(irq);
    auto tid = IrqThreadId(irq);
    UpdateThreadStatus(tid, ThreadState{ThreadState::State::TASK_RUNNING});
    currentThreadId = tid;
}

void PerfettoApi::ExitIrq(uint32_t irq) {
    DEBUG_FUNCTION(irq);

    auto tid = IrqThreadId(irq);
    // Mark the IRQ thread as stopped and clear current context if it matches
    UpdateThreadStatus(tid, ThreadState{ThreadState::State::TASK_STOPPED});
    if (currentThreadId == tid) {
        currentThreadId = 0;
    }
}

void PerfettoApi::ExitAllIrqs() {
    DEBUG_FUNCTION();
}

uint32_t PerfettoApi::IrqThreadId(uint32_t irq) {
    // Use a high-bit namespace to avoid collisions with FreeRTOS task IDs
    return 0x80000000u | (irq & 0x7FFFFFFFu);
}

void PerfettoApi::EnsureIrqThreadRegistered(uint32_t irq) {
    auto tid = IrqThreadId(irq);
    auto it = threads.find(tid);
    if (it != threads.end()) {
        return; // already registered
    }

    // Resolve IRQ name only on first registration
    std::string threadName = "IRQ_" + std::to_string(irq);
    auto deviceInfo = getDeviceInfo();
    auto itName = deviceInfo.irq_table.find(static_cast<DeviceInfo::IrqNumber>(irq));
    if (itName != deviceInfo.irq_table.end()) {
        threadName = std::string(itName->second);
    }

    CreateThread(tid, threadName);
}

void PerfettoApi::EnsureThreadRegistered(uint32_t threadId, std::string_view threadName) {
    DEBUG_FUNCTION(threadId, threadName);
    auto it = threads.find(threadId);
    if (it == threads.end()) {
        CreateThread(threadId, threadName);
    }
}

void PerfettoApi::FunctionTraceEnter(uint32_t functionAddress, std::string_view functionName) {
    DEBUG_FUNCTION(functionAddress, functionName);

    // SymbolInfo symbolInfo = GetResolvedSymbolInfo(functionAddress, functionName);
    // Thread *thread = currentThreadId != 0 ? FindThread(currentThreadId) : nullptr;
    //
    // if (thread) {
    //     auto callStackTrack = thread->rootTrack
    //     if (callStackTrack) {
    //         callStackTrack->StartSlice(symbolInfo.name, GetTime());
    //     }
    // }
}

void PerfettoApi::FunctionTraceExit(uint32_t functionAddress) {
    DEBUG_FUNCTION(functionAddress);

    // Thread *thread = currentThreadId != 0 ? FindThread(currentThreadId) : nullptr;
    // if (thread) {
    //     auto callStackTrack = thread->GetOrCreateTrack(TrackType::CALL_STACK, "Call Stack");
    //     if (callStackTrack && callStackTrack->GetActiveSliceCount() > 0) {
    //         callStackTrack->EndAllSlices();
    //     }
    // }
}

void PerfettoApi::FlowBegin(uint32_t flow_id, std::string_view name) {
    DEBUG_FUNCTION(flow_id, name);

    Thread *thread = currentThreadId != 0 ? FindThread(currentThreadId) : nullptr;
    if (thread && thread->rootTrack) {
        uint64_t timestamp = GetTime();
        auto packet = CreatePacket(timestamp);
        auto *event = packet->set_track_event();
        event->set_type(perfetto::protos::pbzero::TrackEvent::TYPE_INSTANT);
        event->set_track_uuid(thread->rootTrack->id);
        event->set_name(name.empty() ? "Flow Start" : std::string(name));
        event->add_flow_ids(flow_id);
    }
}

void PerfettoApi::FlowEnd(uint32_t flow_id, std::string_view name) {
    DEBUG_FUNCTION(flow_id, name);

    Thread *thread = currentThreadId != 0 ? FindThread(currentThreadId) : nullptr;
    if (thread && thread->rootTrack) {
        uint64_t timestamp = GetTime();
        auto packet = CreatePacket(timestamp);
        auto *event = packet->set_track_event();
        event->set_type(perfetto::protos::pbzero::TrackEvent::TYPE_INSTANT);
        event->set_track_uuid(thread->rootTrack->id);
        event->set_name(name.empty() ? "Flow End" : std::string(name));
        event->add_terminating_flow_ids(flow_id);
    }
}

}
