// Minimal refactor of SEGGER JLink RTT host helpers
#include "JLink_RTT.h"

#include <thread>
#include <chrono>

// SWD interface value used by J-Link
static constexpr int JLINK_TIF_SWD = 1;

bool JLinkRTT::connect_and_start_rtt() {
  // Open J-Link DLL if not already open
  if (!JLINK_IsOpen()) {
    if (JLINK_Open() != 0) {
      return false;
    }
  }

  // Check if an emulator is attached
  if (JLINK_EMU_IsConnected() <= 0) {
    // No probe attached
    return false;
  }

  // Select SWD interface (common for ARM targets). Ignore error; some setups pre-configured.
  (void)JLINK_TIF_Select(JLINK_TIF_SWD);

  // Try to connect to target (if not already)
  if (!JLINK_IsConnected()) {
    if (JLINK_Connect() < 0) {
      return false;
    }
  }

  // Don't call control START here as we don't depend on the specific command codes.
  // Reading will implicitly probe; if not available we'll just get 0 bytes.
  return true;
}

int JLinkRTT::read_channel0(void* buf, std::size_t buf_size) {
  if (buf == nullptr || buf_size == 0) return 0;
  if (!JLINK_IsConnected()) return -1;
  int n = JLINK_RTTERMINAL_Read(0u, static_cast<char*>(buf), static_cast<uint32_t>(buf_size));
  return n;
}

void JLinkRTT::close() {
  JLINK_Close();
}
