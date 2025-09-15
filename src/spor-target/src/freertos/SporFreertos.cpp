#include "FreeRTOS.h"
#include SPOR_SYSTEM_HEADER
#include "queue.h"
#include "spor-common/Messages.hpp"
#include "task.h"
#include "transport/Transport.hpp"
#include "Utils.hpp"

using namespace spor;

static bool InInterruptContext() {
    return __get_IPSR() != 0;
}

static enum FreeRtosSwitchReason currentSwitchReason = FreeRtosSwitchReason::TICK;
static void *currentBlockedOnObject = nullptr;

void SporFreeRtosSetSwitchReason(enum FreeRtosSwitchReason reason, void *blockedOnObject) {
    currentSwitchReason = reason;
    currentBlockedOnObject = blockedOnObject;
}

void SporFreeRtosGetSwitchContext(enum FreeRtosSwitchReason *reason, void **blockedOnObject) {
    if (reason)
        *reason = currentSwitchReason;
    if (blockedOnObject)
        *blockedOnObject = currentBlockedOnObject;
}

void SporFreeRtosStart() {
    Send(SystemInfoData{.clock_frequency_mhz = SystemCoreClock / 1000000});
}

void SporFreeRtosEnd() {}

void SporFreeRtosTaskSwitchedIn(void *tcb) {
    Send(FreertosTaskSwitchedInMessage{.handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(tcb))});
}

void SporFreeRtosTaskSwitchedOut(void *tcb, bool stillReady) {
    enum FreeRtosSwitchReason reason;
    void *blockedOnObject;
    SporFreeRtosGetSwitchContext(&reason, &blockedOnObject);

    Send(
        FreertosTaskSwitchedOutMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(tcb)),
            .switchReason = reason,
            .blockedOnObject = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(blockedOnObject)),
            .stillReady = stillReady
        }
    );

    currentSwitchReason = FreeRtosSwitchReason::TICK;
    currentBlockedOnObject = nullptr;
}

void SporFreeRtosTaskCreated(void *tcb, const char *name, uint32_t priority) {
    Send(
        FreertosTaskCreatedMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(tcb)),
            .name = name ? name : "",
            .priority = priority
        }
    );
}

void SporFreeRtosTaskDeleted(void *tcb) {
    Send(FreertosTaskDeletedMessage{.handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(tcb))});
}

void SporFreeRtosTaskPrioritySet(void *tcb, uint32_t oldPriority, uint32_t newPriority) {
    Send(
        FreertosTaskPrioritySetMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(tcb)),
            .oldPriority = oldPriority,
            .newPriority = newPriority
        }
    );
}

void SporFreeRtosTaskSuspend(void *tcb) {
    Send(FreertosTaskSuspendMessage{.handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(tcb))});
}

void SporFreeRtosTaskResume(void *tcb) {
    Send(
        FreertosTaskResumeMessage{.handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(tcb)), .isFromISR = false}
    );
}

void SporFreeRtosTaskResumeFromISR(void *tcb) {
    Send(
        FreertosTaskResumeMessage{.handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(tcb)), .isFromISR = true}
    );
}

void SporFreeRtosTaskReadied(void *tcb) {
    Send(FreertosTaskReadiedMessage{.handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(tcb))});
}

void SporFreeRtosTaskDelayUntil(uint32_t timeToWake) {
    Send(FreertosTaskDelayUntilMessage{
        .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(xTaskGetCurrentTaskHandle())),
        .timeToWake = timeToWake
    });
    SporFreeRtosSetSwitchReason(FreeRtosSwitchReason::DELAYED, nullptr);
}

void SporFreeRtosTaskDelay(uint32_t ticksToDelay) {
    Send(FreertosTaskDelayMessage{
        .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(xTaskGetCurrentTaskHandle())),
        .ticksToDelay = ticksToDelay
    });
    SporFreeRtosSetSwitchReason(FreeRtosSwitchReason::DELAYED, nullptr);
}

void SporFreeRtosIncreaseTick() {}

void SporFreeRtosTaskIncrementTick(int wasSwitch) {}

void SporFreeRtosLowPowerIdleBegin() {}

void SporFreeRtosLowPowerIdleEnd() {}

void SporFreeRtosTaskPriorityInherit(void *tcb, uint32_t priority) {}

void SporFreeRtosTaskPriorityDisinherit(void *tcb, uint32_t priority) {}

