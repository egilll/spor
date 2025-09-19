#include "State.hpp"

#include <iostream>
#include <string>
#include <unordered_map>

#include "symbol-resolver/ElfSymbolResolver.hpp"

void State::SymbolsLoaded() {
    auto resolver = static_cast<spor::ElfSymbolResolver *>(profiler::GetSymbolResolver());

    for (const auto &[functionPtr, symbolInfo] : resolver->symbols) {
        for (const auto &[irqNumber, irqName] : deviceInfo.irq_table) {
            if (symbolInfo.mangledName == irqName) {
                irqFunctions[functionPtr] = {irqNumber, irqName};
                break;
            }
        }
    }
}

void State::FunctionEnter(TargetPointer ptr) {
    auto irqIt = irqFunctions.find(ptr);
    if (irqIt != irqFunctions.end()) {
        // Use IRQ number for IRQ thread registration and tracking
        // profiler::PerfettoApi::EnterIrq(static_cast<uint32_t>(irqIt->second.irqNumber));
    } else {
        profiler::PerfettoApi::FunctionTraceEnter(ptr);
    }
}

void State::FunctionExit(TargetPointer ptr) {
    auto irqIt = irqFunctions.find(ptr);
    if (irqIt != irqFunctions.end()) {
        // profiler::PerfettoApi::ExitIrq(static_cast<uint32_t>(irqIt->second.irqNumber));
    } else {
        profiler::PerfettoApi::FunctionTraceExit(ptr);
    }
}

void State::HandleException(const orbcat::ExceptionMessage &exception, uint64_t timestamp) {
    return; // TEMP
    DeviceInfo::IrqNumber irqNumber = static_cast<DeviceInfo::IrqNumber>(exception.exceptionNumber);

    if (exception.exceptionNumber == 0 && exception.event == orbcat::ExceptionMessage::ExceptionEvent::RESUME) {
        profiler::PerfettoApi::ExitAllIrqs();
        return;
    }

    // Find the IRQ name from the device info table
    auto irqIt = deviceInfo.irq_table.find(irqNumber);
    if (irqIt != deviceInfo.irq_table.end()) {
        std::string irqName = std::string(irqIt->second);
        TargetPointer irqPtr = static_cast<TargetPointer>(irqNumber);

        switch (exception.event) {
        case orbcat::ExceptionMessage::ExceptionEvent::ENTER:
            std::cout << "     ENTER " << irqName << std::endl;

            profiler::PerfettoApi::EnsureThreadRegistered(irqPtr, irqName.c_str());
            profiler::PerfettoApi::EnterIrq(irqPtr);
            break;
        case orbcat::ExceptionMessage::ExceptionEvent::EXIT:
            std::cout << "     EXIT " << irqName << std::endl;

            profiler::PerfettoApi::ExitIrq(irqPtr);
            break;
        case orbcat::ExceptionMessage::ExceptionEvent::RESUME:
            std::cout << "     RESUME " << irqName << std::endl;
            // Todo?
            break;
        default:
            // ??
            break;
        }
    } else {
        std::cout << "Unknown IRQ number " << irqNumber << ", event: " << (int)exception.event << std::endl;
    }
}
