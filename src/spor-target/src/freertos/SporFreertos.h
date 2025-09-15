// clang-format off
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// #include "FreeRTOS.h"

#if !defined(configUSE_TRACE_FACILITY) || (configUSE_TRACE_FACILITY == 0)
#error "Enable FreeRTOS trace facilities using #define configUSE_TRACE_FACILITY 1"
#endif

#ifdef __cplusplus
extern "C" {
#endif

void SporFreeRtosStart();
void SporFreeRtosEnd();
void SporFreeRtosTaskSwitchedIn(void *tcb);
void SporFreeRtosTaskSwitchedOut(void *tcb, bool stillReady);
void SporFreeRtosTaskCreated(void *tcb, const char *name, uint32_t priority);
void SporFreeRtosTaskDeleted(void *tcb);
void SporFreeRtosTaskPrioritySet(void *tcb, uint32_t oldPriority, uint32_t newPriority);
void SporFreeRtosTaskSuspend(void *tcb);
void SporFreeRtosTaskResume(void *tcb);
void SporFreeRtosTaskResumeFromISR(void *tcb);
void SporFreeRtosTaskReadied(void *tcb);
void SporFreeRtosTaskDelayUntil(uint32_t timeToWake);
void SporFreeRtosTaskDelay(uint32_t ticksToDelay);
void SporFreeRtosIncreaseTick();
void SporFreeRtosTaskIncrementTick(int wasSwitch);
void SporFreeRtosLowPowerIdleBegin();
void SporFreeRtosLowPowerIdleEnd();
void SporFreeRtosTaskPriorityInherit(void *tcb, uint32_t priority);
void SporFreeRtosTaskPriorityDisinherit(void *tcb, uint32_t priority);
void SporFreeRtosBlockingOnQueueSend(void *queue, uint8_t queueType);
void SporFreeRtosBlockingOnQueuePeek(void *queue, uint8_t queueType);
void SporFreeRtosBlockingOnQueueReceive(void *queue, uint8_t queueType);
void SporFreeRtosBlockingOnStreamBufferSend(void *streamBuffer);
void SporFreeRtosBlockingOnStreamBufferReceive(void *streamBuffer);
void SporFreeRtosTaskNotifyTakeBlock(uint32_t index);
void SporFreeRtosTaskNotifyWaitBlock(uint32_t index);
void SporFreeRtosTaskNotify(void *tcb, uint32_t index, uint32_t action, uint32_t value);
void SporFreeRtosTaskNotifyFromISR(void *tcb, uint32_t index, uint32_t action, uint32_t value);
void SporFreeRtosTaskNotifyGiveFromISR(void *tcb, uint32_t index);
void SporFreeRtosTaskNotifyTake(uint32_t index);
void SporFreeRtosTaskNotifyWait(uint32_t index, uint32_t value);
void SporFreeRtosQueueCreate(void *queue, uint32_t capacity, uint8_t queueType);
void SporFreeRtosQueueCreateFailed(uint8_t type);
void SporFreeRtosQueueSend(void *queue, uint32_t updatedCount, uint8_t queueType, bool isFromISR);
void SporFreeRtosQueueSendFailed(void *queue, uint8_t queueType, bool isFromISR);
void SporFreeRtosQueueReceive(void *queue, uint32_t updatedCount, uint8_t queueType, bool isFromISR);
void SporFreeRtosQueueReceiveFailed(void *queue, uint8_t queueType, bool isFromISR);
void SporFreeRtosQueuePeek(void *queue, uint32_t updatedCount);
void SporFreeRtosQueuePeekFailed(void *queue);
void SporFreeRtosQueuePeekFromISR(void *queue, uint32_t updatedCount);
void SporFreeRtosQueuePeekFromISRFailed(void *queue);
void SporFreeRtosQueueDelete(void *queue);
void SporFreeRtosQueueRegistryAdd(void *queue, const char *name);
void SporFreeRtosMutexCreate(void *mutex);
void SporFreeRtosMutexCreateFailed();
void SporFreeRtosMutexGiveRecursive(void *mutex);
void SporFreeRtosMutexGiveRecursiveFailed(void *mutex);
void SporFreeRtosMutexTakeRecursive(void *mutex);
void SporFreeRtosMutexTakeRecursiveFailed(void *mutex);
void SporFreeRtosCountingSemaphoreCreate(void *semaphore, uint32_t maxCount, uint32_t initialCount);
void SporFreeRtosCountingSemaphoreCreateFailed();
void SporFreeRtosQueueSetSend(void *queue, uint32_t type);
void SporFreeRtosTimerCreate(void *timer, const char *name, uint32_t period, uint32_t autoReload);
void SporFreeRtosTimerCreateFailed();
void SporFreeRtosTimerCommandSend(void *timer, uint32_t commandId, uint32_t optionalValue, uint32_t returnValue);
void SporFreeRtosTimerExpired(void *timer);
void SporFreeRtosTimerCommandReceived(void *timer, uint32_t commandId, uint32_t optionalValue);
void SporFreeRtosPendFuncCall(void *function, void *param1, uint32_t param2, uint32_t returnValue);
void SporFreeRtosPendFuncCallFromISR(void *function, void *param1, uint32_t param2, uint32_t returnValue);
void SporFreeRtosMalloc(void *ptr, size_t size);
void SporFreeRtosFree(void *ptr);
void SporFreeRtosEventGroupCreate(void *eventGroup);
void SporFreeRtosEventGroupCreateFailed();
void SporFreeRtosEventGroupSyncBlock(void *eventGroup, uint32_t setBits, uint32_t waitBits);
void SporFreeRtosEventGroupSyncEnd(void *eventGroup, uint32_t setBits, uint32_t waitBits, uint32_t returnBits);
void SporFreeRtosEventGroupWaitBitsBlock(void *eventGroup, uint32_t waitBits);
void SporFreeRtosEventGroupWaitBitsEnd(void *eventGroup, uint32_t waitBits, uint32_t returnBits);
void SporFreeRtosEventGroupClearBits(void *eventGroup, uint32_t clearBits);
void SporFreeRtosEventGroupClearBitsFromISR(void *eventGroup, uint32_t clearBits);
void SporFreeRtosEventGroupSetBits(void *eventGroup, uint32_t setBits);
void SporFreeRtosEventGroupSetBitsFromISR(void *eventGroup, uint32_t setBits);
void SporFreeRtosEventGroupDelete(void *eventGroup);
void SporFreeRtosStreamBufferCreate(void *streamBuffer, size_t size, size_t triggerLevel);
void SporFreeRtosStreamBufferCreateFailed();
void SporFreeRtosStreamBufferCreateStaticFailed();
void SporFreeRtosStreamBufferDelete(void *streamBuffer);
void SporFreeRtosStreamBufferReset(void *streamBuffer);
void SporFreeRtosStreamBufferResetFromISR(void *streamBuffer);
void SporFreeRtosStreamBufferSend(void *streamBuffer, size_t bytesSent);
void SporFreeRtosStreamBufferSendFailed(void *streamBuffer);
void SporFreeRtosStreamBufferSendFromISR(void *streamBuffer, size_t bytesSent);
void SporFreeRtosStreamBufferReceive(void *streamBuffer, size_t bytesReceived);
void SporFreeRtosStreamBufferReceiveFailed(void *streamBuffer);
void SporFreeRtosStreamBufferReceiveFromISR(void *streamBuffer, size_t bytesReceived);
void SporFreeRtosIsrEnter();
void SporFreeRtosIsrExit();
void SporFreeRtosIsrExitToScheduler();
void SporFreeRtosFunctionEnter(void *function);
void SporFreeRtosFunctionReturn(void *function, uint32_t returnValue);

#ifdef __cplusplus
}
#endif