void SporFreeRtosBlockingOnQueueSend(void *queue, uint8_t queueType) {
    switch (queueType) {
    case queueQUEUE_TYPE_BINARY_SEMAPHORE:
        SporFreeRtosSetSwitchReason(FreeRtosSwitchReason::BLOCKED_BINARY_SEMAPHORE_GIVE, queue);
        break;
    case queueQUEUE_TYPE_COUNTING_SEMAPHORE:
        SporFreeRtosSetSwitchReason(FreeRtosSwitchReason::COUNTING_SEMAPHORE_GIVE, queue);
        break;
    case queueQUEUE_TYPE_MUTEX:
    case queueQUEUE_TYPE_RECURSIVE_MUTEX:
        SporFreeRtosSetSwitchReason(FreeRtosSwitchReason::BLOCKED_MUTEX_UNLOCK, queue);
        break;
    case queueQUEUE_TYPE_BASE:
        SporFreeRtosSetSwitchReason(FreeRtosSwitchReason::BLOCKED_QUEUE_PUSH, queue);
        break;
    default:
        SporFreeRtosSetSwitchReason(FreeRtosSwitchReason::BLOCKED_OTHER, queue);
        break;
    }
}

void SporFreeRtosBlockingOnQueuePeek(void *queue, uint8_t queueType) {
    SporFreeRtosBlockingOnQueueReceive(queue, queueType);
}

void SporFreeRtosBlockingOnQueueReceive(void *queue, uint8_t queueType) {
    switch (queueType) {
    case queueQUEUE_TYPE_BINARY_SEMAPHORE:
        SporFreeRtosSetSwitchReason(FreeRtosSwitchReason::BLOCKED_BINARY_SEMAPHORE_RECEIVE, queue);
        break;
    case queueQUEUE_TYPE_COUNTING_SEMAPHORE:
        SporFreeRtosSetSwitchReason(FreeRtosSwitchReason::COUNTING_SEMAPHORE_TAKE, queue);
        break;
    case queueQUEUE_TYPE_MUTEX:
    case queueQUEUE_TYPE_RECURSIVE_MUTEX:
        SporFreeRtosSetSwitchReason(FreeRtosSwitchReason::BLOCKED_MUTEX_LOCK, queue);
        break;
    case queueQUEUE_TYPE_BASE:
        SporFreeRtosSetSwitchReason(FreeRtosSwitchReason::BLOCKED_QUEUE_POP, queue);
        break;
    default:
        SporFreeRtosSetSwitchReason(FreeRtosSwitchReason::BLOCKED_OTHER, queue);
        break;
    }
}

void SporFreeRtosBlockingOnStreamBufferSend(void *streamBuffer) {}

void SporFreeRtosBlockingOnStreamBufferReceive(void *streamBuffer) {}

void SporFreeRtosTaskNotifyTakeBlock(uint32_t index) {}

void SporFreeRtosTaskNotifyWaitBlock(uint32_t index) {
    SporFreeRtosSetSwitchReason(
        FreeRtosSwitchReason::TASK_NOTIFY_WAIT, reinterpret_cast<void *>(static_cast<uintptr_t>(index))
    );
}

void SporFreeRtosTaskNotify(void *tcb, uint32_t index, uint32_t action, uint32_t value) {
    Send(
        FreertosTaskNotifyMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(tcb)),
            .index = index,
            .action = action,
            .updatedValue = value,
            .isFromISR = false
        }
    );
}

void SporFreeRtosTaskNotifyFromISR(void *tcb, uint32_t index, uint32_t action, uint32_t value) {
    Send(
        FreertosTaskNotifyMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(tcb)),
            .index = index,
            .action = action,
            .updatedValue = value,
            .isFromISR = true
        }
    );
}

void SporFreeRtosTaskNotifyGiveFromISR(void *tcb, uint32_t index) {
    Send(
        FreertosTaskNotifyMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(tcb)),
            .index = index,
            .action = 0,
            .updatedValue = 0,
            .isFromISR = true
        }
    );
}

void SporFreeRtosTaskNotifyTake(uint32_t index) {}

void SporFreeRtosTaskNotifyWait(uint32_t index, uint32_t value) {
    Send(
        FreertosTaskNotifyReceivedMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(xTaskGetCurrentTaskHandle())),
            .index = index,
            .updatedValue = value
        }
    );
}

