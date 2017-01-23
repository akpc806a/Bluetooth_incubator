//---------------------------------------------------------------------------

#ifndef InterfaceH
#define InterfaceH

#include <fmx.h>
//---------------------------------------------------------------------------
#define PORT_BUFFER_COUNT 16384

#define HAL_ERROR_OK 0
#define HAL_ERROR_FATAL 256
#define HAL_ERROR_IO 258
#define HAL_ERROR_INIT 257
#define HAL_ERROR_PARAM 259
#define HAL_ERROR_TIMEOUT 260
//---------------------------------------------------------------------------
int HAL_InData(unsigned char *Data, int Count);
int HAL_OutData(unsigned char *Data, int Count);

int HAL_Init();

void HAL_GetDeviceList(TStringList* device_list);
int HAL_Connect(int dev_index);

int HAL_Close();
int HAL_GetInDataCount();
void HAL_ResetIn();
//---------------------------------------------------------------------------
#endif
