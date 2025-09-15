#include "Decoder.hpp"

#include <cassert>
#include <cstring>
#include <iostream>

MessageDecoder::MessageDecoder(IMessageHandler &handler) : handler_(handler), dispatcher_(handler_) {}

MessageDecoder *MessageDecoder::GetInstance() {
    extern MessageDecoder *GetSporInstance();
    return GetSporInstance();
}

void MessageDecoder::ProcessChannelData(uint8_t channel, uint64_t timestamp, std::span<std::byte> data) {
    Channel ch = static_cast<Channel>(channel);

    switch (ch) {
    case Channel::MESSAGE_TYPE:
        HandleMessageType(static_cast<uint8_t>(data[0]));
        break;

    case Channel::MESSAGE_DATA:
        if (state == DecoderState::RECEIVING_DATA) {
            HandleMessageData(data);
        }
        break;

    case Channel::CYCLE_COUNT:
        if (data.size() == 4) {
            uint32_t cycles;
            std::memcpy(&cycles, data.data(), sizeof(cycles));
            handler_.OnCycleCount(cycles);
        }
        break;

    case Channel::CONSOLE_LOG:
        HandleConsoleLog(data);
        break;

    default:
        break;
    }
}

void MessageDecoder::OnMessage(uint8_t messageTypeIndex, std::span<std::byte> data) {
    dispatcher_.DispatchMessage(messageTypeIndex, data);
}

void MessageDecoder::HandleMessageType(uint8_t type) {
    if (state == DecoderState::RECEIVING_DATA && !messageBuffer.empty()) {
        TryProcessMessage();
    }
    currentMessageTypeIndex = type;
    state = DecoderState::RECEIVING_DATA;
    messageBuffer.clear();
}

void MessageDecoder::HandleMessageData(std::span<std::byte> data) {
    auto bytes = reinterpret_cast<const uint8_t *>(data.data());
    messageBuffer.insert(messageBuffer.end(), bytes, bytes + data.size());

    TryProcessMessage();
}

void MessageDecoder::TryProcessMessage() {
    if (messageBuffer.empty()) {
        return;
    }

    std::span<std::byte> messageSpan{reinterpret_cast<std::byte *>(messageBuffer.data()), messageBuffer.size()};

    if (dispatcher_.CanDecodeMessage(currentMessageTypeIndex, messageSpan)) {
        OnMessage(currentMessageTypeIndex, messageSpan);
        Reset();
    }
}

void MessageDecoder::Reset() {
    state = DecoderState::WAITING_FOR_TYPE;
    messageBuffer.clear();
}

void MessageDecoder::HandleConsoleLog(std::span<std::byte> data) {
    for (const auto &byteVal : data) {
        uint8_t byte = static_cast<uint8_t>(byteVal);
        if (byte == 0 || byte == '\n') {
            if (!consoleBuffer.empty()) {
                handler_.OnConsoleLog(consoleBuffer.data(), consoleBuffer.size());
                consoleBuffer.clear();
            }
        } else {
            consoleBuffer.push_back(byte);
        }
    }
}