#define traceSTART() SporFreeRtosStart()
#define traceEND() SporFreeRtosEnd()
#define traceTASK_SWITCHED_IN() SporFreeRtosTaskSwitchedIn(xTaskGetCurrentTaskHandle())
#define traceSTARTING_SCHEDULER(xIdlehandles)
#define traceINCREASE_TICK_COUNT() SporFreeRtosIncreaseTick()
#define traceTASK_SWITCHED_OUT()                                                                                       \
    do {                                                                                                               \
        SporFreeRtosTaskSwitchedOut(                                                                                  \
            xTaskGetCurrentTaskHandle(), (xTaskGetCurrentTaskHandle()->xStateListItem.pxContainer ==                   \
                                                                                                                       \
                                          &pxReadyTasksLists[xTaskGetCurrentTaskHandle()->uxPriority]) ||              \
                                                                                                                       \
                                             (xTaskGetCurrentTaskHandle()->xStateListItem.pxContainer == NULL)         \
                                                                                                                       \
        );                                                                                                             \
    } while (0)
#define traceLOW_POWER_IDLE_BEGIN() SporFreeRtosLowPowerIdleBegin()
#define traceLOW_POWER_IDLE_END() SporFreeRtosLowPowerIdleEnd()
#define traceTASK_PRIORITY_INHERIT(pxTCB, uxPriority) SporFreeRtosTaskPriorityInherit(pxTCB, uxPriority)
#define traceTASK_PRIORITY_DISINHERIT(pxTCB, uxPriority) SporFreeRtosTaskPriorityDisinherit(pxTCB, uxPriority)
#define traceMOVED_TASK_TO_READY_STATE(pxTCB) SporFreeRtosTaskReadied(pxTCB)
#define tracePOST_MOVED_TASK_TO_READY_STATE(pxTCB) SporFreeRtosTaskReadied(pxTCB)
#define traceMOVED_TASK_TO_DELAYED_LIST()
#define traceMOVED_TASK_TO_OVERFLOW_DELAYED_LIST()
#define traceQUEUE_CREATE(pxQueue) SporFreeRtosQueueCreate(pxQueue, (pxQueue) ? (pxQueue)->uxLength : 0, (pxQueue) ? (pxQueue)->ucQueueType : 0)
#define traceQUEUE_CREATE_FAILED(ucQueueType) SporFreeRtosQueueCreateFailed(ucQueueType)
#define traceCREATE_MUTEX(pxMutex) SporFreeRtosMutexCreate(pxMutex)
#define traceCREATE_MUTEX_FAILED() SporFreeRtosMutexCreateFailed()
#define traceGIVE_MUTEX_RECURSIVE(pxMutex) SporFreeRtosMutexGiveRecursive(pxMutex)
#define traceGIVE_MUTEX_RECURSIVE_FAILED(pxMutex) SporFreeRtosMutexGiveRecursiveFailed(pxMutex)
#define traceTAKE_MUTEX_RECURSIVE(pxMutex) SporFreeRtosMutexTakeRecursive(pxMutex)
#define traceTAKE_MUTEX_RECURSIVE_FAILED(pxMutex) SporFreeRtosMutexTakeRecursiveFailed(pxMutex)
#define traceCREATE_COUNTING_SEMAPHORE() SporFreeRtosCountingSemaphoreCreate(xHandle, uxMaxCount, uxInitialCount)
#define traceCREATE_COUNTING_SEMAPHORE_FAILED() SporFreeRtosCountingSemaphoreCreateFailed()
#define traceQUEUE_SET_SEND(pxQueue) SporFreeRtosQueueSetSend(pxQueue, pxQueue->ucQueueType)
#define traceQUEUE_SEND(pxQueue)                                                                                       \
    SporFreeRtosQueueSend(                                                                                            \
        pxQueue, (pxQueue) ? (pxQueue)->uxMessagesWaiting + 1 : 0, (pxQueue) ? (pxQueue)->ucQueueType : 0, false       \
    )
