#pragma once

#include <cstdint>
#include <perfetto.h>
#include <string>

#include "Packet.hpp"
#include "TimeUtils.hpp"

namespace profiler {

struct Slice {
    uint64_t trackId;
    uint64_t startTime;
    std::string name;
    bool active = true;

    Slice(const uint64_t trackId, uint64_t startTime, const std::string &name)
        : trackId(trackId), startTime(startTime), name(name) {
        CreateTrackEvent(trackId, startTime, perfetto::protos::pbzero::TrackEvent::TYPE_SLICE_BEGIN, name);
    }

    ~Slice() {
        if (active) {
            End();
        }
    }

    void End(uint64_t timestamp = GetTime()) {
        if (active) {
            CreateTrackEvent(trackId, timestamp, perfetto::protos::pbzero::TrackEvent::TYPE_SLICE_END);
            active = false;
        }
    }
};

}