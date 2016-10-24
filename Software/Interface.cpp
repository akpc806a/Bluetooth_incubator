//---------------------------------------------------------------------------
//#define TOTAL_LOGGING

#pragma hdrstop

#include <stdio.h>
#include <time.h>

#include "Interface.h"
//---------------------------------------------------------------------------
HANDLE hCOMPort;

int HAL_Init(unsigned char Index, unsigned int Speed)
{
  DCB structDCB;

  char sName[32]; *sName = 0;
  char sNumber[32];
  if (Index >= 10)
    strcpy(sName, "\\\\.\\");
  strcat(sName, "COM");
  itoa(Index, sNumber, 10);
  strcat(sName, sNumber);

  hCOMPort = CreateFile(sName, GENERIC_READ|GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
  if (hCOMPort == 0)
    return HAL_ERROR_FATAL;

  COMMTIMEOUTS comTimeOut;
  GetCommTimeouts(hCOMPort,&comTimeOut);
  comTimeOut.ReadIntervalTimeout = 1000;
  comTimeOut.ReadTotalTimeoutMultiplier = 10;
  comTimeOut.ReadTotalTimeoutConstant = 1000;
  comTimeOut.WriteTotalTimeoutMultiplier = 1000;
  comTimeOut.WriteTotalTimeoutConstant = 16000;
  SetCommTimeouts(hCOMPort,&comTimeOut);

  if (!SetupComm(hCOMPort,PORT_BUFFER_COUNT,PORT_BUFFER_COUNT)) return HAL_ERROR_FATAL;

  GetCommState(hCOMPort, &structDCB);

  structDCB.BaudRate = Speed;
  structDCB.Parity = 0;
  structDCB.StopBits = 0;
  structDCB.ByteSize = 8;
  structDCB.fDtrControl = DTR_CONTROL_DISABLE;
  structDCB.fRtsControl = RTS_CONTROL_ENABLE;

  SetCommState(hCOMPort, &structDCB);

  PurgeComm(hCOMPort,PURGE_RXCLEAR|PURGE_TXCLEAR);

  return HAL_ERROR_OK;
}


int HAL_OutData(unsigned char *Data, int Count)
{
  DWORD dwCount; OVERLAPPED ov ={0};

  if (WriteFile(hCOMPort,Data,Count,&dwCount,&ov) == 0)
    return HAL_ERROR_IO;

  if (dwCount != Count)
    return HAL_ERROR_IO;

  return HAL_ERROR_OK;
}

int HAL_InData(unsigned char *Data, int Count)
{
  DWORD dwCount; OVERLAPPED ov ={0};

  if (ReadFile(hCOMPort,Data,Count,&dwCount,&ov) == 0)
    return HAL_ERROR_IO;

  if (dwCount != Count)
    return HAL_ERROR_IO;

  return HAL_ERROR_OK;
}

int HAL_GetInDataCount()
{
  Sleep(100);
  DWORD iErrors;
  COMSTAT cmsStat;
  ClearCommError(hCOMPort, &iErrors, &cmsStat);
  return cmsStat.cbInQue;
}

void HAL_ResetIn()
{
  Sleep(100);
  PurgeComm(hCOMPort,PURGE_RXCLEAR|PURGE_TXCLEAR);
}

int HAL_Close()
{
  CloseHandle(hCOMPort);
  return HAL_ERROR_OK;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#pragma package(smart_init)