#define traceQUEUE_SEND_FAILED(pxQueue)                                                                                \
    SporFreeRtosQueueSendFailed(pxQueue, (pxQueue) ? (pxQueue)->ucQueueType : 0, false)
#define traceQUEUE_RECEIVE(pxQueue)                                                                                    \
    SporFreeRtosQueueReceive(                                                                                         \
        pxQueue, (pxQueue) ? (pxQueue)->uxMessagesWaiting - 1 : 0, (pxQueue) ? (pxQueue)->ucQueueType : 0, false       \
    )
#define traceQUEUE_PEEK(pxQueue) SporFreeRtosQueuePeek(pxQueue, (pxQueue) ? (pxQueue)->uxMessagesWaiting : 0)
#define traceQUEUE_PEEK_FAILED(pxQueue) SporFreeRtosQueuePeekFailed(pxQueue)
#define traceQUEUE_PEEK_FROM_ISR(pxQueue)                                                                              \
    SporFreeRtosQueuePeekFromISR(pxQueue, (pxQueue) ? (pxQueue)->uxMessagesWaiting : 0)
#define traceQUEUE_RECEIVE_FAILED(pxQueue)                                                                             \
    SporFreeRtosQueueReceiveFailed(pxQueue, (pxQueue) ? (pxQueue)->ucQueueType : 0, false)
