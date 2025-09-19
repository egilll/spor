#pragma once

#include <args.hpp>
#include <cstdint>
#include <iostream>
#include <string>

#include "orbcat/Orbcat.hpp"

struct Options {
    std::string outputFile;
    std::string elfFile;
    std::string inputFile;

    uint32_t cpuFreq = 200'000'000;
    orbcat::Orbcat::Options orbcatOptions;

    static Options parseCommandLine(int argc, char *argv[]);
};

inline Options Options::parseCommandLine(int argc, char *argv[]) {
    Options options;

    args::ArgumentParser parser{"Profiler"};
    args::ValueFlag<uint32_t> cpufreq(parser, "cpufreq", "CPU frequency in KHz", {"cpufreq"});
    args::ValueFlag<std::string> inputFile(parser, "input-file", "Input file", {"input-file"});
    args::Flag itmSync(parser, "itm-sync", "ITM sync enforcement", {"itm-sync"});
    args::ValueFlag<std::string> server(parser, "server", "Server and port specification", {"server"}, "localhost");
    args::ValueFlag<std::string> elfFile(parser, "elf-file", "Path to ELF file for symbol resolution", {"elf-file"});
    args::ValueFlag<std::string> outputFile(
        parser, "output-file", "Perfetto output file", {"output-file"}, "trace.pftrace"
    );

    try {
        parser.ParseCLI(argc, argv);
    } catch (args::Help &) {
        std::cout << parser;
        exit(0);
    } catch (args::ParseError &e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        exit(1);
    } catch (args::ValidationError &e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        exit(1);
    }

    if (cpufreq)
        options.cpuFreq = args::get(cpufreq);
    if (inputFile)
        options.inputFile = args::get(inputFile);
    // options.orbcatOptions.itmSync = args::get(itmSync);
    options.orbcatOptions.server = args::get(server);
    if (elfFile)
        options.elfFile = args::get(elfFile);
    options.outputFile = args::get(outputFile);

    return options;
}