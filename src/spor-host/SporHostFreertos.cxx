#include <cstring>
#include <iostream>
#include <memory>

#include "PerfettoApi.hpp"
#include "SporHost.hpp"
#include "symbol-resolver/SymbolResolver.hpp"

void SporHost::HandleMessage(const FreertosTaskSwitchedInMessage &msg) {
    std::string name = "";
    auto it = tasks.find(msg.handle);
    if (it != tasks.end()) {
        name = it->second.name;
    }
    profiler::PerfettoApi::SwitchToThread(msg.handle, name);
}

void SporHost::HandleMessage(const FreertosTaskSwitchedOutMessage &msg) {
    auto threadState = CreateThreadStateFromSwitchReason(msg.switchReason);
    // std::cout << threadState.description << std::endl;
    profiler::PerfettoApi::UpdateThreadStatus(msg.handle, threadState);

    // Add a Track::Message for blocked thread reasons if there's additional info
    if (msg.blockedOnObject != 0) {
        auto it = tasks.find(msg.handle);
        std::string message = "Task ";
        if (it != tasks.end()) {
            message += it->second.name;
        } else {
            message += std::to_string(msg.handle);
        }
        message += " blocked on object: " + std::to_string(msg.blockedOnObject);
        profiler::PerfettoApi::Message(message);
    }
}

void SporHost::HandleMessage(const FreertosTaskReadiedMessage &msg) {
    // auto it = tasks.find(msg.handle);
    // if (it != tasks.end()) {
    profiler::PerfettoApi::ThreadWakeup(msg.handle);
    // }
}

void SporHost::HandleMessage(const FreertosQueueCreatedMessage &msg) {
    uint32_t handleId = msg.handle;
    const char *typeName = "Queue";
    ObjectInfo::Type objType = ObjectInfo::Type::Queue;

    switch (msg.queueType) {
    case QueueType::MUTEX:
        typeName = "Mutex";
        objType = ObjectInfo::Type::Mutex;
        break;
    case QueueType::RECURSIVE_MUTEX:
        typeName = "RecursiveMutex";
        objType = ObjectInfo::Type::Mutex;
        break;
    case QueueType::COUNTING_SEMAPHORE:
        typeName = "CountingSem";
        objType = ObjectInfo::Type::CountingSem;
        break;
    case QueueType::BINARY_SEMAPHORE:
        typeName = "BinarySem";
        objType = ObjectInfo::Type::BinarySem;
        break;
    case QueueType::BASE:
    default:
        break;
    }

    auto it = pointerNames.find(handleId);
    const std::string &name = (it != pointerNames.end()) ? it->second : "Unknown";

    profiler::PerfettoApi::LockableCreate(handleId, name, typeName);
    objects[handleId] = {handleId, objType};
}

void SporHost::HandleMessage(const FreertosQueuePeekMessage &msg) {
    auto it = objects.find(msg.handle);
    if (it != objects.end()) {
        profiler::PerfettoApi::Plot("Queue Depth", static_cast<int64_t>(msg.updatedCount));
    }
}

void SporHost::HandleMessage(const FreertosQueueSendMessage &msg) {
    // /** TODO EGILL –Handle ISR */
    // if (msg.isFromISR)
    //     return;

    switch (msg.queueType) {
    case QueueType::BINARY_SEMAPHORE:
        profiler::PerfettoApi::LockableRelease(msg.handle);
        break;
    case QueueType::COUNTING_SEMAPHORE:
        profiler::PerfettoApi::LockableRelease(msg.handle);
        profiler::PerfettoApi::Plot("CountingSem Count", static_cast<int64_t>(msg.updatedCount));
        break;
    case QueueType::MUTEX:
    case QueueType::RECURSIVE_MUTEX:
        profiler::PerfettoApi::LockableRelease(msg.handle);
        break;
    case QueueType::BASE:
    default:
        profiler::PerfettoApi::LockableRelease(msg.handle);
        // profiler::PerfettoApi::Plot("Queue Depth", static_cast<int64_t>(msg.updatedCount));
        break;
    }
}

void SporHost::HandleMessage(const FreertosQueueReceiveMessage &msg) {
    // /** TODO EGILL –Handle ISR */
    // if (msg.isFromISR)
    //     return;

    switch (msg.queueType) {
    case QueueType::BINARY_SEMAPHORE:
        profiler::PerfettoApi::LockableObtain(msg.handle);
        break;
    case QueueType::COUNTING_SEMAPHORE:
        profiler::PerfettoApi::LockableObtain(msg.handle);
        profiler::PerfettoApi::Plot("CountingSem Count", static_cast<int64_t>(msg.updatedCount));
        break;
    case QueueType::MUTEX:
    case QueueType::RECURSIVE_MUTEX:
        profiler::PerfettoApi::LockableObtain(msg.handle);
        break;
    case QueueType::BASE:
    default:
        profiler::PerfettoApi::Plot("Queue Depth", static_cast<int64_t>(msg.updatedCount));
        break;
    }
}

