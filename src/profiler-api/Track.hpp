#pragma once

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <iostream>
#include <memory>
#include <perfetto.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "Packet.hpp"
#include "Slice.hpp"
#include "TimeUtils.hpp"

namespace profiler {

uint64_t GenerateUniqueUuid();

enum class TrackType {
    ROOT,
    THREAD_GROUP,
    THREAD_STATUS,
    CALL_STACK,
    LOCK_CONTENTION,
    DMA_TRANSFER,
    IRQ_HANDLER,
    CUSTOM
};

struct TrackNode : std::enable_shared_from_this<TrackNode> {
    TrackType type;
    std::string name;
    std::weak_ptr<TrackNode> parent;
    std::vector<std::shared_ptr<TrackNode>> children;
    std::vector<std::shared_ptr<Slice>> activeSlices;

    uint64_t id = GenerateUniqueUuid();

    TrackNode(TrackType type, const std::string &name, std::shared_ptr<TrackNode> parent = nullptr)
        : type(type), name(name), parent(parent) {
        SetupTrackDescriptor();
    }

    ~TrackNode() {}

    void AddChild(std::shared_ptr<TrackNode> child) {
        children.push_back(child);
        child->parent = shared_from_this();
    }

    std::shared_ptr<Slice> StartSlice(const std::string &name, uint64_t timestamp = GetTime()) {
        auto slice = std::make_shared<Slice>(id, timestamp, name);
        activeSlices.push_back(slice);
        RemoveInactiveSlices();

        return slice;
    }

    void Message(std::string_view name, uint64_t timestamp = GetTime()) {
        CreateTrackEvent(id, timestamp, perfetto::protos::pbzero::TrackEvent::TYPE_INSTANT, name);
    }

private:
    void SetupTrackDescriptor() {
        auto packet = CreatePacket();
        auto *trackDesc = packet->set_track_descriptor();
        trackDesc->set_uuid(id);
        trackDesc->set_name(name);

        std::cout << ">> TRACK CREATED, ID " << id;
        if (auto parentNode = parent.lock()) {
            trackDesc->set_parent_uuid(parentNode->id);
            std::cout << " PARENT  " << parentNode->id;
        }
        std::cout << std::endl;
    }

    void RemoveInactiveSlices() {
        activeSlices.erase(
            std::remove_if(
                activeSlices.begin(), activeSlices.end(),
                [](const std::shared_ptr<Slice> &slice) {
                    return !slice || !slice->active;
                }
            ),
            activeSlices.end()
        );
    }
};

struct TrackManager {
    std::shared_ptr<TrackNode> root;
    std::unordered_map<uint64_t, std::weak_ptr<TrackNode>> trackLookup;
    bool initialized = false;

    static TrackManager &Instance() {
        static TrackManager instance;
        return instance;
    }

    void Initialize() {
        if (!initialized) {
            root = std::make_shared<TrackNode>(TrackType::ROOT, "Root");
            trackLookup[root->id] = root;
            initialized = true;
        }
    }

    std::shared_ptr<TrackNode>
    CreateTrack(TrackType type, const std::string &name, std::shared_ptr<TrackNode> parent = nullptr) {
        Initialize();
        if (!parent) {
            parent = root;
        }

        auto track = std::make_shared<TrackNode>(type, name, parent);

        if (parent) {
            parent->AddChild(track);
        }

        trackLookup[track->id] = track;

        return track;
    }

private:
    TrackManager() = default;
    ~TrackManager() = default;
};

}