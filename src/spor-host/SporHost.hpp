#pragma once

#include <atomic>
#include <string>
#include <unordered_map>

#include "spor-host/Decoder.hpp"
#include "State.hpp"

struct SporHost : State, IMessageHandler {
    static SporHost &GetInstance() {
        static SporHost instance;
        return instance;
    }

    void HandleMessage(const ZoneBeginData &msg) override;
    void HandleMessage(const ZoneEndData &msg) override;
    void HandleMessage(const ZoneTextMessage &msg) override;
    void HandleMessage(const ZoneValueData &msg) override;
    void HandleMessage(const ZoneColorData &msg) override;
    void HandleMessage(const PlotMessage &msg) override;
    void HandleMessage(const PlotConfigMessage &msg) override;
    void HandleMessage(const MessageTextMessage &msg) override;
    void HandleMessage(const AllocMessage &msg) override;
    void HandleMessage(const FreeMessage &msg) override;
    void HandleMessage(const FreertosTaskCreatedMessage &msg) override;
    void HandleMessage(const FreertosTaskSwitchedInMessage &msg) override;
    void HandleMessage(const FreertosTaskSwitchedOutMessage &msg) override;
    void HandleMessage(const FreertosTaskReadiedMessage &msg) override;
    void HandleMessage(const FreertosQueueCreatedMessage &msg) override;
    void HandleMessage(const FreertosQueuePeekMessage &msg) override;
    void HandleMessage(const FreertosQueueSendMessage &msg) override;
    void HandleMessage(const FreertosQueueReceiveMessage &msg) override;
    void HandleMessage(const FreertosTaskNotifyMessage &msg) override;
    void HandleMessage(const FreertosTaskNotifyReceivedMessage &msg) override;
    void HandleMessage(const FreertosTaskDeletedMessage &msg) override;
    void HandleMessage(const FreertosTaskPrioritySetMessage &msg) override;
    void HandleMessage(const FreertosTaskSuspendMessage &msg) override;
    void HandleMessage(const FreertosTaskResumeMessage &msg) override;
    void HandleMessage(const FreertosTaskDelayMessage &msg) override;
    void HandleMessage(const FreertosTaskDelayUntilMessage &msg) override;
    void HandleMessage(const FreertosTimerCreatedMessage &msg) override;
    void HandleMessage(const FreertosTimerCommandMessage &msg) override;
    void HandleMessage(const FreertosTimerExpiredMessage &msg) override;
    void HandleMessage(const FreertosEventGroupCreatedMessage &msg) override;
    void HandleMessage(const FreertosEventGroupDeletedMessage &msg) override;
    void HandleMessage(const FreertosEventGroupSyncMessage &msg) override;
    void HandleMessage(const FreertosEventGroupWaitBitsMessage &msg) override;
    void HandleMessage(const FreertosEventGroupClearBitsMessage &msg) override;
    void HandleMessage(const FreertosEventGroupSetBitsMessage &msg) override;
    void HandleMessage(const FreertosStreamBufferCreatedMessage &msg) override;
    void HandleMessage(const FreertosStreamBufferDeletedMessage &msg) override;
    void HandleMessage(const FreertosStreamBufferSendMessage &msg) override;
    void HandleMessage(const FreertosStreamBufferReceiveMessage &msg) override;
    void HandleMessage(const FreertosStreamBufferResetMessage &msg) override;
    void HandleMessage(const FreertosIsrEnterMessage &msg) override;
    void HandleMessage(const FreertosIsrExitMessage &msg) override;
    void HandleMessage(const FreertosIsrExitToSchedulerMessage &msg) override;
    void HandleMessage(const FunctionTraceEnterData &msg) override;
    void HandleMessage(const FunctionTraceExitData &msg) override;
    void HandleMessage(const DeclarePointerTypeMessage &msg) override;
    void HandleMessage(const FlowBeginMessage &msg) override;
    void HandleMessage(const FlowEndMessage &msg) override;
    void HandleMessage(const SystemInfoData &msg) override;
    void HandleMessage(const InterruptConfigMessage &msg) override;
    void HandleMessage(const InterruptEnterData &msg) override;
    void HandleMessage(const InterruptExitData &msg) override;
    void HandleMessage(const FreertosQueueDeletedMessage &msg) override;
    void HandleMessage(const FreertosQueueRegistryMessage &msg) override;
    void HandleMessage(const FreertosQueueCreateFailedMessage &msg) override;
    void HandleMessage(const FreertosQueueSendFailedMessage &msg) override;
    void HandleMessage(const FreertosQueueReceiveFailedMessage &msg) override;
    void HandleMessage(const FreertosQueuePeekFailedMessage &msg) override;
    void HandleMessage(const FreertosTimerCreateFailedMessage &msg) override;
    void HandleMessage(const FreertosTimerCommandReceivedMessage &msg) override;
    void HandleMessage(const FreertosStreamBufferCreateFailedMessage &msg) override;
    void HandleMessage(const FreertosStreamBufferSendFailedMessage &msg) override;
    void HandleMessage(const FreertosStreamBufferReceiveFailedMessage &msg) override;
    void HandleMessage(const FreertosEventGroupCreateFailedMessage &msg) override;
    void HandleMessage(const PointerAnnounceMessage &msg) override;
    void HandleMessage(const PointerSetNameMessage &msg) override;

    void OnCycleCount(uint32_t cycles) override;
    void OnConsoleLog(const void *data, size_t length) override;

    SporHost() = default;
    SporHost(const SporHost &) = delete;
    SporHost &operator=(const SporHost &) = delete;
};