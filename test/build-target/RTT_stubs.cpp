#include <cstddef>

extern "C" {

// Weak no-op stubs so unit builds link without SEGGER RTT objects.
// On real targets, link SEGGER RTT to override these.
__attribute__((weak)) unsigned SEGGER_RTT_WriteNoLock(unsigned BufferIndex, const void *pBuffer, unsigned NumBytes) {
    (void)BufferIndex;
    (void)pBuffer;
    return NumBytes;
}

__attribute__((weak)) unsigned SEGGER_RTT_Write(unsigned BufferIndex, const void *pBuffer, unsigned NumBytes) {
    (void)BufferIndex;
    (void)pBuffer;
    return NumBytes;
}

__attribute__((weak)) unsigned SEGGER_RTT_PutChar(unsigned BufferIndex, char c) {
    (void)BufferIndex;
    (void)c;
    return 1;
}

__attribute__((weak)) int SEGGER_RTT_AllocUpBuffer(const char *sName, void *pBuffer, unsigned BufferSize, unsigned Flags) {
    (void)sName;
    (void)pBuffer;
    (void)BufferSize;
    (void)Flags;
    return 0; // default to buffer 0 in tests
}

__attribute__((weak)) void SEGGER_RTT_Init(void) {}

}
