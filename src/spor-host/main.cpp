#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "Options.hpp"
#include "orbcat/Orbcat.hpp"
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

        orbcat::MessageHandler handlers;
        handlers.onChannelData = [](uint8_t channel, uint64_t timestamp, std::span<std::byte> data) {
            // SporHost::GetInstance().HandleTimestamp(timestamp);
            MessageDecoder::GetInstance()->ProcessChannelData(channel, timestamp, data);
        };
        handlers.onException = [](const orbcat::ExceptionMessage &exception, uint64_t timestamp) {
            // SporHost::GetInstance().HandleTimestamp(timestamp);
            SporHost::GetInstance().HandleException(exception, timestamp);
        };
        handlers.onTimestamp = [](uint64_t timestamp, orbcat::TimeStatus status) {
            // profiler::PerfettoApi::SetTime(timestamp);
            // profiler::PerfettoApi::SetTime(timestamp);
            // SporHost::GetInstance().HandleTimestamp(timestamp);
        };
        orbcat::Orbcat orbcat(options.orbcatOptions, std::move(handlers));
        std::thread orbThread([&orbcat]() {
            orbcat.Start();
        });
        orbThread.join();

        profiler::PerfettoApi::StopTracing(std::move(tracing_session));
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
