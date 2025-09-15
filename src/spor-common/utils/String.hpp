#pragma once

#ifdef SPOR_HOST
#include "symbol-resolver/SymbolResolver.hpp"
#endif

inline bool IsSymbolInROM(const void *ptr) {
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ptr)) > 0x8000000 &&
           static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ptr)) < 0x9000000;
}

class StringOrSymbol {
public:
    enum class Type : uint8_t { String, Symbol };

    StringOrSymbol() = default;
    StringOrSymbol(const std::string &str) : data_(str) {}
    StringOrSymbol(std::string &&str) : data_(std::move(str)) {}
    StringOrSymbol(uint32_t symbol_ptr) : data_(symbol_ptr) {}
    StringOrSymbol(const char *str) {
        if (str && IsSymbolInROM(str)) {
            data_ = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(str));
        } else if (str) {
            data_ = std::string(str);
        } else {
            data_ = std::string();
        }
    }

    Type GetType() const {
        return std::holds_alternative<std::string>(data_) ? Type::String : Type::Symbol;
    }

    const std::string &AsString() const {
        return std::get<std::string>(data_);
    }

    uint32_t AsSymbol() const {
        return std::get<uint32_t>(data_);
    }

    bool IsString() const {
        return GetType() == Type::String;
    }
    bool IsSymbol() const {
        return GetType() == Type::Symbol;
    }
    bool HasData() const {
        return IsString() ? !AsString().empty() : AsSymbol() != 0;
    }

    constexpr static auto serialize(auto &archive, auto &self) {
        return archive(self.data_);
    }

#ifdef SPOR_HOST
    std::string GetString() const {
        if (this->IsString()) {
            return this->AsString();
        } else {
            return profiler::GetResolvedSymbolInfo(this->AsSymbol()).value;
        }
        // auto symbolInfo = profiler::GetResolvedSymbolInfo(symbolAddr);
        // if (!symbolInfo.name.empty()) {
        //     pointerNames[heapAddr] = symbolInfo.name;
        //
        //     // Send a message to the trace system
        //     std::string message = "Pointer announced: " + symbolInfo.name + " at 0x" + std::to_string(heapAddr);
        //     profiler::PerfettoApi::Message(message);
        // }
    }
#endif

private:
    std::variant<std::string, uint32_t> data_;
};