#define traceQUEUE_SEND_FROM_ISR(pxQueue)                                                                              \
    SporFreeRtosQueueSend(                                                                                            \
        pxQueue, (pxQueue) ? (pxQueue)->uxMessagesWaiting + 1 : 0, (pxQueue) ? (pxQueue)->ucQueueType : 0, true        \
    )
#define traceQUEUE_SEND_FROM_ISR_FAILED(pxQueue)                                                                       \
    SporFreeRtosQueueSendFailed(pxQueue, (pxQueue) ? (pxQueue)->ucQueueType : 0, true)
#define traceQUEUE_RECEIVE_FROM_ISR(pxQueue)                                                                           \
    SporFreeRtosQueueReceive(                                                                                         \
        pxQueue, (pxQueue) ? (pxQueue)->uxMessagesWaiting - 1 : 0, (pxQueue) ? (pxQueue)->ucQueueType : 0, true        \
    )
#define traceQUEUE_RECEIVE_FROM_ISR_FAILED(pxQueue)                                                                    \
    SporFreeRtosQueueReceiveFailed(pxQueue, (pxQueue) ? (pxQueue)->ucQueueType : 0, true)
#define traceQUEUE_PEEK_FROM_ISR_FAILED(pxQueue) SporFreeRtosQueuePeekFromISRFailed(pxQueue)
#define traceQUEUE_DELETE(pxQueue) SporFreeRtosQueueDelete(pxQueue)
#define traceTASK_CREATE(pxTask) SporFreeRtosTaskCreated(pxTask, (pxTask) ? (pxTask)->pcTaskName : "", (pxTask) ? (pxTask)->uxPriority : 0)
#define traceTASK_CREATE_FAILED()
#define traceTASK_DELETE(pxTask) SporFreeRtosTaskDeleted(pxTask)
#define traceTASK_DELAY_UNTIL(xTimeToWake) SporFreeRtosTaskDelayUntil(xTimeToWake)
#define traceTASK_DELAY() SporFreeRtosTaskDelay(xTicksToDelay)
#define traceTASK_PRIORITY_SET(pxTask, uxNewPriority)                                                                  \
    SporFreeRtosTaskPrioritySet(pxTask, (pxTask) ? (pxTask)->uxPriority : 0, uxNewPriority)
#define traceTASK_SUSPEND(pxTask) SporFreeRtosTaskSuspend(pxTask)
#define traceTASK_RESUME(pxTask) SporFreeRtosTaskResume(pxTask)
#define traceTASK_RESUME_FROM_ISR(pxTask) SporFreeRtosTaskResumeFromISR(pxTask)
#define traceTASK_INCREMENT_TICK(xTicksToJump) SporFreeRtosTaskIncrementTick(xTicksToJump)
#define traceTIMER_CREATE(pxTimer)                                                                                     \
    SporFreeRtosTimerCreate(                                                                                          \
        pxTimer, (pxTimer) ? (pxTimer)->pcTimerName : "", (pxTimer) ? (pxTimer)->xTimerPeriodInTicks : 0,              \
        (pxTimer) ? ((pxTimer)->ucStatus & 1) : 0                                                                      \
    )
#define traceTIMER_CREATE_FAILED() SporFreeRtosTimerCreateFailed()
#define traceTIMER_COMMAND_SEND(pxTimer, xCommandID, xOptionalValue, xReturn)                                          \
    SporFreeRtosTimerCommandSend(pxTimer, xCommandID, xOptionalValue, xReturn)
#define traceTIMER_EXPIRED(pxTimer) SporFreeRtosTimerExpired(pxTimer)
#define traceTIMER_COMMAND_RECEIVED(pxTimer, xCommandID, xOptionalValue)                                               \
    SporFreeRtosTimerCommandReceived(pxTimer, xCommandID, xOptionalValue)
#define traceMALLOC(pvAddress, uiSize) SporFreeRtosMalloc(pvAddress, uiSize)
#define traceFREE(pvAddress) SporFreeRtosFree(pvAddress)
#define traceEVENT_GROUP_CREATE(xEventGroup) SporFreeRtosEventGroupCreate(xEventGroup)
#define traceEVENT_GROUP_CREATE_FAILED() SporFreeRtosEventGroupCreateFailed()
#define traceEVENT_GROUP_SYNC_BLOCK(xEventGroup, uxBitsToSet, uxBitsToWaitFor)                                         \
    SporFreeRtosEventGroupSyncBlock(xEventGroup, uxBitsToSet, uxBitsToWaitFor)
