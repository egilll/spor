#include SPOR_SYSTEM_HEADER
#include "spor-common/Messages.hpp"
#include "transport/Transport.hpp"
#include "Utils.hpp"

using namespace spor;

// TODO
void ITMWrite32_assume_enabled(uint8_t port, uint32_t value) {
    if (!ITMIsPortEnabled(port)) {
        return;
    }

    while (ITM->PORT[port].u32 == 0UL) {
        __NOP();
    }
    ITM->PORT[port].u32 = value;
}

extern "C" __attribute__((no_instrument_function, noinline)) void
__cyg_profile_func_enter(void *this_fn, void *call_site) {
    if (isInInterrupt())
        return;
    SendCycleCount();
    ITMWrite32_assume_enabled(static_cast<uint8_t>(Channel::FUNCTION_ENTER), reinterpret_cast<uint32_t>(this_fn));
}

extern "C" __attribute__((no_instrument_function, noinline)) void
__cyg_profile_func_exit(void *this_fn, void *call_site) {
    if (isInInterrupt())
        return;
    SendCycleCount();
    ITMWrite32_assume_enabled(static_cast<uint8_t>(Channel::FUNCTION_EXIT), reinterpret_cast<uint32_t>(this_fn));
}