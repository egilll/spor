#include "Transport.hpp"

#include "spor-common/Messages.hpp"
#include "transport/RTT.hpp"

namespace spor {

bool Transport::isReady() {
    // Consider RTT Up-Buffer 0 as always ready once initialized.
    // If desired, check available space: SEGGER_RTT_GetAvailWriteSpace(0) > 0
    return true;
}

}
