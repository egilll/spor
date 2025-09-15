#pragma once

#include SPOR_SYSTEM_HEADER
#include "utils.hpp"

namespace spor {

class IrqLockGuard {
    uint32_t oldIntStat;

public:
    NO_INSTRUMENT IrqLockGuard() {
        oldIntStat = __get_PRIMASK();
        __disable_irq();
    }

    NO_INSTRUMENT ~IrqLockGuard() {
        __set_PRIMASK(oldIntStat);
    }

    IrqLockGuard(const IrqLockGuard &) = delete;
    IrqLockGuard &operator=(const IrqLockGuard &) = delete;
    IrqLockGuard(IrqLockGuard &&) = delete;
    IrqLockGuard &operator=(IrqLockGuard &&) = delete;
};

}