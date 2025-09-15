#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <memory>

namespace profiler {

enum class SymbolType { Function, Variable };

struct SymbolInfo {
    std::string name;
    std::string mangledName;
    std::string fileName;
    uint32_t lineNumber = 0;
    uint32_t columnNumber = 0;
    uint32_t size = 0;
    bool isInline = false;
    bool isWeak = false;
    bool hasDebugInfo = false;
    SymbolType type = SymbolType::Function;
    std::string value;

    SymbolInfo() = default;
    SymbolInfo(const std::string &symbolName) : name(symbolName) {}
};

class SymbolResolver {
public:
    virtual ~SymbolResolver() {}

    virtual const SymbolInfo *GetSymbolInfo(uint32_t address) = 0;
    virtual bool IsValid() const = 0;
};

void SetSymbolResolver(std::unique_ptr<SymbolResolver> resolver);
SymbolResolver *GetSymbolResolver();
SymbolInfo GetResolvedSymbolInfo(uint32_t address, std::string_view fallbackName = {});

}