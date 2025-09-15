#pragma once

namespace spor {
class ScopedZone {
public:
    ScopedZone(const ScopedZone &) = delete;
    ScopedZone(ScopedZone &&) = delete;
    ScopedZone &operator=(const ScopedZone &) = delete;
    ScopedZone &operator=(ScopedZone &&) = delete;

    const char *ptr_;
    ScopedZone(const char *name) {
        TraceZoneBegin(name);
        ptr_ = name;
    }

    ~ScopedZone() {
        TraceZoneEnd(ptr_);
    }
};

class ScopedInterrupt {
public:
    ScopedInterrupt(const ScopedInterrupt &) = delete;
    ScopedInterrupt(ScopedInterrupt &&) = delete;
    ScopedInterrupt &operator=(const ScopedInterrupt &) = delete;
    ScopedInterrupt &operator=(ScopedInterrupt &&) = delete;

    ScopedInterrupt(uint16_t irq_number, uint32_t enabled_flags) : m_irq_number(irq_number) {
        TraceInterruptEnter(irq_number, enabled_flags);
    }

    ~ScopedInterrupt() {
        TraceInterruptExit(m_irq_number);
    }

private:
    uint16_t m_irq_number;
};
}