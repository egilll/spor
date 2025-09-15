#include "SymbolResolver.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace profiler {

std::unique_ptr<SymbolResolver> resolver_;

void SetSymbolResolver(std::unique_ptr<SymbolResolver> resolver) {
    resolver_ = std::move(resolver);
}
SymbolResolver *GetSymbolResolver() {
    return resolver_.get();
}
SymbolInfo GetResolvedSymbolInfo(uint32_t address, std::string_view fallbackName) {
    if (!resolver_) {
        return SymbolInfo{std::string(fallbackName)};
    }
    return resolver_->GetSymbolInfo(address) ? *resolver_->GetSymbolInfo(address)
                                             : SymbolInfo{std::string(fallbackName)};
}

}