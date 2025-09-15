#pragma once

#include <atomic>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include "Dispatcher.hpp"
#include "spor-common/Messages.hpp"
#include "zpp_bits.h"

enum class DecoderState { WAITING_FOR_TYPE, RECEIVING_DATA };

class IMessageHandler {
public:
    virtual ~IMessageHandler() = default;

    template <typename T>
    void OnMessage(const T &msg) {
        HandleMessage(msg);
    }

    virtual void HandleMessage(const ZoneBeginData &msg) = 0;
    virtual void HandleMessage(const ZoneEndData &msg) = 0;
    virtual void HandleMessage(const ZoneTextMessage &msg) = 0;
    virtual void HandleMessage(const ZoneValueData &msg) = 0;
    virtual void HandleMessage(const ZoneColorData &msg) = 0;
    virtual void HandleMessage(const PlotMessage &msg) = 0;
    virtual void HandleMessage(const PlotConfigMessage &msg) = 0;
    virtual void HandleMessage(const MessageTextMessage &msg) = 0;
    virtual void HandleMessage(const AllocMessage &msg) = 0;
    virtual void HandleMessage(const FreeMessage &msg) = 0;
    virtual void HandleMessage(const FreertosTaskCreatedMessage &msg) = 0;
    virtual void HandleMessage(const FreertosTaskSwitchedInMessage &msg) = 0;
    virtual void HandleMessage(const FreertosTaskSwitchedOutMessage &msg) = 0;
    virtual void HandleMessage(const FreertosTaskReadiedMessage &msg) = 0;
    virtual void HandleMessage(const FreertosQueueCreatedMessage &msg) = 0;
    virtual void HandleMessage(const FreertosQueuePeekMessage &msg) = 0;
    virtual void HandleMessage(const FreertosQueueSendMessage &msg) = 0;
    virtual void HandleMessage(const FreertosQueueReceiveMessage &msg) = 0;
    virtual void HandleMessage(const FreertosTaskNotifyMessage &msg) = 0;
    virtual void HandleMessage(const FreertosTaskNotifyReceivedMessage &msg) = 0;
    virtual void HandleMessage(const FreertosTaskDeletedMessage &msg) = 0;
    virtual void HandleMessage(const FreertosTaskPrioritySetMessage &msg) = 0;
    virtual void HandleMessage(const FreertosTaskSuspendMessage &msg) = 0;
    virtual void HandleMessage(const FreertosTaskResumeMessage &msg) = 0;
    virtual void HandleMessage(const FreertosTimerCreatedMessage &msg) = 0;
    virtual void HandleMessage(const FreertosTimerCommandMessage &msg) = 0;
    virtual void HandleMessage(const FreertosTimerExpiredMessage &msg) = 0;
    virtual void HandleMessage(const FreertosEventGroupCreatedMessage &msg) = 0;
    virtual void HandleMessage(const FreertosEventGroupDeletedMessage &msg) = 0;
    virtual void HandleMessage(const FreertosEventGroupSyncMessage &msg) = 0;
    virtual void HandleMessage(const FreertosEventGroupWaitBitsMessage &msg) = 0;
    virtual void HandleMessage(const FreertosEventGroupClearBitsMessage &msg) = 0;
    virtual void HandleMessage(const FreertosEventGroupSetBitsMessage &msg) = 0;
    virtual void HandleMessage(const FreertosStreamBufferCreatedMessage &msg) = 0;
    virtual void HandleMessage(const FreertosStreamBufferDeletedMessage &msg) = 0;
    virtual void HandleMessage(const FreertosStreamBufferSendMessage &msg) = 0;
    virtual void HandleMessage(const FreertosStreamBufferReceiveMessage &msg) = 0;
    virtual void HandleMessage(const FreertosStreamBufferResetMessage &msg) = 0;
    virtual void HandleMessage(const FreertosIsrEnterMessage &msg) = 0;
    virtual void HandleMessage(const FreertosIsrExitMessage &msg) = 0;
    virtual void HandleMessage(const FreertosIsrExitToSchedulerMessage &msg) = 0;
    virtual void HandleMessage(const FunctionTraceEnterData &msg) = 0;
    virtual void HandleMessage(const FunctionTraceExitData &msg) = 0;
    virtual void HandleMessage(const DeclarePointerTypeMessage &msg) = 0;
    virtual void HandleMessage(const FlowBeginMessage &msg) = 0;
    virtual void HandleMessage(const FlowEndMessage &msg) = 0;
    virtual void HandleMessage(const SystemInfoData &msg) = 0;
    virtual void HandleMessage(const InterruptConfigMessage &msg) = 0;
    virtual void HandleMessage(const InterruptEnterData &msg) = 0;
    virtual void HandleMessage(const InterruptExitData &msg) = 0;
    virtual void HandleMessage(const FreertosQueueDeletedMessage &msg) = 0;
    virtual void HandleMessage(const FreertosQueueRegistryMessage &msg) = 0;
    virtual void HandleMessage(const FreertosQueueCreateFailedMessage &msg) = 0;
    virtual void HandleMessage(const FreertosQueueSendFailedMessage &msg) = 0;
    virtual void HandleMessage(const FreertosQueueReceiveFailedMessage &msg) = 0;
    virtual void HandleMessage(const FreertosQueuePeekFailedMessage &msg) = 0;
    virtual void HandleMessage(const FreertosTimerCreateFailedMessage &msg) = 0;
    virtual void HandleMessage(const FreertosTimerCommandReceivedMessage &msg) = 0;
    virtual void HandleMessage(const FreertosStreamBufferCreateFailedMessage &msg) = 0;
    virtual void HandleMessage(const FreertosStreamBufferSendFailedMessage &msg) = 0;
    virtual void HandleMessage(const FreertosStreamBufferReceiveFailedMessage &msg) = 0;
    virtual void HandleMessage(const FreertosEventGroupCreateFailedMessage &msg) = 0;
    virtual void HandleMessage(const FreertosTaskDelayMessage &msg) = 0;
    virtual void HandleMessage(const FreertosTaskDelayUntilMessage &msg) = 0;
    virtual void HandleMessage(const PointerAnnounceMessage &msg) = 0;
    virtual void HandleMessage(const PointerSetNameMessage &msg) = 0;

    virtual void OnCycleCount(uint32_t cycles) = 0;
    virtual void OnConsoleLog(const void *data, size_t length) = 0;
};

class MessageDecoder {
public:
    MessageDecoder(IMessageHandler &handler);

    static MessageDecoder *GetInstance();

    void ProcessChannelData(uint8_t channel, uint64_t timestamp, std::span<std::byte> data);

private:
    void OnMessage(uint8_t messageTypeIndex, std::span<std::byte> data);

    DecoderState state = DecoderState::WAITING_FOR_TYPE;
    uint8_t currentMessageTypeIndex;
    std::vector<uint8_t> messageBuffer;
    std::vector<uint8_t> consoleBuffer;

    IMessageHandler &handler_;
    MessageDispatcher<IMessageHandler> dispatcher_;

    void HandleMessageType(uint8_t type);
    void HandleMessageData(std::span<std::byte> data);
    void HandleConsoleLog(std::span<std::byte> data);
    void TryProcessMessage();
    void Reset();
};