void SporHost::HandleMessage(const FreertosTaskNotifyMessage &msg) {
    auto it = tasks.find(msg.handle);
    if (it != tasks.end()) {
        std::string message = "Task " + it->second.name + " notified (index: " + std::to_string(msg.index) +
                              ", action: " + std::to_string(msg.action) + ")";
        profiler::PerfettoApi::Message(message);
        profiler::PerfettoApi::Plot("Task Notify Value", static_cast<int64_t>(msg.updatedValue));
    }
}

void SporHost::HandleMessage(const FreertosTaskNotifyReceivedMessage &msg) {
    auto it = tasks.find(msg.handle);
    if (it != tasks.end()) {
        std::string message =
            "Task " + it->second.name + " notification received (index: " + std::to_string(msg.index) + ")";
        profiler::PerfettoApi::Message(message);
        profiler::PerfettoApi::Plot("Task Notify Value", static_cast<int64_t>(msg.updatedValue));
    }
}

void SporHost::HandleMessage(const FreertosTaskDeletedMessage &msg) {
    auto it = tasks.find(msg.handle);
    if (it != tasks.end()) {
        std::string message = "Task " + it->second.name + " deleted";
        profiler::PerfettoApi::Message(message);
        tasks.erase(it);
    }
}

void SporHost::HandleMessage(const FreertosTaskPrioritySetMessage &msg) {
    auto it = tasks.find(msg.handle);
    if (it != tasks.end()) {
        std::string message = "Task " + it->second.name + " priority changed from " + std::to_string(msg.oldPriority) +
                              " to " + std::to_string(msg.newPriority);
        profiler::PerfettoApi::Message(message);
        profiler::PerfettoApi::Plot("Task Priority", static_cast<int64_t>(msg.newPriority));
    }
}

void SporHost::HandleMessage(const FreertosTaskSuspendMessage &msg) {
    auto it = tasks.find(msg.handle);
    if (it != tasks.end()) {
        std::string message = "Task " + it->second.name + " suspended";
        profiler::PerfettoApi::Message(message);
    }
}

void SporHost::HandleMessage(const FreertosTaskResumeMessage &msg) {
    auto it = tasks.find(msg.handle);
    if (it != tasks.end()) {
        std::string message = "Task " + it->second.name + " resumed";
        if (msg.isFromISR) {
            message += " (from ISR)";
        }
        profiler::PerfettoApi::Message(message);
        profiler::PerfettoApi::ThreadWakeup(msg.handle);
    }
}

void SporHost::HandleMessage(const FreertosTaskDelayMessage &msg) {
    profiler::ThreadState delayedState{
        profiler::ThreadState::State::TASK_DELAYED, "Delayed for " + std::to_string(msg.ticksToDelay) + " ticks"
    };
    profiler::PerfettoApi::UpdateThreadStatus(msg.handle, delayedState);
    profiler::PerfettoApi::Message("Task delayed for " + std::to_string(msg.ticksToDelay) + " ticks");
}

void SporHost::HandleMessage(const FreertosTaskDelayUntilMessage &msg) {
    profiler::ThreadState delayedState{
        profiler::ThreadState::State::TASK_DELAYED, "Delayed until tick " + std::to_string(msg.timeToWake)
    };
    profiler::PerfettoApi::UpdateThreadStatus(msg.handle, delayedState);
    profiler::PerfettoApi::Message("Task delayed until tick " + std::to_string(msg.timeToWake));
}

void SporHost::HandleMessage(const FreertosTimerCreatedMessage &msg) {
    std::string message = "Timer created (handle: " + std::to_string(msg.handle) +
                          ", period: " + std::to_string(msg.period) +
                          ", auto-reload: " + (msg.autoReload ? "yes" : "no") + ")";
    profiler::PerfettoApi::Message(message);
}

void SporHost::HandleMessage(const FreertosTimerCommandMessage &msg) {
    std::string message =
        "Timer command (handle: " + std::to_string(msg.handle) + ", cmd: " + std::to_string(msg.commandId) + ")";
    if (msg.optionalValue != 0) {
        message += " value: " + std::to_string(msg.optionalValue);
    }
    if (msg.isFromISR) {
        message += " (from ISR)";
    }
    profiler::PerfettoApi::Message(message);
}

