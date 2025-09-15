#pragma once
#include <string_view>
#include <unordered_map>
#include <cstdint>

struct DeviceInfo {
    using IrqNumber = int16_t;
    using IrqTable = std::unordered_map<IrqNumber, std::string_view>;
    IrqTable irq_table;
};

extern const DeviceInfo device_info_gd32f4xx;

inline auto getDeviceInfo() {
    return device_info_gd32f4xx;
}