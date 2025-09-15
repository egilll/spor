#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <gelf.h>

#include "spor-common/TargetPointer.hpp"
#include "SymbolResolver.hpp"

struct Dwarf_Debug_s;
struct Dwarf_Die_s;
using Dwarf_Debug = Dwarf_Debug_s *;
using Dwarf_Die = Dwarf_Die_s *;

namespace spor {

class ElfSymbolResolver : public profiler::SymbolResolver {
private:
public:
    std::string elfPath;
    int elfFd = -1;
    bool isInitialized = false;

    std::unordered_map<TargetPointer, profiler::SymbolInfo> symbols;

    explicit ElfSymbolResolver(const std::string &elfFilePath);
    ~ElfSymbolResolver() override;

    ElfSymbolResolver(const ElfSymbolResolver &) = delete;
    ElfSymbolResolver &operator=(const ElfSymbolResolver &) = delete;
    ElfSymbolResolver(ElfSymbolResolver &&) = delete;
    ElfSymbolResolver &operator=(ElfSymbolResolver &&) = delete;

    const profiler::SymbolInfo *GetSymbolInfo(TargetPointer address) override;
    bool IsValid() const override;

private:
    static ElfSymbolResolver *instance;

    bool LoadSymbols();
    bool LoadDwarfSymbols();
    bool LoadElfSymbols();
    bool LoadRodataSymbols();
    void LoadWeakSymbolsFromDwarf(Dwarf_Debug dbg);
    void ProcessSymbolTable(Elf *elf, Elf_Scn *scn, GElf_Shdr *shdr, size_t shstrndx);
    std::string DemangleFunctionName(const char *mangledName);
    void ProcessDieRecursively(Dwarf_Debug dbg, Dwarf_Die die);
    void ExtractFunctionInfo(Dwarf_Debug dbg, Dwarf_Die die);
    void ExtractVariableInfo(Dwarf_Debug dbg, Dwarf_Die die);
    void ExtractRodataStrings(uint32_t baseAddr, const char *data, size_t size);
};

}