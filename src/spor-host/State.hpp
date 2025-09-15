#pragma once

#include <atomic>
#include <string>
#include <unordered_map>

#include "orbcat/Orbcat.hpp"
#include "PerfettoApi.hpp"
#include "spor-devices/DeviceInfo.hpp"
#include "spor-host/Decoder.hpp"
#include "symbol-resolver/ElfSymbolResolver.hpp"

struct TaskInfo {
    std::string name;
    TargetPointer handle;
};

struct ObjectInfo {
    enum class Type { Queue, Mutex, CountingSem, BinarySem, RecursiveMutex, EventGroup, StreamBuffer, Timer, Unknown };

    TargetPointer handle;
    Type type;
};

struct IrqInfo {
    DeviceInfo::IrqNumber irqNumber;
    std::string_view name;
};

inline profiler::ThreadState CreateThreadStateFromSwitchReason(FreeRtosSwitchReason switchReason) {
    switch (switchReason) {
    case FreeRtosSwitchReason::TICK:
        return profiler::ThreadState{profiler::ThreadState::State::TASK_WAITING, "Preempted (tick)"};
    case FreeRtosSwitchReason::DELAYED:
        return profiler::ThreadState{profiler::ThreadState::State::TASK_WAITING, "Delayed"};
    case FreeRtosSwitchReason::TASK_NOTIFY_WAIT:
        return profiler::ThreadState{profiler::ThreadState::State::TASK_WAITING, "Waiting for notification"};
    case FreeRtosSwitchReason::BLOCKED_QUEUE_PUSH:
        return profiler::ThreadState{profiler::ThreadState::State::TASK_WAITING, "Blocked on queue send"};
    case FreeRtosSwitchReason::BLOCKED_QUEUE_POP:
        return profiler::ThreadState{profiler::ThreadState::State::TASK_WAITING, "Blocked on queue receive"};
    case FreeRtosSwitchReason::BLOCKED_BINARY_SEMAPHORE_RECEIVE:
        return profiler::ThreadState{profiler::ThreadState::State::TASK_WAITING, "Blocked on binary semaphore"};
    case FreeRtosSwitchReason::BLOCKED_BINARY_SEMAPHORE_GIVE:
        return profiler::ThreadState{profiler::ThreadState::State::TASK_WAITING, "Blocked giving binary semaphore"};
    case FreeRtosSwitchReason::BLOCKED_EVENT_GROUP:
        return profiler::ThreadState{profiler::ThreadState::State::TASK_WAITING, "Blocked on event group"};
    case FreeRtosSwitchReason::COUNTING_SEMAPHORE_GIVE:
        return profiler::ThreadState{profiler::ThreadState::State::TASK_WAITING, "Blocked giving counting semaphore"};
    case FreeRtosSwitchReason::COUNTING_SEMAPHORE_TAKE:
        return profiler::ThreadState{profiler::ThreadState::State::TASK_WAITING, "Blocked on counting semaphore"};
    case FreeRtosSwitchReason::BLOCKED_MUTEX_LOCK:
        return profiler::ThreadState{profiler::ThreadState::State::TASK_WAITING, "Blocked on mutex"};
    case FreeRtosSwitchReason::BLOCKED_MUTEX_UNLOCK:
        return profiler::ThreadState{profiler::ThreadState::State::TASK_WAITING, "Blocked unlocking mutex"};
    case FreeRtosSwitchReason::BLOCKED_OTHER:
        return profiler::ThreadState{profiler::ThreadState::State::TASK_WAITING, "Blocked (other)"};
    default:
        return profiler::ThreadState{profiler::ThreadState::State::TASK_WAITING, "Blocked (unknown reason)"};
    }
}

struct State {
    DeviceInfo deviceInfo = getDeviceInfo();

    std::atomic<uint64_t> cpuFrequencyHz{200000000};

    std::unordered_map<TargetPointer, TaskInfo> tasks;
    std::unordered_map<TargetPointer, ObjectInfo> objects;

    std::unordered_map<TargetPointer, IrqInfo> irqFunctions;

    void SymbolsLoaded();

    void FunctionEnter(TargetPointer ptr);
    void FunctionExit(TargetPointer ptr);

    void HandleException(const orbcat::ExceptionMessage &exception, uint64_t timestamp);

    std::unordered_map<uint32_t, std::string> pointerNames;
    std::unordered_map<uint32_t, std::string> pointerTypes;
};
