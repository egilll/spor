#pragma once

#include <cstdint>
#include <perfetto.h>
#include <string_view>

#include "TimeUtils.hpp"

namespace profiler {

class ThreadDataSource : public perfetto::DataSource<ThreadDataSource> {
public:
    void OnSetup(const SetupArgs &) override {}
    void OnStart(const StartArgs &) override {}
    void OnStop(const StopArgs &) override {}
};

inline auto CreatePacket(uint64_t timestamp = GetTime())
    -> protozero::MessageHandle<perfetto::protos::pbzero::TracePacket> {
    protozero::MessageHandle<perfetto::protos::pbzero::TracePacket> packet;
    ThreadDataSource::Trace([&packet, timestamp](ThreadDataSource::TraceContext ctx) {
        packet = ctx.NewTracePacket();
        packet->set_timestamp(timestamp);
    });
    if (!packet) {
        throw std::runtime_error("Failed to create packet");
    }
    return packet;
}

inline void CreateTrackEvent(
    uint64_t trackUuid,
    uint64_t timestamp,
    perfetto::protos::pbzero::TrackEvent::Type eventType,
    std::string_view name = {}
) {
    auto packet = CreatePacket(timestamp);
    // packet->set_trusted_packet_sequence_id(1);
    auto *event = packet->set_track_event();
    event->set_type(eventType);
    event->set_track_uuid(trackUuid);
    if (!name.empty()) {
        event->set_name(std::string(name));
    }

    // ThreadDataSource::Trace([](ThreadDataSource::TraceContext ctx) {
    //     ctx.Flush();
    // });
}

}