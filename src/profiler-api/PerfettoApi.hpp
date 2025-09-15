#pragma once

#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <optional>
#include <perfetto.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "symbol-resolver/SymbolResolver.hpp"
#include "TimeUtils.hpp"
#include "Track.hpp"

namespace profiler {

struct Lockable {
    std::string name;
    std::string type;
    uint32_t count = 0;

    Lockable() = default;
    Lockable(std::string name, std::string type, uint32_t count = 0)
        : name(std::move(name)), type(std::move(type)), count(count) {}

private:
    std::optional<std::shared_ptr<Slice>> currentSlice;

public:
    std::optional<std::shared_ptr<Slice>> GetCurrentSlice() const {
        return currentSlice;
    }

    void SetCurrentSlice(std::shared_ptr<Slice> slice) {
        if (currentSlice.has_value() && currentSlice.value()) {
            currentSlice.value()->End();
        }
        currentSlice = slice;
    }

    void ResetCurrentSlice() {
        if (currentSlice.has_value() && currentSlice.value()) {
            currentSlice.value()->End();
        }
        currentSlice.reset();
    }
};

struct ThreadState {
    enum class State : int64_t {
        TASK_RUNNING,
        TASK_IDLE,
        TASK_WAITING,
        TASK_STOPPED,
        TASK_DELAYED,
    };

    State state;
    std::string description;

    ThreadState(State s = State::TASK_STOPPED, std::string desc = {}) : state(s), description(std::move(desc)) {}

    bool operator==(const ThreadState &other) const {
        return state == other.state && description == other.description;
    }

    bool operator!=(const ThreadState &other) const {
        return !(*this == other);
    }
};

class ThreadStatusWrapper {
public:
    ThreadStatusWrapper(std::shared_ptr<TrackNode> track) : statusTrack(track) {}

    void UpdateStatus(const ThreadState &newState, uint64_t timestamp = GetTime()) {
        if (currentState != newState) {
            EndCurrentSlice();

            if (newState.state != ThreadState::State::TASK_STOPPED) {
                std::string sliceName = GetStateDisplayName(newState);
                if (!newState.description.empty()) {
                    sliceName = newState.description;
                }
                currentSlice = statusTrack->StartSlice(sliceName, timestamp);
            }

            currentState = newState;
        }
    }

    ThreadState GetCurrentState() const {
        return currentState;
    }

private:
    std::shared_ptr<TrackNode> statusTrack;
    ThreadState currentState{ThreadState::State::TASK_STOPPED};
    std::shared_ptr<Slice> currentSlice;

    void EndCurrentSlice() {
        if (currentSlice) {
            currentSlice->End();
            currentSlice.reset();
        }
    }

    std::string GetStateDisplayName(const ThreadState &state) const {
        switch (state.state) {
        case ThreadState::State::TASK_RUNNING:
            return "Running";
        case ThreadState::State::TASK_IDLE:
            return "Sleeping";
        case ThreadState::State::TASK_WAITING:
            return "Waiting";
        case ThreadState::State::TASK_STOPPED:
            return "Stopped";
        case ThreadState::State::TASK_DELAYED:
            return "Delayed";
        default:
            return "Unknown";
        }
    }
};

struct Thread {
    int32_t pid = 0;
    std::string name;
    int32_t prio = 120;
    std::unique_ptr<ThreadStatusWrapper> statusWrapper;
    std::unordered_map<uint32_t, Lockable> lockables; // Track locks per thread

    std::shared_ptr<TrackNode> rootTrack;
    std::shared_ptr<TrackNode> lockTrack; // Single lock track for all locks
    std::shared_ptr<TrackNode> messageTrack;
    std::shared_ptr<TrackNode> extraInfoTrack;

    Thread() = default;
    Thread(int32_t pid, std::string name) : pid(pid), name(name) {
        rootTrack = TrackManager::Instance().CreateTrack(TrackType::THREAD_STATUS, name, nullptr);
        messageTrack = TrackManager::Instance().CreateTrack(TrackType::THREAD_STATUS, "Log", rootTrack);
        extraInfoTrack = TrackManager::Instance().CreateTrack(TrackType::THREAD_STATUS, "Debug events", rootTrack);
        statusWrapper = std::make_unique<ThreadStatusWrapper>(rootTrack);
        lockTrack = TrackManager::Instance().CreateTrack(TrackType::THREAD_STATUS, "Mutexes", rootTrack);
    }
};

struct PerfettoApi {
    static std::unordered_map<uint32_t, Lockable> lockables;
    static std::unordered_map<uint32_t, Thread> threads;
    static uint32_t currentThreadId;

public:
    static std::unique_ptr<perfetto::TracingSession> StartTracing();
    static void StopTracing(std::unique_ptr<perfetto::TracingSession> tracing_session);

    static void RegisterThread(uint32_t threadId, std::string_view threadName, int32_t groupHint = 0);
    static void SwitchToThread(uint32_t threadId, std::string_view threadName = {});

    static void ZoneBegin(
        std::string_view name,
        std::string_view function = {},
        std::string_view file = {},
        uint32_t line = 0,
        uint32_t color = 0
    );
    static void ZoneEnd();
    static void Plot(std::string_view name, int64_t value);
    static void PlotConfig(std::string_view name, int type, bool step, bool fill, uint32_t color);
    static void Message(std::string_view text, bool isConsole = 0);
    static void Alloc(const void *ptr, size_t size, std::string_view name = {});
    static void Free(const void *ptr, std::string_view name = {});

    static void SetupLockableIfNeeded(uint32_t id);
    static void LockableCreate(uint32_t id, std::string_view name, std::string_view type);
    static void LockableWait(uint32_t id);
    static void LockableObtain(uint32_t id);
    static void LockableRelease(uint32_t id);

    static void ThreadWakeup(uint32_t threadId);
    static void UpdateThreadStatus(uint32_t threadId, const ThreadState &newState);

    static void EnterIrq(uint32_t ptr);
    static void ExitIrq(uint32_t ptr);
    static void ExitAllIrqs();
    static void EnsureThreadRegistered(uint32_t threadId, std::string_view threadName = {});

    static void FunctionTraceEnter(uint32_t functionAddress, std::string_view functionName = {});
    static void FunctionTraceExit(uint32_t functionAddress);

    static void FlowBegin(uint32_t flow_id, std::string_view name = {});
    static void FlowEnd(uint32_t flow_id, std::string_view name = {});

    static TrackManager &GetTrackManager() {
        return TrackManager::Instance();
    }

private:
    static std::unique_ptr<SymbolResolver> symbolResolver;

    static void CreateThread(uint32_t threadId, std::string_view threadName = {});
    static Thread *FindThread(uint32_t threadId);

    // IRQ-specific helpers to avoid ID collisions with FreeRTOS task IDs
    static uint32_t IrqThreadId(uint32_t irq);
    static void EnsureIrqThreadRegistered(uint32_t irq);
};

}
