// Simple RTT listener: reads all binary data from UP buffer 0 and writes to a file.

#include "JLink_RTT.h"

#include <atomic>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

static std::atomic_bool g_stop{false};

static void handle_sigint(int) {
  g_stop = true;
}

int main(int argc, char** argv) {
  const char* out_path = (argc > 1) ? argv[1] : "rtt_channel0.bin";
  std::ofstream out(out_path, std::ios::binary | std::ios::out | std::ios::trunc);
  if (!out) {
    std::cerr << "Failed to open output file: " << out_path << "\n";
    return 1;
  }

  std::signal(SIGINT, handle_sigint);

  JLinkRTT rtt;
  std::vector<unsigned char> buf(64 * 1024);

  while (!g_stop.load()) {
    if (!rtt.connect_and_start_rtt()) {
      // No J-Link attached or cannot connect. Sleep and try again.
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }

    // Connected and RTT started: drain channel 0 until disconnect or stop.
    for (;;) {
      if (g_stop.load()) break;
      int n = rtt.read_channel0(buf.data(), buf.size());
      if (n < 0) {
        // Lost connection
        break;
      } else if (n == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      } else {
        out.write(reinterpret_cast<const char*>(buf.data()), n);
        if (!out) {
          std::cerr << "Error writing to output file.\n";
          rtt.close();
          return 2;
        }
      }
    }

    rtt.close();
    if (!g_stop.load()) {
      // If we didn't ask to stop, wait a bit before re-trying
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }

  out.flush();
  return 0;
}
