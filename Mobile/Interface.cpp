//---------------------------------------------------------------------------
//#define TOTAL_LOGGING

//#pragma hdrstop

//#include <stdio.h>
//#include <time.h>

#include "Interface.h"
#include <System.Bluetooth.hpp>
#include <queue>
//---------------------------------------------------------------------------
//HANDLE hCOMPort;
TBluetoothManager * FBluetoothManager;
TBluetoothDeviceList *FDiscoverDevices;
TBluetoothDeviceList * FPairedDevices = 0;
TBluetoothAdapter * FAdapter;
TBytes FData;
TBluetoothSocket * FSocket = 0;
int FDeviceItemIndex = -1;

String Msg = "";
const String ServiceName = "Basic Text Server";
const String ServiceGUI = "{00001101-0000-1000-8000-00805F9B34FB}";

#define TIMEOUT_MS 50
//---------------------------------------------------------------------------

int HAL_Init()
{
	try
    {
		//LabelServer->Text = ServiceName;
		//LabelClient->Text = "Client of " + ServiceName;
		FBluetoothManager = TBluetoothManager::Current;
		FAdapter = FBluetoothManager->CurrentAdapter;
		if (FBluetoothManager->ConnectionState == TBluetoothConnectionState::Connected)
        {
        	return HAL_ERROR_OK;
			//PairedDevices();
			//ComboBoxPaired->ItemIndex = 0;
			//FDeviceItemIndex = -1;
		}
	} catch (Exception &ex)
    {
		ShowMessage(ex.Message);
        return HAL_ERROR_FATAL;
	}

/*
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
*/

}

TBytes ToSend;
TBytes RxBuffer;
std::queue <System::Byte> ToReceive;

int HAL_OutData(unsigned char *Data, int Count)
{
  while ( !ToReceive.empty() ) ToReceive.pop();

  ToSend.Length = Count;
  for (int i = 0; i < Count; i++) 
  {
     ToSend[i] = Data[i];
  }
//ToSend.Length = 1;
//ToSend[0] = 'v';
    if (FSocket != NULL && FSocket->Connected)
    {
        try
        {
            FSocket->SendData(ToSend);
        }
        catch (Exception &ex)
        {
            return HAL_ERROR_IO;
        }
        return HAL_ERROR_OK;
    }
    else
        return HAL_ERROR_INIT;
/*
	
	ToSend.Length = 1;
    ToSend[0] = 'v';
  
*/
/*
  DWORD dwCount; OVERLAPPED ov ={0};

  if (WriteFile(hCOMPort,Data,Count,&dwCount,&ov) == 0)
    return HAL_ERROR_IO;

  if (dwCount != Count)
    return HAL_ERROR_IO;
*/
  
}

int HAL_InData(unsigned char *Data, int Count)
{
    int iTimeOut = 0; // 1 ms timeouts = read conts
    int iByte = 0;
    //return 0;
    if (FSocket && FSocket->Connected)
    {
        while (iByte < Count)
        {
            if (iTimeOut > 0)
            {
                Sleep(1); // wait 1 ms if this is secnd try
            }
            iTimeOut++;
            if (iTimeOut > TIMEOUT_MS)
            {
                return HAL_ERROR_TIMEOUT;
            }

            try
            {
                RxBuffer = FSocket->ReadData();
            }
            catch (Exception &ex)
            {
                return HAL_ERROR_IO;
            }

            for (int j = 0; j < RxBuffer.Length; j++)
            {
                if (iByte < Count)
                {
                    Data[iByte] = RxBuffer[j];
                    iByte++;
                }
            }


        }
    }

  return HAL_ERROR_OK;
}

int HAL_GetInDataCount()
{
  return 0;
}

void HAL_ResetIn()
{

}

int HAL_Close()
{
  //CloseHandle(hCOMPort);
  return HAL_ERROR_OK;
}

void HAL_GetDeviceList(TStringList* device_list)
{
	device_list->Clear();
	FPairedDevices = FBluetoothManager->GetPairedDevices();
	if(FPairedDevices->Count > 0) {
		for(int i = 0; i < FPairedDevices->Count; i++) {
			device_list->Add(FPairedDevices->Items[i]->DeviceName +
            " : " + FPairedDevices->Items[i]->Address );
        }
	}
}

int HAL_Connect(int dev_index)
{
    if (dev_index < 0 || dev_index >= FPairedDevices->Count) 
        return HAL_ERROR_PARAM;  // index out of bounds

    if (FPairedDevices == 0)
        return HAL_ERROR_INIT; // not initialized yet


    TBluetoothDevice * LDevice = FPairedDevices->Items[dev_index];
    FSocket = LDevice->CreateClientSocket(StringToGUID(ServiceGUI), false);
    if (FSocket != NULL) 
    {
        FDeviceItemIndex = dev_index;
        FSocket->Connect();
        return HAL_ERROR_OK;
    }
    else 
    {
        ShowMessage("Out of time ~15s~");
        return HAL_ERROR_IO;
    }
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

//#pragma package(smart_init)
