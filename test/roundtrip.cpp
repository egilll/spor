#include <cassert>
#include <cstdint>
#include <iostream>
#include <span>
#include <string>
#include <vector>

#include "src/spor-common/Messages.hpp"
#include "src/spor-host/Decoder.hpp"

// Provide a stub to satisfy Decoder.cpp's reference when linking this test.
MessageDecoder *GetSporInstance() { return nullptr; }

struct TestHandler : public IMessageHandler {
    bool gotPlot = false;
    bool gotMsg = false;
    std::string plotName;
    int64_t plotValue = 0;
    std::string textMsg;
    uint32_t lastCycles = 0;

    void HandleMessage(const ZoneBeginData &) override {}
    void HandleMessage(const ZoneEndData &) override {}
    void HandleMessage(const ZoneTextMessage &) override {}
    void HandleMessage(const ZoneValueData &) override {}
    void HandleMessage(const ZoneColorData &) override {}
    void HandleMessage(const PlotMessage &msg) override {
        gotPlot = true;
        plotName = msg.name;
        plotValue = msg.data.value;
    }
    void HandleMessage(const PlotConfigMessage &) override {}
    void HandleMessage(const MessageTextMessage &msg) override {
        gotMsg = true;
        textMsg = msg.text;
    }
    void HandleMessage(const AllocMessage &) override {}
    void HandleMessage(const FreeMessage &) override {}
    void HandleMessage(const FreertosTaskCreatedMessage &) override {}
    void HandleMessage(const FreertosTaskSwitchedInMessage &) override {}
    void HandleMessage(const FreertosTaskSwitchedOutMessage &) override {}
    void HandleMessage(const FreertosTaskReadiedMessage &) override {}
    void HandleMessage(const FreertosQueueCreatedMessage &) override {}
    void HandleMessage(const FreertosQueuePeekMessage &) override {}
    void HandleMessage(const FreertosQueueSendMessage &) override {}
    void HandleMessage(const FreertosQueueReceiveMessage &) override {}
    void HandleMessage(const FreertosTaskNotifyMessage &) override {}
    void HandleMessage(const FreertosTaskNotifyReceivedMessage &) override {}
    void HandleMessage(const FreertosTaskDeletedMessage &) override {}
    void HandleMessage(const FreertosTaskPrioritySetMessage &) override {}
    void HandleMessage(const FreertosTaskSuspendMessage &) override {}
    void HandleMessage(const FreertosTaskResumeMessage &) override {}
    void HandleMessage(const FreertosTimerCreatedMessage &) override {}
    void HandleMessage(const FreertosTimerCommandMessage &) override {}
    void HandleMessage(const FreertosTimerExpiredMessage &) override {}
    void HandleMessage(const FreertosEventGroupCreatedMessage &) override {}
    void HandleMessage(const FreertosEventGroupDeletedMessage &) override {}
    void HandleMessage(const FreertosEventGroupSyncMessage &) override {}
    void HandleMessage(const FreertosEventGroupWaitBitsMessage &) override {}
    void HandleMessage(const FreertosEventGroupClearBitsMessage &) override {}
    void HandleMessage(const FreertosEventGroupSetBitsMessage &) override {}
    void HandleMessage(const FreertosStreamBufferCreatedMessage &) override {}
    void HandleMessage(const FreertosStreamBufferDeletedMessage &) override {}
    void HandleMessage(const FreertosStreamBufferSendMessage &) override {}
    void HandleMessage(const FreertosStreamBufferReceiveMessage &) override {}
    void HandleMessage(const FreertosStreamBufferResetMessage &) override {}
    void HandleMessage(const FreertosIsrEnterMessage &) override {}
    void HandleMessage(const FreertosIsrExitMessage &) override {}
    void HandleMessage(const FreertosIsrExitToSchedulerMessage &) override {}
    void HandleMessage(const FunctionTraceEnterData &) override {}
    void HandleMessage(const FunctionTraceExitData &) override {}
    void HandleMessage(const DeclarePointerTypeMessage &) override {}
    void HandleMessage(const FlowBeginMessage &) override {}
    void HandleMessage(const FlowEndMessage &) override {}
    void HandleMessage(const SystemInfoData &) override {}
    void HandleMessage(const InterruptConfigMessage &) override {}
    void HandleMessage(const InterruptEnterData &) override {}
    void HandleMessage(const InterruptExitData &) override {}
    void HandleMessage(const FreertosQueueDeletedMessage &) override {}
    void HandleMessage(const FreertosQueueRegistryMessage &) override {}
    void HandleMessage(const FreertosQueueCreateFailedMessage &) override {}
    void HandleMessage(const FreertosQueueSendFailedMessage &) override {}
    void HandleMessage(const FreertosQueueReceiveFailedMessage &) override {}
    void HandleMessage(const FreertosQueuePeekFailedMessage &) override {}
    void HandleMessage(const FreertosTimerCreateFailedMessage &) override {}
    void HandleMessage(const FreertosTimerCommandReceivedMessage &) override {}
    void HandleMessage(const FreertosStreamBufferCreateFailedMessage &) override {}
    void HandleMessage(const FreertosStreamBufferSendFailedMessage &) override {}
    void HandleMessage(const FreertosStreamBufferReceiveFailedMessage &) override {}
    void HandleMessage(const FreertosEventGroupCreateFailedMessage &) override {}
    void HandleMessage(const FreertosTaskDelayMessage &) override {}
    void HandleMessage(const FreertosTaskDelayUntilMessage &) override {}
    void HandleMessage(const PointerAnnounceMessage &) override {}
    void HandleMessage(const PointerSetNameMessage &) override {}

