#ifdef SPOR_RETARGET_WRITE

#include "spor-common/Messages.hpp"
#include SPOR_SYSTEM_HEADER
#include "transport/Transport.hpp"

extern "C" int _write(int fd, char *ptr, int len) {
    if (ptr == nullptr || len <= 0) return 0;
    // Send raw bytes followed by a zero terminator
    spor::Send(MessageTextMessage{ptr, static_cast<uint32_t>(len)});
    return len;
}

#endif
