#ifdef SPOR_RETARGET_WRITE

#include "spor-common/Messages.hpp"
#include SPOR_SYSTEM_HEADER
#include "orbcode/trace/itm.h"
#include "transport/Transport.hpp"

extern "C" int _write(int fd, char *ptr, int len) {
    spor::SendCycleCount();
    ITMWriteBuffer(static_cast<uint8_t>(Channel::CONSOLE_LOG), ptr, len);
    return len;
}

#endif
