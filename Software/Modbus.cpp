//---------------------------------------------------------------------------
//#define TOTAL_LOGGING

#pragma hdrstop

#include "Interface.h"
#include "Modbus.h"
//---------------------------------------------------------------------------
#define MODBUS_BUFFER_COUNT 1024
static unsigned char bOutBuffer[MODBUS_BUFFER_COUNT];
static unsigned char bInBuffer[MODBUS_BUFFER_COUNT];

// simplies MODBUS master functions
void CalcCRC16(unsigned char *Data, int Count, unsigned short int* Crc)
{
  int i;
  int iS;
  unsigned short int iCrc;
  unsigned short int iCrcConst;

  // direct CRC
  iCrc = 0xFFFF;
  iCrcConst = 0xA001;

  for (i = 0; i < Count; i++)
  {
    iCrc = ((short int)Data[i]) ^ iCrc;
    for (iS = 0; iS < 8; iS++)
    {
      if (iCrc & 0x1)
      {
        iCrc = (iCrc >> 1) & 0x7FFF;
        iCrc = iCrc ^ iCrcConst;
      }
      else
        iCrc = (iCrc >> 1) & 0x7FFF;
    }
  }

  *Crc = iCrc;
}

int ModBus_WriteReg(unsigned char ID, int Addr, short int Data)
{
  int iRes;
  unsigned char *bBuffer;
  int i;

  bBuffer = bOutBuffer;
  *bBuffer = ID; bBuffer++;
  *bBuffer = 0x6; bBuffer++;
  *bBuffer = Addr >> 8; bBuffer++;
  *bBuffer = Addr; bBuffer++;
  *bBuffer = Data >> 8; bBuffer++;
  *bBuffer = Data; bBuffer++;

  CalcCRC16(bOutBuffer, (int)(bBuffer-bOutBuffer), (unsigned short int*)bBuffer);
  bBuffer += 2;

  iRes = HAL_OutData(bOutBuffer, (int)(bBuffer-bOutBuffer));
  if (iRes != HAL_ERROR_OK) return iRes;

  iRes = HAL_InData(bInBuffer, (int)(bBuffer-bOutBuffer));
  if (iRes != HAL_ERROR_OK) return iRes;

  for (i = 0; i < (int)(bBuffer-bOutBuffer); i++)
    if (bInBuffer[i] != bOutBuffer[i])
      return MODBUS_ERROR_FORMAT;

  return MODBUS_ERROR_OK;
}

int ModBus_ReadReg(unsigned char ID, int Addr, short int *Data)
{
  int iRes;
  unsigned char *bBuffer;
  unsigned short int iCrc;

  bBuffer = bOutBuffer;
  *bBuffer = ID; bBuffer++;
  *bBuffer = 0x4; bBuffer++;
  *bBuffer = Addr >> 8; bBuffer++;
  *bBuffer = Addr; bBuffer++;
  *bBuffer = 0; bBuffer++;
  *bBuffer = 1; bBuffer++;

  CalcCRC16(bOutBuffer, (int)(bBuffer-bOutBuffer), (unsigned short int*)bBuffer);
  bBuffer += 2;

  iRes = HAL_OutData(bOutBuffer, (int)(bBuffer-bOutBuffer));
  if (iRes != HAL_ERROR_OK) return iRes;

  iRes = HAL_InData(bInBuffer, 7);
  if (iRes != HAL_ERROR_OK) return iRes;

  bBuffer = bInBuffer;
  if (*(bBuffer++) != ID) return MODBUS_ERROR_FORMAT;
  if (*(bBuffer++) != 4) return MODBUS_ERROR_FORMAT;
  if (*(bBuffer++) != 2) return MODBUS_ERROR_FORMAT;
//  *Data = *((short int*)bBuffer); bBuffer += 2;
  *Data = bBuffer[0] | (bBuffer[1] << 8); bBuffer += 2;

  CalcCRC16(bInBuffer, 5, &iCrc);
  if (iCrc != (bBuffer[0] | (bBuffer[1] << 8))) return MODBUS_ERROR_CRC;

  *Data = ((*Data >> 8) & 0xFF) | ((*Data << 8) & 0xFF00);

  return MODBUS_ERROR_OK;
}