void SporHost::HandleMessage(const FreertosTimerExpiredMessage &msg) {
    std::string message = "Timer expired (handle: " + std::to_string(msg.handle) + ")";
    profiler::PerfettoApi::Message(message);
}

void SporHost::HandleMessage(const FreertosEventGroupCreatedMessage &msg) {
    std::string message = "Event group created (handle: " + std::to_string(msg.handle) + ")";
    profiler::PerfettoApi::Message(message);
    profiler::PerfettoApi::LockableCreate(msg.handle, "EventGroup", "EventGroup");
}

void SporHost::HandleMessage(const FreertosEventGroupDeletedMessage &msg) {
    std::string message = "Event group deleted (handle: " + std::to_string(msg.handle) + ")";
    profiler::PerfettoApi::Message(message);
}

void SporHost::HandleMessage(const FreertosEventGroupSyncMessage &msg) {
    std::string message = "Event group sync (handle: " + std::to_string(msg.handle) + ", set: 0x" +
                          std::to_string(msg.setBits) + ", wait: 0x" + std::to_string(msg.waitBits) + ")";
    profiler::PerfettoApi::Message(message);
    profiler::PerfettoApi::Plot("EventGroup Bits", static_cast<int64_t>(msg.resultBits));
}

void SporHost::HandleMessage(const FreertosEventGroupWaitBitsMessage &msg) {
    std::string message = "Event group wait bits (handle: " + std::to_string(msg.handle) + ", wait: 0x" +
                          std::to_string(msg.waitBits) + ")";
    profiler::PerfettoApi::Message(message);
    profiler::PerfettoApi::LockableWait(msg.handle);
    profiler::PerfettoApi::LockableObtain(msg.handle);
    profiler::PerfettoApi::Plot("EventGroup Bits", static_cast<int64_t>(msg.resultBits));
}

void SporHost::HandleMessage(const FreertosEventGroupClearBitsMessage &msg) {
    std::string message = "Event group clear bits (handle: " + std::to_string(msg.handle) + ", clear: 0x" +
                          std::to_string(msg.clearBits) + ")";
    if (msg.isFromISR) {
        message += " (from ISR)";
    }
    profiler::PerfettoApi::Message(message);
}

void SporHost::HandleMessage(const FreertosEventGroupSetBitsMessage &msg) {
    std::string message =
        "Event group set bits (handle: " + std::to_string(msg.handle) + ", set: 0x" + std::to_string(msg.setBits) + ")";
    // TODO
    // if (msg.isFromISR) {
    //     message += " (from ISR)";
    // }
    profiler::PerfettoApi::Message(message);
    profiler::PerfettoApi::LockableRelease(msg.handle);
}

void SporHost::HandleMessage(const FreertosStreamBufferCreatedMessage &msg) {
    // std::string message = msg.isMessageBuffer ? "Message buffer created" : "Stream buffer created";
    // message += " (handle: " + std::to_string(msg.handle) + ", size: " + std::to_string(msg.size) +
    //            ", trigger: " + std::to_string(msg.triggerLevel) + ")";
    // profiler::PerfettoApi::Message(message);
    // profiler::PerfettoApi::LockableCreate(
    //     msg.handle, msg.isMessageBuffer ? "MessageBuffer" : "StreamBuffer", "StreamBuffer"
    // );
}

void SporHost::HandleMessage(const FreertosStreamBufferDeletedMessage &msg) {
    // std::string message = "Stream buffer deleted (handle: " + std::to_string(msg.handle) + ")";
    // profiler::PerfettoApi::Message(message);
}

void SporHost::HandleMessage(const FreertosStreamBufferSendMessage &msg) {
    // std::string message = "Stream buffer send (handle: " + std::to_string(msg.handle) +
    //                       ", bytes: " + std::to_string(msg.bytesSent) + ")";
    // if (msg.isFromISR) {
    //     message += " (from ISR)";
    // }
    // profiler::PerfettoApi::Message(message);
    // profiler::PerfettoApi::Plot("StreamBuffer Send", static_cast<int64_t>(msg.bytesSent));
}

void SporHost::HandleMessage(const FreertosStreamBufferReceiveMessage &msg) {
    // std::string message = "Stream buffer receive (handle: " + std::to_string(msg.handle) +
    //                       ", bytes: " + std::to_string(msg.bytesReceived) + ")";
    // if (msg.isFromISR) {
    //     message += " (from ISR)";
    // }
    // profiler::PerfettoApi::Message(message);
    // profiler::PerfettoApi::LockableWait(msg.handle);
    // profiler::PerfettoApi::LockableObtain(msg.handle);
    // profiler::PerfettoApi::Plot("StreamBuffer Receive", static_cast<int64_t>(msg.bytesReceived));
}

