#pragma once

#include SPOR_SYSTEM_HEADER

#define NO_INSTRUMENT __attribute__((no_instrument_function))

inline bool isInInterrupt() {
    return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0;
}
