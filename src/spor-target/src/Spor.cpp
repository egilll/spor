#include "Spor.h"

#include <cstring>

#include "spor-common/Messages.hpp"
#include "transport/Transport.hpp"

using namespace spor;

void TraceZoneBegin(const char *name) {
    Send(
        ZoneBeginData{
            .ptr = (uint32_t)name,
        }
    );
}

void TraceZoneEnd(const char *name) {
    Send(
        ZoneEndData{
            .ptr = (uint32_t)name,
        }
    );
}

void Plot(const char *name, int64_t value) {
    if (!name)
        return;
    Send(PlotMessage{{value}, name});
}

void PlotConfig(const char *name, int type, bool step, bool fill, uint32_t color) {
    if (!name)
        return;
    Send(PlotConfigMessage{{type, step ? uint8_t(1) : uint8_t(0), fill ? uint8_t(1) : uint8_t(0), color}, name});
}

void TraceMessage(const char *text, size_t length, uint32_t color) {
    if (!text)
        return;
    if (length == 0) {
        length = strlen(text);
    }
    Send(MessageTextMessage{{color, static_cast<uint32_t>(length)}, text});
}

void TraceAlloc(const void *ptr, size_t size, const char *name) {
    if (!ptr)
        return;
    Send(AllocMessage{{static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ptr)), static_cast<uint32_t>(size)}, name});
}

void TraceFree(const void *ptr, const char *name) {
    if (!ptr)
        return;
    Send(FreeMessage{{static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ptr))}, name});
}

// void LockableCreate(uint32_t id, const char *name, const char *type) {
//     Send(LockableCreateMessage{{id, 0, 0}, name, type});
// }
//
// void LockableWait(uint32_t id) {
//     Send(LockableWaitData{0, id});
// }
//
// void LockableObtain(uint32_t id) {
//     Send(LockableObtainData{0, id});
// }
//
// void LockableRelease(uint32_t id) {
//     Send(LockableReleaseData{id});
// }

void SetProgramName(const char *name) {
    if (!name)
        return;
    TraceMessage(name, strlen(name), 0);
}

// void ThreadContextSwitch(uint32_t old_thread_id, uint32_t new_thread_id, int64_t time) {
//     // if (!Transport::isReady())
//     //     return;
//     //
//     // if (time == 0) {
//     //     time = GetCurrentTime();
//     // }
//     //
//     // Send(ThreadContextSwitchMessage{old_thread_id, new_thread_id});
// }
//
// void ThreadWakeup(uint32_t thread_id, int64_t time) {
//     // if (!Transport::isReady())
//     //     return;
//     //
//     // if (time == 0) {
//     //     time = GetCurrentTime();
//     // }
//     //
//     // Send(ThreadWakeupData{thread_id});
// }

// void SendSystemInfo(uint32_t clock_frequency_mhz) {
//     Send(SystemInfoData{clock_frequency_mhz});
// }

// void ConfigureInterrupt(uint32_t irq_number, const char *irq_name, const char **flag_names, uint32_t flag_count) {
//     Send(InterruptConfigMessage{{irq_number, flag_count}, irq_name, flag_names});
// }

void TraceInterruptEnter(uint16_t irq_number, uint32_t enabled_flags) {
    Send(InterruptEnterData{irq_number, enabled_flags});
}

void TraceInterruptExit(uint16_t irq_number) {
    Send(InterruptExitData{irq_number});
}

void TraceFlowBegin(const void *ptr) {
    Send(FlowBeginMessage{static_cast<TargetPointer>(reinterpret_cast<uintptr_t>(ptr))});
}

void TraceFlowEnd(const void *ptr) {
    Send(FlowEndMessage{static_cast<TargetPointer>(reinterpret_cast<uintptr_t>(ptr))});
}

#ifdef __cplusplus
void TraceDeclareType(const void *ptr, const std::type_info *typeInfo) {
    Send(
        DeclarePointerTypeMessage{
            static_cast<TargetPointer>(reinterpret_cast<uintptr_t>(ptr)),
            static_cast<TargetPointer>(reinterpret_cast<uintptr_t>(typeInfo))
        }
    );
}
#endif

void TraceAnnounceName(const void *ptr, const char *name) {
    Send(PointerSetNameMessage{.ptr = (uint32_t)ptr, .name = name});
}

void TraceAnnouncePointer(const void *ptr, const char *name) {
    if (ptr == nullptr)
        return;

    if (name == nullptr) {
        Send(PointerAnnounceMessage{(uint32_t)ptr, (uint32_t)&ptr});
    } else {
        TraceAnnounceName(&ptr, name);
    }
}
