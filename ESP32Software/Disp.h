#ifndef DISP_H
#define DISP_H
#include "Common.h"

// Initialize the display module (SPI, Buffers)
bool boDISP_Init();
// Set color data indexing from 0,0 left top corner
bool boDISP_SetColor(uint8_t u8RowIdx, uint8_t u8ColIdx, uint8_t u8R, uint8_t u8G, uint8_t u8B);
// Clears the buffer
void vClearBuffer();
// Sends the first buffer (after that, the sendig is done by Callback)
bool boDISP_SendBuffer();
// Stops the display
void boDISP_Stop();
// Returns true if the buffer playes out, false if stopped
bool boDISP_IsRunning();

#endif