int ModBus_ReadRegs(unsigned char ID, int StartAddr, int Count, short int *Data)
{
  int iRes;
  unsigned char *bBuffer;
  unsigned short int iCrc;

  bBuffer = bOutBuffer;
  *bBuffer = ID; bBuffer++;
  *bBuffer = 0x3; bBuffer++;
  *bBuffer = StartAddr >> 8; bBuffer++;
  *bBuffer = StartAddr; bBuffer++;
  *bBuffer = Count >> 8; bBuffer++;
  *bBuffer = Count; bBuffer++;

  CalcCRC16(bOutBuffer, (int)(bBuffer-bOutBuffer), (unsigned short int*)bBuffer);
  bBuffer += 2;

  iRes = HAL_OutData(bOutBuffer, (int)(bBuffer-bOutBuffer));
  if (iRes != HAL_ERROR_OK) return iRes;

  iRes = HAL_InData(bInBuffer, 5+2*Count);
  if (iRes != HAL_ERROR_OK) return iRes;

  bBuffer = bInBuffer;
  if (*(bBuffer++) != ID) return MODBUS_ERROR_FORMAT;
  if (*(bBuffer++) != 0x3) return MODBUS_ERROR_FORMAT; // function code
  if (*(bBuffer++) != (2*Count)) return MODBUS_ERROR_FORMAT; // number of bytes
  for (int i = 0; i < Count; i++)
  {
    Data[i] = ((bBuffer[0] << 8) & 0xFF00) | ((bBuffer[1]) & 0xFF);
    bBuffer += 2;
  }

  CalcCRC16(bInBuffer, 3+2*Count, &iCrc);
  if (iCrc != (bBuffer[0] | (bBuffer[1] << 8))) return MODBUS_ERROR_CRC;

  return MODBUS_ERROR_OK;
}
//---------------------------------------------------------------------------
int ModBus_ReadReg_Req(unsigned char ID, int Addr)
{
  int iRes;
  unsigned char *bBuffer;

  bBuffer = bOutBuffer;
  *bBuffer = ID; bBuffer++;
  *bBuffer = 0x4; bBuffer++;
  *bBuffer = Addr >> 8; bBuffer++;
  *bBuffer = Addr; bBuffer++;
  *bBuffer = 0; bBuffer++;
  *bBuffer = 1; bBuffer++;

  CalcCRC16(bOutBuffer, (int)(bBuffer-bOutBuffer), (unsigned short int*)bBuffer);
  bBuffer += 2;

  iRes = HAL_OutData(bOutBuffer, (int)(bBuffer-bOutBuffer));
  if (iRes != HAL_ERROR_OK) return iRes;

  return MODBUS_ERROR_OK;
}

int ModBus_ReadReg_Recv(unsigned char ID, short int *Data)
{
  int iRes;
  unsigned char *bBuffer;
  unsigned short int iCrc;
  
  iRes = HAL_InData(bInBuffer, 7);
  if (iRes != HAL_ERROR_OK) return iRes;

  bBuffer = bInBuffer;
  if (*(bBuffer++) != ID) return MODBUS_ERROR_FORMAT;
  if (*(bBuffer++) != 4) return MODBUS_ERROR_FORMAT;
  if (*(bBuffer++) != 2) return MODBUS_ERROR_FORMAT;
//  *Data = *((short int*)bBuffer); bBuffer += 2;
  *Data = bBuffer[0] | (bBuffer[1] << 8); bBuffer += 2;

  CalcCRC16(bInBuffer, 5, &iCrc);
  if (iCrc != (bBuffer[0] | (bBuffer[1] << 8))) return MODBUS_ERROR_CRC;

  *Data = ((*Data >> 8) & 0xFF) | ((*Data << 8) & 0xFF00);

  return MODBUS_ERROR_OK;
}

unsigned char Modbus_IsDataAvailable()
{
  int iCount = HAL_GetInDataCount();
  return (iCount > 0);
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#pragma package(smart_init)
