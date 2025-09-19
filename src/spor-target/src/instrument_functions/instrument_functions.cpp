#include SPOR_SYSTEM_HEADER
#include "spor-common/Messages.hpp"
#include "transport/Transport.hpp"
#include "Utils.hpp"

using namespace spor;

extern "C" __attribute__((no_instrument_function, noinline)) void
__cyg_profile_func_enter(void *this_fn, void *call_site) {
    if (isInInterrupt())
        return;
    Send(FunctionTraceEnterData{static_cast<uint32_t>(reinterpret_cast<uintptr_t>(this_fn))});
}

extern "C" __attribute__((no_instrument_function, noinline)) void
__cyg_profile_func_exit(void *this_fn, void *call_site) {
    if (isInInterrupt())
        return;
    Send(FunctionTraceExitData{static_cast<uint32_t>(reinterpret_cast<uintptr_t>(this_fn))});
}
