#pragma once

#include <cstdint>
#include <string_view>

#include "utils/String.hpp"
#include "utils/TypeList.hpp"

enum class FreeRtosSwitchReason : uint8_t {
    TICK,
    DELAYED,
    TASK_NOTIFY_WAIT,
    BLOCKED_QUEUE_PUSH,
    BLOCKED_QUEUE_POP,
    BLOCKED_BINARY_SEMAPHORE_RECEIVE,
    BLOCKED_BINARY_SEMAPHORE_GIVE,
    BLOCKED_EVENT_GROUP,
    COUNTING_SEMAPHORE_GIVE,
    COUNTING_SEMAPHORE_TAKE,
    BLOCKED_MUTEX_LOCK,
    BLOCKED_MUTEX_UNLOCK,
    BLOCKED_OTHER
};

/** From FreeRTOS's queueQUEUE_TYPE_ defines */
enum class QueueType : uint8_t {
    BASE = 0,
    MUTEX = 1,
    COUNTING_SEMAPHORE = 2,
    BINARY_SEMAPHORE = 3,
    RECURSIVE_MUTEX = 4,
    SET = 5,
};

struct FreertosTaskCreatedMessage {
    TargetPointer handle;
    StringOrSymbol name;
    uint32_t priority;

    constexpr static auto serialize(auto &archive, auto &self) {
        return archive(self.handle, self.name, self.priority);
    }
};

struct FreertosTaskSwitchedInMessage {
    TargetPointer handle;
};

struct FreertosTaskSwitchedOutMessage {
    TargetPointer handle;
    FreeRtosSwitchReason switchReason; // enum FreeRtosSwitchReason
    uint32_t blockedOnObject;
    bool stillReady;
};

struct FreertosTaskReadiedMessage {
    TargetPointer handle;
};

struct FreertosQueueCreatedMessage {
    TargetPointer handle;
    uint32_t capacity;
    QueueType queueType;
};

struct FreertosQueuePeekMessage {
    TargetPointer handle;
    uint32_t updatedCount;
    bool isFromISR;
};

struct FreertosTaskNotifyMessage {
    TargetPointer handle;
    uint32_t index;
    uint32_t action;
    uint32_t updatedValue;
    bool isFromISR;
};

struct FreertosTaskNotifyReceivedMessage {
    TargetPointer handle;
    uint32_t index;
    uint32_t updatedValue;
};

struct FreertosTaskDeletedMessage {
    TargetPointer handle;
};

struct FreertosTaskPrioritySetMessage {
    TargetPointer handle;
    uint32_t oldPriority;
    uint32_t newPriority;
};

struct FreertosTaskSuspendMessage {
    TargetPointer handle;
};

struct FreertosTaskResumeMessage {
    TargetPointer handle;
    bool isFromISR;
};

struct FreertosTimerCreatedMessage {
    uint32_t handle;
    uint32_t period;
    bool autoReload;
};

struct FreertosTimerCommandMessage {
    uint32_t handle;
    uint32_t commandId;
    uint32_t optionalValue;
    bool isFromISR;
};

struct FreertosTimerExpiredMessage {
    uint32_t handle;
};

struct FreertosEventGroupCreatedMessage {
    TargetPointer handle;
};

struct FreertosEventGroupDeletedMessage {
    TargetPointer handle;
};

struct FreertosEventGroupSyncMessage {
    TargetPointer handle;
    uint32_t setBits;
    uint32_t waitBits;
    uint32_t resultBits;
};

struct FreertosEventGroupWaitBitsMessage {
    TargetPointer handle;
    uint32_t waitBits;
    uint32_t resultBits;
};

struct FreertosEventGroupClearBitsMessage {
    TargetPointer handle;
    uint32_t clearBits;
    bool isFromISR;
};

struct FreertosEventGroupSetBitsMessage {
    TargetPointer handle;
    uint32_t setBits;
    bool isFromISR;
};

struct FreertosStreamBufferCreatedMessage {
    TargetPointer handle;
    uint32_t size;
    uint32_t triggerLevel;
    bool isMessageBuffer;
};

struct FreertosStreamBufferDeletedMessage {
    TargetPointer handle;
};

struct FreertosStreamBufferSendMessage {
    TargetPointer handle;
    uint32_t bytesSent;
    bool isFromISR;
};

struct FreertosStreamBufferReceiveMessage {
    TargetPointer handle;
    uint32_t bytesReceived;
    bool isFromISR;
};

struct FreertosStreamBufferResetMessage {
    TargetPointer handle;
    bool isFromISR;
};

struct FreertosIsrEnterMessage {
    uint32_t isrNumber;
};

struct FreertosIsrExitMessage {
    uint32_t isrNumber;
};

struct FreertosIsrExitToSchedulerMessage {
    uint32_t isrNumber;
};

struct FreertosQueueDeletedMessage {
    TargetPointer handle;
};

struct FreertosQueueRegistryMessage {
    TargetPointer handle;
    StringOrSymbol name;

    constexpr static auto serialize(auto &archive, auto &self) {
        return archive(self.handle, self.name);
    }
};

struct FreertosQueueCreateFailedMessage {
    QueueType queueType;
};

struct FreertosQueueSendMessage {
    TargetPointer handle;
    uint32_t updatedCount;
    QueueType queueType;
    bool isFromISR;
};

struct FreertosQueueReceiveMessage {
    TargetPointer handle;
    uint32_t updatedCount;
    QueueType queueType;
    bool isFromISR;
};

struct FreertosQueueSendFailedMessage {
    TargetPointer handle;
    QueueType queueType;
    bool isFromISR;
};

struct FreertosQueueReceiveFailedMessage {
    TargetPointer handle;
    QueueType queueType;
    bool isFromISR;
};

struct FreertosQueuePeekFailedMessage {
    TargetPointer handle;
    QueueType queueType;
    bool isFromISR;
};

struct FreertosTimerCreateFailedMessage {};

struct FreertosTimerCommandReceivedMessage {
    TargetPointer handle;
    uint32_t commandId;
    uint32_t optionalValue;
};

struct FreertosStreamBufferCreateFailedMessage {};

struct FreertosStreamBufferSendFailedMessage {
    TargetPointer handle;
    bool isFromISR;
};

struct FreertosStreamBufferReceiveFailedMessage {
    TargetPointer handle;
    bool isFromISR;
};

struct FreertosEventGroupCreateFailedMessage {};

struct FreertosTaskDelayMessage {
    TargetPointer handle;
    uint32_t ticksToDelay;
};

struct FreertosTaskDelayUntilMessage {
    TargetPointer handle;
    uint32_t timeToWake;
};

using FreertosMessages = TypeList<
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
    FreertosTaskDelayUntilMessage>;