void SporFreeRtosQueueCreate(void *queue, uint32_t capacity, uint8_t queueType) {
    Send(
        FreertosQueueCreatedMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(queue)),
            .capacity = capacity,
            .queueType = static_cast<QueueType>(queueType)
        }
    );
}

void SporFreeRtosQueueCreateFailed(uint8_t type) {
    Send(FreertosQueueCreateFailedMessage{.queueType = (QueueType)type});
}

void SporFreeRtosQueueSend(void *queue, uint32_t updatedCount, uint8_t queueType, bool isFromISR) {
    Send(
        FreertosQueueSendMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(queue)),
            .updatedCount = updatedCount,
            .queueType = (QueueType)queueType,
            .isFromISR = isFromISR
        }
    );
}

void SporFreeRtosQueueSendFailed(void *queue, uint8_t queueType, bool isFromISR) {
    Send(
        FreertosQueueSendFailedMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(queue)),
            .queueType = (QueueType)queueType,
            .isFromISR = isFromISR
        }
    );
}

void SporFreeRtosQueueReceive(void *queue, uint32_t updatedCount, uint8_t queueType, bool isFromISR) {
    Send(
        FreertosQueueReceiveMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(queue)),
            .updatedCount = updatedCount,
            .queueType = (QueueType)queueType,
            .isFromISR = isFromISR
        }
    );
}

void SporFreeRtosQueueReceiveFailed(void *queue, uint8_t queueType, bool isFromISR) {
    Send(
        FreertosQueueReceiveFailedMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(queue)),
            .queueType = (QueueType)queueType,
            .isFromISR = isFromISR
        }
    );
}

void SporFreeRtosQueuePeek(void *queue, uint32_t updatedCount) {
    Send(
        FreertosQueuePeekMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(queue)),
            .updatedCount = updatedCount,
            .isFromISR = InInterruptContext()
        }
    );
}

void SporFreeRtosQueuePeekFailed(void *queue) {
    Send(
        FreertosQueuePeekFailedMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(queue)),
            .queueType = QueueType::BASE,
            .isFromISR = InInterruptContext()
        }
    );
}

void SporFreeRtosQueuePeekFromISR(void *queue, uint32_t updatedCount) {
    Send(
        FreertosQueuePeekMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(queue)),
            .updatedCount = updatedCount,
            .isFromISR = true
        }
    );
}

void SporFreeRtosQueuePeekFromISRFailed(void *queue) {
    Send(
        FreertosQueuePeekFailedMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(queue)),
            .queueType = QueueType::BASE,
            .isFromISR = true
        }
    );
}

void SporFreeRtosQueueDelete(void *queue) {
    Send(FreertosQueueDeletedMessage{.handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(queue))});
}

void SporFreeRtosQueueRegistryAdd(void *queue, const char *name) {
    Send(
        FreertosQueueRegistryMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(queue)), .name = name ? name : ""
        }
    );
}

void SporFreeRtosMutexCreate(void *mutex) {
    Send(
        FreertosQueueCreatedMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(mutex)),
            .capacity = 1,
            .queueType = QueueType::MUTEX
        }
    );
}

void SporFreeRtosMutexCreateFailed() {
    Send(FreertosQueueCreateFailedMessage{.queueType = QueueType::MUTEX});
}

void SporFreeRtosMutexGiveRecursive(void *mutex) {
    Send(
        FreertosQueueSendMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(mutex)),
            .updatedCount = 1,
            .queueType = QueueType::RECURSIVE_MUTEX,
            .isFromISR = InInterruptContext()
        }
    );
}

void SporFreeRtosMutexGiveRecursiveFailed(void *mutex) {
    Send(
        FreertosQueueSendFailedMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(mutex)),
            .queueType = QueueType::RECURSIVE_MUTEX,
            .isFromISR = InInterruptContext()
        }
    );
}

void SporFreeRtosMutexTakeRecursive(void *mutex) {
    Send(
        FreertosQueueReceiveMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(mutex)),
            .updatedCount = 0,
            .queueType = QueueType::RECURSIVE_MUTEX,
            .isFromISR = InInterruptContext()
        }
    );
}

void SporFreeRtosMutexTakeRecursiveFailed(void *mutex) {
    Send(
        FreertosQueueReceiveFailedMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(mutex)),
            .queueType = QueueType::RECURSIVE_MUTEX,
            .isFromISR = InInterruptContext()
        }
    );
}

