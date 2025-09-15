#pragma once

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
#include <typeinfo>
extern "C" {
#endif
void TraceZoneBegin(const char *name);
void TraceZoneEnd(const char *name);
void TraceMessage(const char *text, size_t length, uint32_t color);
void TraceAlloc(const void *ptr, size_t size, const char *name);
void TraceFree(const void *ptr, const char *name);
void TraceAnnounceName(const void *ptr, const char *name);
void TraceAnnouncePointer(const void *ptr, const char *name);
void TraceInterruptEnter(uint16_t irq_number, uint32_t enabled_flags);
void TraceInterruptExit(uint16_t irq_number);
void TraceFlowBegin(const void *ptr);
void TraceFlowEnd(const void *ptr);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

void TraceDeclareType(const void *ptr, const std::type_info *typeInfo);

template <typename T>
void TraceDeclareType(const T *ptr) {
    TraceDeclareType(ptr, &typeid(ptr));
}

#define ZoneScoped const spor::ScopedZone ___spor_scoped_zone(__FUNCTION__)
#define ZoneScopedN(name) const spor::ScopedZone ___spor_scoped_zone(name)
#define TraceInterrupt(irq_number, enabled_flags)                                                                      \
    const spor::ScopedInterrupt ___spor_scoped_interrupt(irq_number, enabled_flags)
#include "SporScoped.hpp"

#endif
