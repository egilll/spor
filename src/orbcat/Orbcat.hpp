#pragma once

#include <cstdint>
#include <functional>
#include <span>
#include <string>
#include <memory>

// Forward declarations for C structs from msgDecoder.h
struct excMsg;
struct dwtMsg;
struct watchMsg;
struct wptMsg;
struct oswMsg;
struct nisyncMsg;

namespace orbcat {

/** @deprecated */
enum class TimestampMode {
    NONE,
    HOST_ABSOLUTE,
    HOST_RELATIVE,
    HOST_DELTA,
    TARGET_CYCLES, /* Target-based timestamps require message reordering */
    TARGET_DELTA   /* Target-based timestamps require message reordering */
};

struct ExceptionMessage {
    enum class ExceptionEvent { RESERVED = 0, ENTER = 1, EXIT = 2, RESUME = 3 };
    ExceptionEvent event;
    uint32_t exceptionNumber;
};

/* Time conditions of a TS message - from itmDecoder.h */
enum TimeStatus { TIME_CURRENT = 0, TIME_DELAYED = 1, EVENT_DELAYED = 2, EVENT_AND_TIME_DELAYED = 3 };

class MessageHandler {
public:
    using ExceptionHandler = std::function<void(const ExceptionMessage &, uint64_t timestamp)>;
    using DwtEventHandler = std::function<void(const dwtMsg &, uint64_t timestamp)>;
    using DataWatchpointHandler = std::function<void(const watchMsg &, uint64_t timestamp)>;
    using DataAccessHandler = std::function<void(const wptMsg &, uint64_t timestamp)>;
    using OffsetWriteHandler = std::function<void(const oswMsg &, uint64_t timestamp)>;
    using NiSyncHandler = std::function<void(const nisyncMsg &, uint64_t timestamp)>;
    using TimestampHandler = std::function<void(uint64_t timestamp, TimeStatus status)>;
    using ChannelDataHandler = std::function<void(uint8_t channel, uint64_t timestamp, std::span<std::byte> data)>;

    ExceptionHandler onException;
    DwtEventHandler onDwtEvent;
    DataWatchpointHandler onDataWatchpoint;
    DataAccessHandler onDataAccess;
    OffsetWriteHandler onOffsetWrite;
    NiSyncHandler onNiSync;
    TimestampHandler onTimestamp;
    ChannelDataHandler onChannelData;
};

class Orbcat {
public:
    struct Options {
        std::string channel;
        std::string inputFile = "/Users/egill/spor/data/output.swo";
        bool itmSync = true;
        std::string server = "localhost";
        int port = 3443;

        uint32_t tag = 1;
        bool endTerminate = true;
        bool enableExceptionHandling = false;
        TimestampMode timestampMode = TimestampMode::NONE;

        bool useTPIU = false;
        bool enableInstructionTrace = false;
    };

    Orbcat(const Options &options, MessageHandler handlers);
    ~Orbcat();
    void Start();
    void Stop();

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;

    Orbcat(const Orbcat &) = delete;
    Orbcat &operator=(const Orbcat &) = delete;
    Orbcat(Orbcat &&) = delete;
    Orbcat &operator=(Orbcat &&) = delete;
};

} // namespace orbcat