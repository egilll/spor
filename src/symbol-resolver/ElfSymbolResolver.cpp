#include "symbol-resolver/ElfSymbolResolver.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cxxabi.h>
#include <dwarf.h>
#include <fcntl.h>
#include <gelf.h>
#include <iostream>
#include <libdwarf.h>
#include <memory>
#include <unistd.h>

namespace spor {

ElfSymbolResolver *ElfSymbolResolver::instance = nullptr;

ElfSymbolResolver::ElfSymbolResolver(const std::string &elfFilePath) : elfPath(elfFilePath) {
    elfFd = open(elfFilePath.c_str(), O_RDONLY);
    if (elfFd >= 0) {
        isInitialized = LoadSymbols();
    }
}

ElfSymbolResolver::~ElfSymbolResolver() {
    if (elfFd >= 0) {
        close(elfFd);
    }
}

bool ElfSymbolResolver::LoadSymbols() {
    if (elf_version(EV_CURRENT) == EV_NONE) {
        return false;
    }

    return LoadDwarfSymbols() && LoadElfSymbols() && LoadRodataSymbols();
}

bool ElfSymbolResolver::LoadDwarfSymbols() {
    Dwarf_Debug dbg;
    Dwarf_Error err;

    if (dwarf_init_b(elfFd, DW_GROUPNUMBER_ANY, nullptr, nullptr, &dbg, &err) != DW_DLV_OK) {
        return false;
    }

    Dwarf_Unsigned cu_header_length = 0;
    Dwarf_Half version_stamp = 0;
    Dwarf_Off abbrev_offset = 0;
    Dwarf_Half address_size = 0;
    Dwarf_Unsigned next_cu_header = 0;
    Dwarf_Die cu_die = nullptr;
    Dwarf_Half dw_length_size = 0;
    Dwarf_Half dw_extension_size = 0;
    Dwarf_Sig8 dw_type_signature;
    Dwarf_Unsigned dw_typeoffset = 0;
    Dwarf_Half dw_header_cu_type = DW_UT_compile;

    memset(&dw_type_signature, 0, sizeof(dw_type_signature));

    while (true) {
        if (dwarf_next_cu_header_d(
                dbg, true, &cu_header_length, &version_stamp, &abbrev_offset, &address_size, &dw_length_size,
                &dw_extension_size, &dw_type_signature, &dw_typeoffset, &next_cu_header, &dw_header_cu_type, nullptr
            ) != DW_DLV_OK) {
            break;
        }

        if (dwarf_siblingof_b(dbg, nullptr, true, &cu_die, nullptr) != DW_DLV_OK) {
            continue;
        }

        ProcessDieRecursively(dbg, cu_die);
        dwarf_dealloc(dbg, cu_die, DW_DLA_DIE);
    }

    LoadWeakSymbolsFromDwarf(dbg);
    dwarf_finish(dbg);

    return true;
}

void ElfSymbolResolver::ProcessDieRecursively(Dwarf_Debug dbg, Dwarf_Die die) {
    Dwarf_Die sib = die;

    while (dwarf_siblingof_b(dbg, sib, true, &sib, nullptr) == DW_DLV_OK) {
        Dwarf_Half tag;
        if (dwarf_tag(sib, &tag, nullptr) == DW_DLV_OK) {
            if (tag == DW_TAG_subprogram || tag == DW_TAG_inlined_subroutine) {
                ExtractFunctionInfo(dbg, sib);
            } else if (tag == DW_TAG_variable) {
                ExtractVariableInfo(dbg, sib);
            }
        }
    }

    Dwarf_Die child;
    if (dwarf_child(die, &child, nullptr) == DW_DLV_OK) {
        ProcessDieRecursively(dbg, child);
        dwarf_dealloc(dbg, child, DW_DLA_DIE);
    }
}

void ElfSymbolResolver::ExtractFunctionInfo(Dwarf_Debug dbg, Dwarf_Die die) {
    char *name = nullptr;
    char *fileName = nullptr;
    char *mangledName = nullptr;
    Dwarf_Addr lowAddr = 0;
    Dwarf_Addr highAddr = 0;
    Dwarf_Unsigned lineNumber = 0;
    Dwarf_Unsigned columnNumber = 0;
    enum Dwarf_Form_Class formclass = DW_FORM_CLASS_UNKNOWN;
    Dwarf_Attribute attr;

    if (dwarf_diename(die, &name, nullptr) == DW_DLV_OK && dwarf_lowpc(die, &lowAddr, nullptr) == DW_DLV_OK &&
        dwarf_highpc_b(die, &highAddr, nullptr, &formclass, nullptr) == DW_DLV_OK) {

        if (formclass == DW_FORM_CLASS_CONSTANT) {
            highAddr += lowAddr;
        }

        if (name && highAddr && lowAddr <= UINT32_MAX) {
            profiler::SymbolInfo symbolInfo;
            symbolInfo.name = DemangleFunctionName(name);
            symbolInfo.hasDebugInfo = true;
            symbolInfo.size = static_cast<uint32_t>(highAddr - lowAddr);

            if (dwarf_attr(die, DW_AT_linkage_name, &attr, nullptr) == DW_DLV_OK) {
                if (dwarf_formstring(attr, &mangledName, nullptr) == DW_DLV_OK && mangledName) {
                    symbolInfo.mangledName = mangledName;
                }
            }

            if (dwarf_attr(die, DW_AT_decl_file, &attr, nullptr) == DW_DLV_OK) {
                Dwarf_Unsigned fileIndex;
                if (dwarf_formudata(attr, &fileIndex, nullptr) == DW_DLV_OK) {
                    char **srcFiles;
                    Dwarf_Signed fileCount;
                    if (dwarf_srcfiles(die, &srcFiles, &fileCount, nullptr) == DW_DLV_OK &&
                        fileIndex < static_cast<Dwarf_Unsigned>(fileCount)) {
                        symbolInfo.fileName = srcFiles[fileIndex];
                    }
                }
            }

            if (dwarf_attr(die, DW_AT_decl_line, &attr, nullptr) == DW_DLV_OK) {
                if (dwarf_formudata(attr, &lineNumber, nullptr) == DW_DLV_OK) {
                    symbolInfo.lineNumber = static_cast<uint32_t>(lineNumber);
                }
            }

            if (dwarf_attr(die, DW_AT_decl_column, &attr, nullptr) == DW_DLV_OK) {
                if (dwarf_formudata(attr, &columnNumber, nullptr) == DW_DLV_OK) {
                    symbolInfo.columnNumber = static_cast<uint32_t>(columnNumber);
                }
            }

            Dwarf_Half tag;
            if (dwarf_tag(die, &tag, nullptr) == DW_DLV_OK) {
                symbolInfo.isInline = (tag == DW_TAG_inlined_subroutine);
            }

            symbols[static_cast<uint32_t>(lowAddr)] = std::move(symbolInfo);
        }
    }
}

void ElfSymbolResolver::LoadWeakSymbolsFromDwarf(Dwarf_Debug dbg) {
    Dwarf_Error err;
    Dwarf_Global *globals = nullptr;
    Dwarf_Signed globalCount = 0;

    if (dwarf_globals_by_type(dbg, DW_GL_WEAKS, &globals, &globalCount, &err) == DW_DLV_OK) {
        for (Dwarf_Signed i = 0; i < globalCount; ++i) {
            char *name = nullptr;
            Dwarf_Off dieOffset = 0;
            Dwarf_Die die = nullptr;

            Dwarf_Off cuDieOffset = 0;
            if (dwarf_global_name_offsets(globals[i], &name, &dieOffset, &cuDieOffset, nullptr) == DW_DLV_OK &&
                dwarf_offdie_b(dbg, dieOffset, true, &die, nullptr) == DW_DLV_OK) {

                Dwarf_Addr lowAddr = 0;
                if (dwarf_lowpc(die, &lowAddr, nullptr) == DW_DLV_OK && lowAddr <= UINT32_MAX) {
                    uint32_t address = static_cast<uint32_t>(lowAddr);

                    if (symbols.find(address) == symbols.end()) {
                        profiler::SymbolInfo symbolInfo;
                        symbolInfo.name = DemangleFunctionName(name);
                        symbolInfo.isWeak = true;
                        symbolInfo.hasDebugInfo = true;

                        Dwarf_Half tag;
                        if (dwarf_tag(die, &tag, nullptr) == DW_DLV_OK &&
                            (tag == DW_TAG_subprogram || tag == DW_TAG_inlined_subroutine)) {

                            Dwarf_Addr highAddr = 0;
                            enum Dwarf_Form_Class formclass = DW_FORM_CLASS_UNKNOWN;
                            if (dwarf_highpc_b(die, &highAddr, nullptr, &formclass, nullptr) == DW_DLV_OK) {
                                if (formclass == DW_FORM_CLASS_CONSTANT) {
                                    highAddr += lowAddr;
                                }
                                symbolInfo.size = static_cast<uint32_t>(highAddr - lowAddr);
                            }

                            symbols[address] = std::move(symbolInfo);
                        }
                    }
                }

                dwarf_dealloc(dbg, die, DW_DLA_DIE);
            }
        }

        dwarf_globals_dealloc(dbg, globals, globalCount);
    }
}

bool ElfSymbolResolver::LoadElfSymbols() {
    Elf *elf;
    Elf_Scn *scn = nullptr;
    GElf_Shdr shdr;
    size_t shstrndx;

    if ((elf = elf_begin(elfFd, ELF_C_READ, nullptr)) == nullptr) {
        return false;
    }

    if (elf_getshdrstrndx(elf, &shstrndx) != 0) {
        elf_end(elf);
        return false;
    }

    while ((scn = elf_nextscn(elf, scn)) != nullptr) {
        if (gelf_getshdr(scn, &shdr) != &shdr) {
            continue;
        }

        if (shdr.sh_type == SHT_SYMTAB || shdr.sh_type == SHT_DYNSYM) {
            ProcessSymbolTable(elf, scn, &shdr, shstrndx);
        }
    }

    elf_end(elf);
    return true;
}

bool ElfSymbolResolver::LoadRodataSymbols() {
    Elf *elf;
    Elf_Scn *scn = nullptr;
    GElf_Shdr shdr;
    size_t shstrndx;

    if ((elf = elf_begin(elfFd, ELF_C_READ, nullptr)) == nullptr) {
        return false;
    }

    if (elf_getshdrstrndx(elf, &shstrndx) != 0) {
        elf_end(elf);
        return false;
    }

    while ((scn = elf_nextscn(elf, scn)) != nullptr) {
        if (gelf_getshdr(scn, &shdr) != &shdr) {
            continue;
        }

        char *sectionName = elf_strptr(elf, shstrndx, shdr.sh_name);
        if (sectionName && strcmp(sectionName, ".rodata") == 0) {
            Elf_Data *data = elf_rawdata(scn, nullptr);
            if (data && data->d_buf) {
                ExtractRodataStrings(
                    static_cast<uint32_t>(shdr.sh_addr), static_cast<const char *>(data->d_buf), data->d_size
                );
            }
            break;
        }
    }

    elf_end(elf);
    return true;
}

void ElfSymbolResolver::ExtractRodataStrings(uint32_t baseAddr, const char *data, size_t size) {
    for (size_t i = 0; i < size;) {
        const char *str = data + i;
        size_t strLen = strnlen(str, size - i);

        if (strLen > 0 && strLen < size - i) {
            profiler::SymbolInfo symbolInfo;
            symbolInfo.name = "rodata_string";
            symbolInfo.type = profiler::SymbolType::Variable;
            symbolInfo.value = std::string(str, strLen);
            symbolInfo.hasDebugInfo = false;

            symbols[baseAddr + i] = std::move(symbolInfo);
        }

        i += strLen + 1;
    }
}

std::string ElfSymbolResolver::DemangleFunctionName(const char *mangledName) {
    if (!mangledName) {
        return "";
    }

    int status = 0;
    char *demangled = abi::__cxa_demangle(mangledName, nullptr, nullptr, &status);

    if (status == 0 && demangled) {
        std::string result(demangled);
        free(demangled);
        return result;
    }

    return std::string(mangledName);
}

const profiler::SymbolInfo *ElfSymbolResolver::GetSymbolInfo(uint32_t address) {
    auto it = symbols.find(address);
    if (it != symbols.end()) {
        return &it->second;
    }

    return nullptr;
}

bool ElfSymbolResolver::IsValid() const {
    return isInitialized && elfFd >= 0;
}

void ElfSymbolResolver::ProcessSymbolTable(Elf *elf, Elf_Scn *scn, GElf_Shdr *shdr, size_t shstrndx) {
    Elf_Data *data = elf_getdata(scn, nullptr);
    if (!data) {
        return;
    }

    size_t symbolCount = shdr->sh_size / shdr->sh_entsize;

    for (size_t i = 0; i < symbolCount; ++i) {
        GElf_Sym sym;
        if (gelf_getsym(data, static_cast<int>(i), &sym) != &sym) {
            continue;
        }

        if (sym.st_value > 0 && sym.st_value <= UINT32_MAX) {
            int symType = ELF64_ST_TYPE(sym.st_info);
            const char *symbolName = elf_strptr(elf, shdr->sh_link, sym.st_name);
            
            if (symType == STT_FUNC || symType == STT_OBJECT || symType == STT_COMMON || symType == STT_NOTYPE) {
                
                uint32_t address = static_cast<uint32_t>(sym.st_value);

                if (symbols.find(address) == symbols.end()) {
                    profiler::SymbolInfo symbolInfo;
                    
                    if (symType == STT_FUNC) {
                        symbolInfo.name = symbolName ? DemangleFunctionName(symbolName) : "<unnamed_function>";
                        symbolInfo.type = profiler::SymbolType::Function;
                    } else {
                        if (symbolName && strlen(symbolName) > 0) {
                            symbolInfo.name = symbolName;
                        } else if (symType == STT_NOTYPE) {
                            symbolInfo.name = "<unnamed_symbol>";
                        } else {
                            symbolInfo.name = "<unnamed_data>";
                        }
                        symbolInfo.type = profiler::SymbolType::Variable;
                    }
                    
                    if (symbolName) {
                        symbolInfo.mangledName = symbolName;
                    }
                    
                    symbolInfo.size = static_cast<uint32_t>(sym.st_size);
                    symbolInfo.isWeak = (ELF64_ST_BIND(sym.st_info) == STB_WEAK);
                    symbolInfo.hasDebugInfo = false;

                    if (symType == STT_OBJECT || symType == STT_COMMON || symType == STT_NOTYPE) {
                        Elf_Scn *targetScn = nullptr;
                        while ((targetScn = elf_nextscn(elf, targetScn)) != nullptr) {
                            GElf_Shdr targetShdr;
                            if (gelf_getshdr(targetScn, &targetShdr) == &targetShdr &&
                                sym.st_value >= targetShdr.sh_addr &&
                                sym.st_value < targetShdr.sh_addr + targetShdr.sh_size) {

                                char *sectionName = elf_strptr(elf, shstrndx, targetShdr.sh_name);
                                if (sectionName) {
                                    symbolInfo.value = std::string("section:") + sectionName;
                                }
                                
                                if (symType == STT_NOTYPE) {
                                    Elf_Data *sectionData = elf_getdata(targetScn, nullptr);
                                    if (sectionData && sectionData->d_buf) {
                                        size_t offset = sym.st_value - targetShdr.sh_addr;
                                        if (offset < sectionData->d_size) {
                                            const char *dataPtr = static_cast<const char*>(sectionData->d_buf) + offset;
                                            size_t maxLen = sectionData->d_size - offset;
                                            
                                            size_t strLen = strnlen(dataPtr, std::min(maxLen, size_t(256)));
                                            if (strLen > 0 && strLen < maxLen) {
                                                symbolInfo.value = std::string(dataPtr, strLen);
                                            } else {
                                                std::string hexValue;
                                                size_t bytesToShow = std::min(maxLen, size_t(32));
                                                for (size_t i = 0; i < bytesToShow; ++i) {
                                                    char hex[4];
                                                    snprintf(hex, sizeof(hex), "%02x ", static_cast<unsigned char>(dataPtr[i]));
                                                    hexValue += hex;
                                                }
                                                if (maxLen > 32) {
                                                    hexValue += "...";
                                                }
                                                symbolInfo.value = hexValue;
                                            }
                                        }
                                    }
                                }
                                break;
                            }
                        }
                    }

                    symbols[address] = std::move(symbolInfo);
                }
            }
        }
    }
}

void ElfSymbolResolver::ExtractVariableInfo(Dwarf_Debug dbg, Dwarf_Die die) {
    char *name = nullptr;
    char *fileName = nullptr;
    char *mangledName = nullptr;
    Dwarf_Addr address = 0;
    Dwarf_Unsigned lineNumber = 0;
    Dwarf_Unsigned columnNumber = 0;
    Dwarf_Attribute attr;

    if (dwarf_diename(die, &name, nullptr) != DW_DLV_OK || !name) {
        return;
    }
    //
    // bool isGlobalVariable = false;
    // if (dwarf_attr(die, DW_AT_external, &attr, nullptr) == DW_DLV_OK) {
    //     Dwarf_Bool external_flag;
    //     if (dwarf_formflag(attr, &external_flag, nullptr) == DW_DLV_OK) {
    //         isGlobalVariable = external_flag;
    //     }
    // }

    if (dwarf_attr(die, DW_AT_location, &attr, nullptr) == DW_DLV_OK) {
        Dwarf_Half form;
        if (dwarf_whatform(attr, &form, nullptr) == DW_DLV_OK) {
            if (form == DW_FORM_exprloc || form == DW_FORM_block1 || form == DW_FORM_block2 || form == DW_FORM_block4) {
                Dwarf_Ptr expr_bytes;
                Dwarf_Unsigned expr_len;
                if (dwarf_formexprloc(attr, &expr_len, &expr_bytes, nullptr) == DW_DLV_OK) {
                    const uint8_t *bytes = static_cast<const uint8_t *>(expr_bytes);

                    if (expr_len >= 5 && bytes[0] == DW_OP_addr) {
                        memcpy(&address, &bytes[1], sizeof(uint32_t));
                    } else if (expr_len >= 1 && bytes[0] >= DW_OP_reg0 && bytes[0] <= DW_OP_reg31) {
                        return;
                    } else if (expr_len >= 2 && bytes[0] == DW_OP_fbreg) {
                        return;
                    }
                }
            } else if (form == DW_FORM_data4 || form == DW_FORM_data8) {
                if (dwarf_formudata(attr, reinterpret_cast<Dwarf_Unsigned *>(&address), nullptr) == DW_DLV_OK) {}
            }
        }
    }

    if (address > 0 && address <= UINT32_MAX) {
        uint32_t varAddress = static_cast<uint32_t>(address);

        if (symbols.find(varAddress) == symbols.end()) {
            profiler::SymbolInfo symbolInfo;
            symbolInfo.name = name;
            symbolInfo.type = profiler::SymbolType::Variable;
            symbolInfo.hasDebugInfo = true;

            if (dwarf_attr(die, DW_AT_linkage_name, &attr, nullptr) == DW_DLV_OK) {
                if (dwarf_formstring(attr, &mangledName, nullptr) == DW_DLV_OK && mangledName) {
                    symbolInfo.mangledName = mangledName;
                }
            }

            if (dwarf_attr(die, DW_AT_decl_file, &attr, nullptr) == DW_DLV_OK) {
                Dwarf_Unsigned fileIndex;
                if (dwarf_formudata(attr, &fileIndex, nullptr) == DW_DLV_OK) {
                    char **srcFiles;
                    Dwarf_Signed fileCount;
                    if (dwarf_srcfiles(die, &srcFiles, &fileCount, nullptr) == DW_DLV_OK &&
                        fileIndex < static_cast<Dwarf_Unsigned>(fileCount)) {
                        symbolInfo.fileName = srcFiles[fileIndex];
                    }
                }
            }

            if (dwarf_attr(die, DW_AT_decl_line, &attr, nullptr) == DW_DLV_OK) {
                if (dwarf_formudata(attr, &lineNumber, nullptr) == DW_DLV_OK) {
                    symbolInfo.lineNumber = static_cast<uint32_t>(lineNumber);
                }
            }

            if (dwarf_attr(die, DW_AT_decl_column, &attr, nullptr) == DW_DLV_OK) {
                if (dwarf_formudata(attr, &columnNumber, nullptr) == DW_DLV_OK) {
                    symbolInfo.columnNumber = static_cast<uint32_t>(columnNumber);
                }
            }

            if (dwarf_attr(die, DW_AT_type, &attr, nullptr) == DW_DLV_OK) {
                Dwarf_Off type_offset;
                if (dwarf_global_formref(attr, &type_offset, nullptr) == DW_DLV_OK) {
                    Dwarf_Die type_die;
                    if (dwarf_offdie_b(dbg, type_offset, true, &type_die, nullptr) == DW_DLV_OK) {
                        char *type_name = nullptr;
                        if (dwarf_diename(type_die, &type_name, nullptr) == DW_DLV_OK && type_name) {
                            symbolInfo.value = std::string("type:") + type_name;
                        }
                        dwarf_dealloc(dbg, type_die, DW_DLA_DIE);
                    }
                }
            }

            symbols[varAddress] = std::move(symbolInfo);
        }
    }
}


}