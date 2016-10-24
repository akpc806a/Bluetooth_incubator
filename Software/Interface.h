//---------------------------------------------------------------------------

#ifndef InterfaceH
#define InterfaceH

#include <windows.h>

//---------------------------------------------------------------------------
#define PORT_BUFFER_COUNT 16384

#define HAL_ERROR_OK 0
#define HAL_ERROR_FATAL 256
#define HAL_ERROR_IO 258
//---------------------------------------------------------------------------
int HAL_InData(unsigned char *Data, int Count);
int HAL_OutData(unsigned char *Data, int Count);

int HAL_Init(unsigned char Index, unsigned int Speed);

int HAL_Close();
int HAL_GetInDataCount();
void HAL_ResetIn();
//---------------------------------------------------------------------------
#endif
