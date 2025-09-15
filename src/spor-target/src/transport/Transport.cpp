#include "Transport.hpp"

#include "spor-common/Messages.hpp"

namespace spor {

bool Transport::isReady() {
    return ITMIsPortEnabled(0);
}

void NO_INSTRUMENT SendChannel(Channel channel, std::span<const std::byte> data) {
    if (data.empty())
        return;
    ITMWriteBuffer(static_cast<uint8_t>(channel), data.data(), data.size());
}

}