void SporFreeRtosCountingSemaphoreCreate(void *semaphore, uint32_t maxCount, uint32_t initialCount) {
    Send(
        FreertosQueueCreatedMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(semaphore)),
            .capacity = maxCount,
            .queueType = QueueType::COUNTING_SEMAPHORE
        }
    );
    if (initialCount > 0) {
        Send(
            FreertosQueueSendMessage{
                .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(semaphore)),
                .updatedCount = initialCount,
                .queueType = QueueType::COUNTING_SEMAPHORE,
                .isFromISR = false
            }
        );
    }
}

void SporFreeRtosCountingSemaphoreCreateFailed() {
    Send(FreertosQueueCreateFailedMessage{.queueType = QueueType::COUNTING_SEMAPHORE});
}

void SporFreeRtosQueueSetSend(void *queue, uint32_t type) {}

void SporFreeRtosTimerCreate(void *timer, const char *name, uint32_t period, uint32_t autoReload) {
    Send(
        FreertosTimerCreatedMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(timer)),
            .period = period,
            .autoReload = autoReload != 0
        }
    );
}

void SporFreeRtosTimerCreateFailed() {
    Send(FreertosTimerCreateFailedMessage{});
}

void SporFreeRtosTimerCommandSend(void *timer, uint32_t commandId, uint32_t optionalValue, uint32_t returnValue) {
    Send(
        FreertosTimerCommandMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(timer)),
            .commandId = commandId,
            .optionalValue = optionalValue,
            .isFromISR = InInterruptContext()
        }
    );
}

void SporFreeRtosTimerExpired(void *timer) {
    Send(FreertosTimerExpiredMessage{.handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(timer))});
}

void SporFreeRtosTimerCommandReceived(void *timer, uint32_t commandId, uint32_t optionalValue) {
    Send(
        FreertosTimerCommandReceivedMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(timer)),
            .commandId = commandId,
            .optionalValue = optionalValue
        }
    );
}

void SporFreeRtosPendFuncCall(void *function, void *param1, uint32_t param2, uint32_t returnValue) {}

void SporFreeRtosPendFuncCallFromISR(void *function, void *param1, uint32_t param2, uint32_t returnValue) {}

void SporFreeRtosMalloc(void *ptr, size_t size) {
    Send(
        AllocData{.ptr = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ptr)), .size = static_cast<uint32_t>(size)}
    );
}

void SporFreeRtosFree(void *ptr) {
    Send(FreeData{.ptr = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ptr))});
}

void SporFreeRtosEventGroupCreate(void *eventGroup) {
    Send(FreertosEventGroupCreatedMessage{.handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(eventGroup))});
}

void SporFreeRtosEventGroupCreateFailed() {
    Send(FreertosEventGroupCreateFailedMessage{});
}

void SporFreeRtosEventGroupSyncBlock(void *eventGroup, uint32_t setBits, uint32_t waitBits) {
    SporFreeRtosSetSwitchReason(FreeRtosSwitchReason::BLOCKED_EVENT_GROUP, eventGroup);
}

void SporFreeRtosEventGroupSyncEnd(void *eventGroup, uint32_t setBits, uint32_t waitBits, uint32_t returnBits) {
    Send(
        FreertosEventGroupSyncMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(eventGroup)),
            .setBits = setBits,
            .waitBits = waitBits,
            .resultBits = returnBits
        }
    );
}

void SporFreeRtosEventGroupWaitBitsBlock(void *eventGroup, uint32_t waitBits) {
    SporFreeRtosSetSwitchReason(FreeRtosSwitchReason::BLOCKED_EVENT_GROUP, eventGroup);
}

void SporFreeRtosEventGroupWaitBitsEnd(void *eventGroup, uint32_t waitBits, uint32_t returnBits) {
    Send(
        FreertosEventGroupWaitBitsMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(eventGroup)),
            .waitBits = waitBits,
            .resultBits = returnBits
        }
    );
}

void SporFreeRtosEventGroupClearBits(void *eventGroup, uint32_t clearBits) {
    Send(
        FreertosEventGroupClearBitsMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(eventGroup)),
            .clearBits = clearBits,
            .isFromISR = false
        }
    );
}

