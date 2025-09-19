#include <algorithm>
#include <chrono>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <thread>

#include "Options.hpp"
// #include "orbcat/Orbcat.hpp"
#include "PerfettoApi.hpp"
#include "SporHost.hpp"
#include "symbol-resolver/ElfSymbolResolver.hpp"

int main(int argc, char *argv[]) {
    auto options = Options::parseCommandLine(argc, argv);

    try {
        auto tracing_session = profiler::PerfettoApi::StartTracing();

        if (!options.elfFile.empty()) {
            auto symbolResolver = std::make_unique<spor::ElfSymbolResolver>(options.elfFile);
            if (symbolResolver->IsValid()) {
                std::cout << "Symbol resolver initialized with ELF file: " << options.elfFile << std::endl;
                profiler::SetSymbolResolver(std::move(symbolResolver));
                SporHost::GetInstance().SymbolsLoaded();
            } else {
                std::cerr << "Failed to initialize symbol resolver with ELF file: " << options.elfFile << std::endl;
            }
        }

        if (options.inputFile.empty()) {
            std::cerr << "--input-file is required for offline decode" << std::endl;
            return 1;
        }

        std::ifstream ifs(options.inputFile, std::ios::binary);
        if (!ifs) {
            std::cerr << "Failed to open input file: " << options.inputFile << std::endl;
            return 1;
        }
        // Skip initial bytes (e.g., header) and start reading from offset 0x70
        ifs.seekg(0x70, std::ios::beg);
        if (!ifs) {
            std::cerr << "Failed to seek to offset 0x70 in input file: " << options.inputFile << std::endl;
            return 1;
        }
        std::vector<std::byte> buf(64 * 1024);
        while (ifs) {
            ifs.read(reinterpret_cast<char *>(buf.data()), static_cast<std::streamsize>(buf.size()));
            std::streamsize got = ifs.gcount();
            if (got <= 0) break;
            std::span<const std::byte> data(buf.data(), static_cast<size_t>(got));
            MessageDecoder::GetInstance()->ProcessBytes(data);
        }

        profiler::PerfettoApi::StopTracing(std::move(tracing_session));
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
