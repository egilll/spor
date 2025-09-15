#include "Orbcat.hpp"

#include <atomic>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>
#include <unistd.h>
#include <utility>

#include "generics.h"
#include "itmDecoder.h"
#include "msgDecoder.h"
#include "msgSeq.h"
#include "nw.h"
#include "stream.h"

namespace orbcat {

class Orbcat::Impl {
public:
    Impl(Options options, MessageHandler handlers) : options_(std::move(options)), handlers_(std::move(handlers)) {}

    void Start();
    void Stop() {
        running_ = false;
    }

private:
    Options options_;
    MessageHandler handlers_;
    std::atomic<bool> running_{false};

    struct {
        ITMDecoder itmDecoder{};
        MSGSeq msgSequencer{};

        uint64_t currentTimestamp = 0;
        uint64_t lastTimestamp = 0;
        bool timestampInitialized = false;
        timeDelay timeStatus = ::TIME_CURRENT;
    } decoders_;

    void initializeDecoders();
    std::unique_ptr<Stream, void (*)(Stream *)> tryOpenStream();
    void handleTimestamp(const genericMsg &msg);
    void dispatchMessage(const msg &message);
    void processByte(uint8_t byte);
    void processData(uint8_t *data, size_t size);
    void feedStream(Stream *stream);
};

Orbcat::Orbcat(const Orbcat::Options &options, MessageHandler handlers)
    : pImpl(std::make_unique<Impl>(options, std::move(handlers))) {}

Orbcat::~Orbcat() = default;

void Orbcat::Start() {
    pImpl->Start();
}

void Orbcat::Stop() {
    pImpl->Stop();
}

void Orbcat::Impl::initializeDecoders() {
    // ITMDecoderInit(&decoders_.itmDecoder, options_.itmSync);
    ITMDecoderInit(&decoders_.itmDecoder, true);
    MSGSeqInit(&decoders_.msgSequencer, &decoders_.itmDecoder, 30);
}

std::unique_ptr<Stream, void (*)(Stream *)> Orbcat::Impl::tryOpenStream() {
    Stream *stream = nullptr;

    if (!options_.inputFile.empty()) {
        stream = streamCreateFile(options_.inputFile.c_str());
    } else {
        stream = streamCreateSocket(options_.server.c_str(), options_.port);
    }

    return {stream, [](Stream *s) {
                if (s) {
                    s->close(s);
                    free(s);
                }
            }};
}

void Orbcat::Impl::feedStream(Stream *stream) {
    static constexpr size_t BUFFER_SIZE = 2048;
    uint8_t buffer[BUFFER_SIZE];

    while (running_) {
        timeval timeout{0, 100000};
        size_t receivedSize = 0;

        auto result = stream->receive(stream, buffer, BUFFER_SIZE, &timeout, &receivedSize);

        if (result != RECEIVE_RESULT_OK) {
            if (result == RECEIVE_RESULT_EOF && options_.endTerminate) {
                return;
            } else if (result == RECEIVE_RESULT_ERROR) {
                break;
            }
        }

        if (receivedSize > 0) {
            processData(buffer, receivedSize);
        }
    }
}

void Orbcat::Impl::processData(uint8_t *data, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        processByte(data[i]);
    }
}

void Orbcat::Impl::processByte(uint8_t byte) {
    msg message;

    /** For any mode except the ones where we collect timestamps from the
    target we need to send */
    /* the samples out directly to give the host a chance of having accurate
     * timing info. For   */
    /* target-based timestamps we need to re-sequence the messages so that
    the
     * timestamps are   */
    /* issued _before_ the data they apply to.  These are the two cases. */
    if (options_.timestampMode != TimestampMode::TARGET_CYCLES &&
        options_.timestampMode != TimestampMode::TARGET_DELTA) {
        if (ITM_EV_PACKET_RXED == ITMPump(&decoders_.itmDecoder, byte)) {
            if (ITMGetDecodedPacket(&decoders_.itmDecoder, &message)) {
                dispatchMessage(message);
            }
        }
    } else {
        /* Pump messages into the store until we get a time message, then we can
         * read them out */
        if (MSGSeqPump(&decoders_.msgSequencer, byte)) {
            msg *pendingMsg;
            /* We are synced timewise, so empty anything that has been waiting
             */
            while ((pendingMsg = MSGSeqGetPacket(&decoders_.msgSequencer))) {
                dispatchMessage(*pendingMsg);
            }
        }
    }
}

void Orbcat::Impl::dispatchMessage(const msg &message) {
    switch (message.genericMsg.msgtype) {
    case MSG_SOFTWARE: {
        // profiler::TracyApi::SetTime(message.swMsg.ts);
        if (handlers_.onChannelData) {
            auto bytes = std::as_bytes(std::span(&message.swMsg.value, 1));
            handlers_.onChannelData(
                message.swMsg.srcAddr, message.swMsg.ts,
                std::span(const_cast<std::byte *>(bytes.data()), message.swMsg.len)
            );
        }
        break;
    }

    case MSG_TS:
        handleTimestamp(message.genericMsg);
        break;

    case MSG_EXCEPTION:
        if (handlers_.onException) {
            // profiler::TracyApi::SetTime(decoders_.currentTimestamp);

            ExceptionMessage exMsg{
                .event = static_cast<ExceptionMessage::ExceptionEvent>(message.excMsg.eventType & 0x03),
                .exceptionNumber = message.excMsg.exceptionNumber,
            };
            handlers_.onException(exMsg, message.excMsg.ts);
        }
        break;

    case MSG_DWT_EVENT:
        if (handlers_.onDwtEvent) {
            handlers_.onDwtEvent(message.dwtMsg, message.dwtMsg.ts);
        }
        break;

    case MSG_DATA_RWWP:
        if (handlers_.onDataWatchpoint) {
            handlers_.onDataWatchpoint(message.watchMsg, message.watchMsg.ts);
        }
        break;

    case MSG_DATA_ACCESS_WP:
        if (handlers_.onDataAccess) {
            handlers_.onDataAccess(message.wptMsg, message.wptMsg.ts);
        }
        break;

    case MSG_OSW:
        if (handlers_.onOffsetWrite) {
            handlers_.onOffsetWrite(message.oswMsg, message.oswMsg.ts);
        }
        break;

    case MSG_NISYNC:
        if (handlers_.onNiSync) {
            handlers_.onNiSync(message.nisyncMsg, message.nisyncMsg.ts);
        }
        break;

    default:
        break;
    }
}

void Orbcat::Impl::handleTimestamp(const genericMsg &msg) {
    auto *tsMsg = reinterpret_cast<const TSMsg *>(&msg);
    decoders_.currentTimestamp += tsMsg->timeInc;
    // decoders_.timeStatus = static_cast<enum timeDelay>(tsMsg->timeStatus);

    if (!decoders_.timestampInitialized) {
        decoders_.lastTimestamp = decoders_.currentTimestamp;
        decoders_.timestampInitialized = true;
    }

    if (handlers_.onTimestamp) {
        handlers_.onTimestamp(decoders_.currentTimestamp, TimeStatus::TIME_CURRENT /* TODO */);
    }
}

void Orbcat::Impl::Start() {
    // genericsSetReportLevel(V_DEBUG);
    running_ = true;

    initializeDecoders();

    while (running_) {
        auto stream = tryOpenStream();
        if (!stream) {
            if (options_.endTerminate) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        feedStream(stream.get());

        if (options_.endTerminate) {
            break;
        }
    }
}

} // namespace orbcat