#define traceEVENT_GROUP_SYNC_END(xEventGroup, uxBitsToSet, uxBitsToWaitFor, xReturn)                                  \
    SporFreeRtosEventGroupSyncEnd(xEventGroup, uxBitsToSet, uxBitsToWaitFor, xReturn)
#define traceEVENT_GROUP_WAIT_BITS_BLOCK(xEventGroup, uxBitsToWaitFor)                                                 \
    SporFreeRtosEventGroupWaitBitsBlock(xEventGroup, uxBitsToWaitFor)
#define traceEVENT_GROUP_WAIT_BITS_END(xEventGroup, uxBitsToWaitFor, xReturn)                                          \
    SporFreeRtosEventGroupWaitBitsEnd(xEventGroup, uxBitsToWaitFor, xReturn)
#define traceEVENT_GROUP_CLEAR_BITS(xEventGroup, uxBitsToClear)                                                        \
    SporFreeRtosEventGroupClearBits(xEventGroup, uxBitsToClear)
#define traceEVENT_GROUP_CLEAR_BITS_FROM_ISR(xEventGroup, uxBitsToClear)                                               \
    SporFreeRtosEventGroupClearBitsFromISR(xEventGroup, uxBitsToClear)
#define traceEVENT_GROUP_SET_BITS(xEventGroup, uxBitsToSet) SporFreeRtosEventGroupSetBits(xEventGroup, uxBitsToSet)
#define traceEVENT_GROUP_SET_BITS_FROM_ISR(xEventGroup, uxBitsToSet)                                                   \
    SporFreeRtosEventGroupSetBitsFromISR(xEventGroup, uxBitsToSet)
#define traceEVENT_GROUP_DELETE(xEventGroup) SporFreeRtosEventGroupDelete(xEventGroup)
#define tracePEND_FUNC_CALL(xFunctionToPend, pvParameter1, ulParameter2, xReturn)                                      \
    SporFreeRtosPendFuncCall(xFunctionToPend, pvParameter1, ulParameter2, xReturn)
#define tracePEND_FUNC_CALL_FROM_ISR(xFunctionToPend, pvParameter1, ulParameter2, xReturn)                             \
    SporFreeRtosPendFuncCallFromISR(xFunctionToPend, pvParameter1, ulParameter2, xReturn)
#define traceQUEUE_REGISTRY_ADD(pxQueue, pcQueueName) SporFreeRtosQueueRegistryAdd(pxQueue, pcQueueName)
#define traceTASK_NOTIFY_TAKE_BLOCK(uxIndexToWait) SporFreeRtosTaskNotifyTakeBlock(uxIndexToWait)
#define traceTASK_NOTIFY_TAKE(uxIndexToWait) SporFreeRtosTaskNotifyTake(uxIndexToWait)
#define traceTASK_NOTIFY_WAIT_BLOCK(uxIndexToWait) SporFreeRtosTaskNotifyWaitBlock(uxIndexToWait)
#define traceTASK_NOTIFY_WAIT(uxIndexToWait)                                                                           \
    SporFreeRtosTaskNotifyWait(                                                                                       \
        uxIndexToWait, xTaskGetCurrentTaskHandle()->ulNotifiedValue[uxIndexToWait] & ~ulBitsToClearOnExit              \
    )
#define traceTASK_NOTIFY(uxIndexToNotify)                                                                              \
    SporFreeRtosTaskNotify(pxTCB, uxIndexToNotify, eAction, pxTCB->ulNotifiedValue[uxIndexToNotify])
#define traceTASK_NOTIFY_FROM_ISR(uxIndexToNotify)                                                                     \
    SporFreeRtosTaskNotifyFromISR(pxTCB, uxIndexToNotify, eAction, pxTCB->ulNotifiedValue[uxIndexToNotify])
