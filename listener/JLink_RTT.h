// Minimal refactor of SEGGER JLink RTT host helpers
// Provides a tiny wrapper around J-Link DLL RTT functions to read UP buffer 0

#pragma once

#include <cstddef>
#include <cstdint>

// We avoid including JLink headers; declare the few needed C APIs here.
extern "C" {
int   JLINK_Open(void);
void  JLINK_Close(void);
int   JLINK_IsOpen(void);
int   JLINK_IsConnected(void);
int   JLINK_EMU_IsConnected(void);
int   JLINK_TIF_Select(int Interface);
int   JLINK_Connect(void);

int   JLINK_RTTERMINAL_Control(uint32_t Cmd, void* p);
int   JLINK_RTTERMINAL_Read(uint32_t BufferIndex, char* sBuffer, uint32_t BufferSize);
int   JLINK_RTTERMINAL_Write(uint32_t BufferIndex, const char* sBuffer, uint32_t BufferSize);
}

class JLinkRTT {
public:
    // Attempts to open and connect to a J-Link probe and start RTT.
    // Returns true on success, false if no probe/target is available.
    bool connect_and_start_rtt();

    // Reads up to `buf_size` bytes from UP buffer 0 into `buf`.
    // Returns number of bytes read, 0 if none available, or <0 on fatal errors.
    int read_channel0(void *buf, std::size_t buf_size);

    // Closes the J-Link session if open.
    void close();
};
