#pragma once

extern "C" {
int SEGGER_RTT_AllocDownBuffer(const char *sName, void *pBuffer, unsigned BufferSize, unsigned Flags);
int SEGGER_RTT_AllocUpBuffer(const char *sName, void *pBuffer, unsigned BufferSize, unsigned Flags);
int SEGGER_RTT_ConfigUpBuffer(
    unsigned BufferIndex, const char *sName, void *pBuffer, unsigned BufferSize, unsigned Flags
);
int SEGGER_RTT_ConfigDownBuffer(
    unsigned BufferIndex, const char *sName, void *pBuffer, unsigned BufferSize, unsigned Flags
);
int SEGGER_RTT_GetKey(void);
unsigned SEGGER_RTT_HasData(unsigned BufferIndex);
int SEGGER_RTT_HasKey(void);
unsigned SEGGER_RTT_HasDataUp(unsigned BufferIndex);
void SEGGER_RTT_Init(void);
unsigned SEGGER_RTT_Read(unsigned BufferIndex, void *pBuffer, unsigned BufferSize);
unsigned SEGGER_RTT_ReadNoLock(unsigned BufferIndex, void *pData, unsigned BufferSize);
int SEGGER_RTT_SetNameDownBuffer(unsigned BufferIndex, const char *sName);
int SEGGER_RTT_SetNameUpBuffer(unsigned BufferIndex, const char *sName);
int SEGGER_RTT_SetFlagsDownBuffer(unsigned BufferIndex, unsigned Flags);
int SEGGER_RTT_SetFlagsUpBuffer(unsigned BufferIndex, unsigned Flags);
int SEGGER_RTT_WaitKey(void);
unsigned SEGGER_RTT_Write(unsigned BufferIndex, const void *pBuffer, unsigned NumBytes);
unsigned SEGGER_RTT_WriteNoLock(unsigned BufferIndex, const void *pBuffer, unsigned NumBytes);
unsigned SEGGER_RTT_WriteSkipNoLock(unsigned BufferIndex, const void *pBuffer, unsigned NumBytes);
unsigned SEGGER_RTT_ASM_WriteSkipNoLock(unsigned BufferIndex, const void *pBuffer, unsigned NumBytes);
unsigned SEGGER_RTT_WriteString(unsigned BufferIndex, const char *s);
void SEGGER_RTT_WriteWithOverwriteNoLock(unsigned BufferIndex, const void *pBuffer, unsigned NumBytes);
unsigned SEGGER_RTT_PutChar(unsigned BufferIndex, char c);
unsigned SEGGER_RTT_PutCharSkip(unsigned BufferIndex, char c);
unsigned SEGGER_RTT_PutCharSkipNoLock(unsigned BufferIndex, char c);
unsigned SEGGER_RTT_GetAvailWriteSpace(unsigned BufferIndex);
unsigned SEGGER_RTT_GetBytesInBuffer(unsigned BufferIndex);
}