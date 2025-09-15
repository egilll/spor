#include "Track.hpp"
namespace profiler {

uint64_t GenerateUniqueUuid() {
    static std::atomic<uint64_t> counter{1};
    return counter.fetch_add(1);
}
}