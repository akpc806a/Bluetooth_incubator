//---------------------------------------------------------------------------

#ifndef ModbusH
#define ModbusH

//---------------------------------------------------------------------------
#define MODBUS_ERROR_OK 0
#define MODBUS_ERROR_IO 1
#define MODBUS_ERROR_TIMEOUT 2
#define MODBUS_ERROR_CRC 3
#define MODBUS_ERROR_FORMAT 4
//---------------------------------------------------------------------------
int ModBus_WriteReg(unsigned char ID, int Addr, short int Data);
int ModBus_ReadReg(unsigned char ID, int Addr, short int *Data);
int ModBus_ReadReg_Req(unsigned char ID, int Addr);
int ModBus_ReadReg_Recv(unsigned char ID, short int *Data);
int ModBus_ReadRegs(unsigned char ID, int StartAddr, int Count, short int *Data);
//---------------------------------------------------------------------------
#endif