#define traceTASK_NOTIFY_GIVE_FROM_ISR(uxIndexToNotify) SporFreeRtosTaskNotifyGiveFromISR(pxTCB, uxIndexToNotify)
#define traceISR_EXIT_TO_SCHEDULER() SporFreeRtosIsrExitToScheduler()
#define traceISR_EXIT() SporFreeRtosIsrExit()
#define traceISR_ENTER() SporFreeRtosIsrEnter()
#define traceSTREAM_BUFFER_CREATE_FAILED(type) SporFreeRtosStreamBufferCreateFailed()
#define traceSTREAM_BUFFER_CREATE_STATIC_FAILED(xReturn, type) SporFreeRtosStreamBufferCreateStaticFailed()
#define traceSTREAM_BUFFER_CREATE(pxStreamBuffer, xIsMessageBuffer)                                                    \
    SporFreeRtosStreamBufferCreate(                                                                                   \
        pxStreamBuffer, (pxStreamBuffer) ? (pxStreamBuffer)->xLength : 0,                                              \
        (pxStreamBuffer) ? (pxStreamBuffer)->xTriggerLevelBytes : 0                                                    \
    )
#define traceSTREAM_BUFFER_DELETE(pxStreamBuffer) SporFreeRtosStreamBufferDelete(pxStreamBuffer)
#define traceSTREAM_BUFFER_RESET(pxStreamBuffer) SporFreeRtosStreamBufferReset(pxStreamBuffer)
#define traceSTREAM_BUFFER_RESET_FROM_ISR(pxStreamBuffer) SporFreeRtosStreamBufferResetFromISR(pxStreamBuffer)
#define traceBLOCKING_ON_STREAM_BUFFER_SEND(pxStreamBuffer) SporFreeRtosBlockingOnStreamBufferSend(pxStreamBuffer)
#define traceSTREAM_BUFFER_SEND(pxStreamBuffer, xBytesSent) SporFreeRtosStreamBufferSend(pxStreamBuffer, xBytesSent)
#define traceSTREAM_BUFFER_SEND_FAILED(pxStreamBuffer) SporFreeRtosStreamBufferSendFailed(pxStreamBuffer)
#define traceSTREAM_BUFFER_SEND_FROM_ISR(pxStreamBuffer, xBytesSent)                                                   \
    SporFreeRtosStreamBufferSendFromISR(pxStreamBuffer, xBytesSent)
#define traceBLOCKING_ON_STREAM_BUFFER_RECEIVE(pxStreamBuffer)                                                         \
    SporFreeRtosBlockingOnStreamBufferReceive(pxStreamBuffer)
#define traceSTREAM_BUFFER_RECEIVE(pxStreamBuffer, xReceivedLength)                                                    \
    SporFreeRtosStreamBufferReceive(pxStreamBuffer, xReceivedLength)
#define traceSTREAM_BUFFER_RECEIVE_FAILED(pxStreamBuffer) SporFreeRtosStreamBufferReceiveFailed(pxStreamBuffer)
#define traceSTREAM_BUFFER_RECEIVE_FROM_ISR(pxStreamBuffer, xReceivedLength)                                           \
    SporFreeRtosStreamBufferReceiveFromISR(pxStreamBuffer, xReceivedLength)

#define traceBLOCKING_ON_QUEUE_SEND(pxQueue)                                                                           \
    SporFreeRtosBlockingOnQueueSend(pxQueue, (pxQueue) ? (pxQueue)->ucQueueType : 0)
#define traceBLOCKING_ON_QUEUE_PEEK(pxQueue)                                                                           \
    SporFreeRtosBlockingOnQueuePeek(pxQueue, (pxQueue) ? (pxQueue)->ucQueueType : 0)
#define traceBLOCKING_ON_QUEUE_RECEIVE(pxQueue)                                                                        \
    SporFreeRtosBlockingOnQueueReceive(pxQueue, (pxQueue) ? (pxQueue)->ucQueueType : 0)
#define traceTASK_NOTIFY_WAIT_BLOCK(uxIndexToWait) SporFreeRtosTaskNotifyWaitBlock(uxIndexToWait)
#define traceEVENT_GROUP_WAIT_BITS_BLOCK(xEventGroup, uxBitsToWaitFor)                                                 \
    SporFreeRtosEventGroupWaitBitsBlock(xEventGroup, uxBitsToWaitFor)
#define traceEVENT_GROUP_SYNC_BLOCK(xEventGroup, uxBitsToSet, uxBitsToWaitFor)                                         \
    SporFreeRtosEventGroupSyncBlock(xEventGroup, uxBitsToSet, uxBitsToWaitFor)