void SporHost::HandleMessage(const FreertosStreamBufferResetMessage &msg) {
    // std::string message = "Stream buffer reset (handle: " + std::to_string(msg.handle) + ")";
    // if (msg.isFromISR) {
    //     message += " (from ISR)";
    // }
    // profiler::PerfettoApi::Message(message);
}

void SporHost::HandleMessage(const FreertosIsrEnterMessage &msg) {
    // profiler::PerfettoApi::EnterIrq(msg.isrNumber);
}

void SporHost::HandleMessage(const FreertosIsrExitMessage &msg) {
    // profiler::PerfettoApi::ExitIrq(msg.isrNumber);
}

void SporHost::HandleMessage(const FreertosIsrExitToSchedulerMessage &msg) {
    // profiler::PerfettoApi::ExitIrq(msg.isrNumber);
}

void SporHost::HandleMessage(const FreertosQueueDeletedMessage &msg) {
    auto it = objects.find(msg.handle);
    if (it != objects.end()) {
        objects.erase(it);
    }
}

void SporHost::HandleMessage(const FreertosQueueRegistryMessage &msg) {}

void SporHost::HandleMessage(const FreertosQueueCreateFailedMessage &msg) {
    std::string message;
    switch (msg.queueType) {
    case QueueType::MUTEX:
        message = "Mutex create failed";
        break;
    case QueueType::RECURSIVE_MUTEX:
        message = "Recursive mutex create failed";
        break;
    case QueueType::COUNTING_SEMAPHORE:
        message = "Counting semaphore create failed";
        break;
    case QueueType::BINARY_SEMAPHORE:
        message = "Binary semaphore create failed";
        break;
    case QueueType::BASE:
    default:
        message = "Queue create failed";
        break;
    }
    profiler::PerfettoApi::Message(message);
}

void SporHost::HandleMessage(const FreertosQueueSendFailedMessage &msg) {
    std::string message = "Queue send failed (handle: " + std::to_string(msg.handle) +
                          ")"; //", type: " + std::to_string(msg.queueType) + ")";
    if (msg.isFromISR) {
        message += " (from ISR)";
    }
    profiler::PerfettoApi::Message(message);
}

void SporHost::HandleMessage(const FreertosQueueReceiveFailedMessage &msg) {
    std::string message = "Queue receive failed (handle: " + std::to_string(msg.handle) + ")";
    if (msg.isFromISR) {
        message += " (from ISR)";
    }
    profiler::PerfettoApi::Message(message);
}

void SporHost::HandleMessage(const FreertosQueuePeekFailedMessage &msg) {
    std::string message = "Queue peek failed (handle: " + std::to_string(msg.handle) + ")";
    if (msg.isFromISR) {
        message += " (from ISR)";
    }
    profiler::PerfettoApi::Message(message);
}

void SporHost::HandleMessage(const FreertosTimerCreateFailedMessage &msg) {
    std::string message = "Timer create failed";
    profiler::PerfettoApi::Message(message);
}

void SporHost::HandleMessage(const FreertosTimerCommandReceivedMessage &msg) {
    std::string message = "Timer command received (handle: " + std::to_string(msg.handle) +
                          ", cmd: " + std::to_string(msg.commandId) + ")";
    if (msg.optionalValue != 0) {
        message += " value: " + std::to_string(msg.optionalValue);
    }
    profiler::PerfettoApi::Message(message);
}

void SporHost::HandleMessage(const FreertosStreamBufferCreateFailedMessage &msg) {
    std::string message = "Stream buffer create failed)";
    profiler::PerfettoApi::Message(message);
}

void SporHost::HandleMessage(const FreertosStreamBufferSendFailedMessage &msg) {
    std::string message = "Stream buffer send failed (handle: " + std::to_string(msg.handle) + ")";
    if (msg.isFromISR) {
        message += " (from ISR)";
    }
    profiler::PerfettoApi::Message(message);
}

void SporHost::HandleMessage(const FreertosStreamBufferReceiveFailedMessage &msg) {
    std::string message = "Stream buffer receive failed (handle: " + std::to_string(msg.handle) + ")";
    if (msg.isFromISR) {
        message += " (from ISR)";
    }
    profiler::PerfettoApi::Message(message);
}

void SporHost::HandleMessage(const FreertosEventGroupCreateFailedMessage &msg) {
    std::string message = "Event group create failed)";
    profiler::PerfettoApi::Message(message);
}