void SporFreeRtosEventGroupClearBitsFromISR(void *eventGroup, uint32_t clearBits) {
    Send(
        FreertosEventGroupClearBitsMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(eventGroup)),
            .clearBits = clearBits,
            .isFromISR = true
        }
    );
}

void SporFreeRtosEventGroupSetBits(void *eventGroup, uint32_t setBits) {
    Send(
        FreertosEventGroupSetBitsMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(eventGroup)),
            .setBits = setBits,
            .isFromISR = false
        }
    );
}

void SporFreeRtosEventGroupSetBitsFromISR(void *eventGroup, uint32_t setBits) {
    Send(
        FreertosEventGroupSetBitsMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(eventGroup)),
            .setBits = setBits,
            .isFromISR = true
        }
    );
}

void SporFreeRtosEventGroupDelete(void *eventGroup) {
    Send(FreertosEventGroupDeletedMessage{.handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(eventGroup))});
}

void SporFreeRtosStreamBufferCreate(void *streamBuffer, size_t size, size_t triggerLevel) {
    Send(
        FreertosStreamBufferCreatedMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(streamBuffer)),
            .size = static_cast<uint32_t>(size),
            .triggerLevel = static_cast<uint32_t>(triggerLevel),
            .isMessageBuffer = false
        }
    );
}

void SporFreeRtosStreamBufferCreateFailed() {
    Send(FreertosStreamBufferCreateFailedMessage{});
}

void SporFreeRtosStreamBufferCreateStaticFailed() {
    Send(FreertosStreamBufferCreateFailedMessage{});
}

void SporFreeRtosStreamBufferDelete(void *streamBuffer) {
    Send(
        FreertosStreamBufferDeletedMessage{.handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(streamBuffer))}
    );
}

void SporFreeRtosStreamBufferReset(void *streamBuffer) {
    Send(
        FreertosStreamBufferResetMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(streamBuffer)), .isFromISR = false
        }
    );
}

void SporFreeRtosStreamBufferResetFromISR(void *streamBuffer) {
    Send(
        FreertosStreamBufferResetMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(streamBuffer)), .isFromISR = true
        }
    );
}

void SporFreeRtosStreamBufferSend(void *streamBuffer, size_t bytesSent) {
    Send(
        FreertosStreamBufferSendMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(streamBuffer)),
            .bytesSent = static_cast<uint32_t>(bytesSent),
            .isFromISR = false
        }
    );
}

void SporFreeRtosStreamBufferSendFailed(void *streamBuffer) {
    Send(
        FreertosStreamBufferSendFailedMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(streamBuffer)),
            .isFromISR = InInterruptContext()
        }
    );
}

void SporFreeRtosStreamBufferSendFromISR(void *streamBuffer, size_t bytesSent) {
    Send(
        FreertosStreamBufferSendMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(streamBuffer)),
            .bytesSent = static_cast<uint32_t>(bytesSent),
            .isFromISR = true
        }
    );
}

void SporFreeRtosStreamBufferReceive(void *streamBuffer, size_t bytesReceived) {
    Send(
        FreertosStreamBufferReceiveMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(streamBuffer)),
            .bytesReceived = static_cast<uint32_t>(bytesReceived),
            .isFromISR = false
        }
    );
}

void SporFreeRtosStreamBufferReceiveFailed(void *streamBuffer) {
    Send(
        FreertosStreamBufferReceiveFailedMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(streamBuffer)),
            .isFromISR = InInterruptContext()
        }
    );
}

void SporFreeRtosStreamBufferReceiveFromISR(void *streamBuffer, size_t bytesReceived) {
    Send(
        FreertosStreamBufferReceiveMessage{
            .handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(streamBuffer)),
            .bytesReceived = static_cast<uint32_t>(bytesReceived),
            .isFromISR = true
        }
    );
}

void SporFreeRtosIsrEnter() {
    Send(FreertosIsrEnterMessage{.isrNumber = __get_IPSR()});
}

void SporFreeRtosIsrExit() {
    Send(FreertosIsrExitMessage{.isrNumber = __get_IPSR()});
}

void SporFreeRtosIsrExitToScheduler() {
    Send(FreertosIsrExitToSchedulerMessage{.isrNumber = __get_IPSR()});
}

void SporFreeRtosFunctionEnter(void *function) {
    Send(ZoneBeginData{.ptr = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(function))});
}

void SporFreeRtosFunctionReturn(void *function, uint32_t returnValue) {
    Send(ZoneEndData{});
}