    void OnCycleCount(uint32_t cycles) override { lastCycles = cycles; }
    void OnConsoleLog(const void *, size_t) override {}
};

struct MemWriter {
    std::vector<uint8_t> &buf;
    void put_u8(uint8_t v) { buf.push_back(v); }
    void put_u16(uint16_t v) {
        put_u8(uint8_t(v & 0xFF));
        put_u8(uint8_t((v >> 8) & 0xFF));
    }
    void put_u32(uint32_t v) {
        put_u8(uint8_t(v & 0xFF));
        put_u8(uint8_t((v >> 8) & 0xFF));
        put_u8(uint8_t((v >> 16) & 0xFF));
        put_u8(uint8_t((v >> 24) & 0xFF));
    }
    void put_u64(uint64_t v) {
        put_u32(uint32_t(v & 0xFFFFFFFFULL));
        put_u32(uint32_t((v >> 32) & 0xFFFFFFFFULL));
    }
    void put_s32(int32_t v) { put_u32(uint32_t(v)); }
    void put_s64(int64_t v) { put_u64(uint64_t(v)); }
    void put_stringz(const char *s) {
        if (!s) {
            put_u8(0);
            return;
        }
        while (*s) put_u8(uint8_t(*s++));
        put_u8(0);
    }
};

template <typename T>
static void emit(std::vector<uint8_t> &out, const T &msg) {
    MemWriter w{out};
    constexpr uint8_t type = static_cast<uint8_t>(MessageTag<T>::value);
    w.put_u8(type);
    w.put_u32(0); // cycles
    spor::wire::write(w, msg);
}

int main() {
    // Build a small stream: Plot("alpha", 42), MessageText("hello", color=0)
    std::vector<uint8_t> bytes;
    emit(bytes, PlotMessage{PlotData{42}, std::string("alpha").c_str()});
    emit(bytes, MessageTextMessage{MessageData{0, 5}, std::string("hello").c_str()});

    TestHandler th;
    MessageDecoder dec(th);
    std::span<const std::byte> data(reinterpret_cast<const std::byte *>(bytes.data()), bytes.size());
    dec.ProcessBytes(data);

    if (!th.gotPlot || !th.gotMsg) {
        std::cerr << "Did not decode expected messages\n";
        return 1;
    }
    if (th.plotName != "alpha" || th.plotValue != 42) {
        std::cerr << "Plot mismatch: name=" << th.plotName << " value=" << th.plotValue << "\n";
        return 2;
    }
    if (th.textMsg != "hello") {
        std::cerr << "Text mismatch: got '" << th.textMsg << "'\n";
        return 3;
    }
    std::cout << "Roundtrip OK\n";
    return 0;
}
