#include "modbus.h"

//extern void HAL_Uart_put(uint8_t ch);
//extern uint8_t HAL_Uart_get();

//#define hal_uart_put(x) HAL_Uart_put(x)
//#define hal_uart_get() HAL_Uart_get()

void hal_uart_clear(void); // clear uart transmitter
void hal_uart_put(uint8_t data); // add data byte to uart TX
void hal_uart_send(void); // initiation of data transmission


const uint8_t CRCHI[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40
};

const uint8_t CRCLO[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
    0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
    0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
    0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
    0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
    0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
    0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
    0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
    0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
    0x40
}; 

// TODO: how about hash table? =)
// It should be no more than 255 (otherwise: TODO: re-implement loop variables)
#define MODBUS_MAX_NUM_MESSAGES 32

static struct
{
    uint8_t length;
    T_ModbusEntry entries[MODBUS_MAX_NUM_MESSAGES];
} entryList;

static uint8_t modbusID; // ID
static uint8_t modbusFunc; // last received function code

static uint8_t numWrBytes = 0; // number of write to write (for WRITE_MULTIPLE)
static uint16_t dataToWrite; // data to write

static uint16_t startAddr; // starting address of request
static uint16_t numPoints; // total amount of requested entries to read/write
static uint16_t numPointsCounter; // total amount of requested entries to read/write -- used as counter in state machine
static uint16_t receivedCRC; // received CRC to compare with calculated

static uint8_t ucHi = 0xff;
static uint8_t ucLo = 0xff;

void Clear_CRC16()
{
    ucHi = 0xff;
    ucLo = 0xff;
}

uint8_t Update_CRC16(uint8_t data)
{
    uint8_t ucIndex;
    
    ucIndex = ucHi ^ data;
    ucHi = ucLo ^ CRCHI[ucIndex];
    ucLo = CRCLO[ucIndex];
    
    return data;
}

uint16_t Get_CRC16()
{
    return (uint16_t)((ucHi<<8) | ucLo);
}

void Modbus_Init(uint8_t modbusID_)
{
    modbusID = modbusID_;
    
    // Initialize empty list of messages/fields
    entryList.length = 0;
}

void Modbus_SetID(uint8_t modbusID_)
{
    modbusID = modbusID_;
}


// if = 0, waiting for ID
// 1 -- waiting for function code
// 2 -- data address first byte
// 3 -- data address second byte
// 4 -- register count first byte
// 5 -- register count second byte
// 6 -- CRC first byte
// 7 -- CRC second byte
typedef enum
{
    MODBUS_STATE_ID = 0,
    MODBUS_STATE_FUNC_CODE,
    MODBUS_STATE_ADDR_H,
    MODBUS_STATE_ADDR_L,
    MODBUS_STATE_COUNT_H,
    MODBUS_STATE_COUNT_L,
    MODBUS_STATE_CRC_H,
    MODBUS_STATE_CRC_L,

    MODBUS_STATE_WR_BYTE_COUNT,
    MODBUS_STATE_WR_DATA_H,
    MODBUS_STATE_WR_DATA_L
} T_ModbusState;

static T_ModbusState modbusState = MODBUS_STATE_ID;


#define FUNC_READ_INPUT_REGISTERS	4
#define FUNC_READ_MULTIPLE_HOLDING_REGISTERS	3
#define FUNC_WRITE_SINGLE_HOLDING_REGISTER	6
#define FUNC_WRITE_MULTIPLE_HOLDING_REGISTERS	16


// search in the variable table and get actual data
uint16_t Modbus_Get_Data(uint16_t addr)
{
    uint8_t i;
    uint16_t iResult;
    for (i = 0; i < entryList.length; i++)
        if (entryList.entries[i].address == addr)
        {
            if (entryList.entries[i].pVariable)
            {
                if (entryList.entries[i].txCallback)
                	(*entryList.entries[i].txCallback)(entryList.entries[i].pVariable);
                return *(entryList.entries[i].pVariable);
            }
            else
            {
                if (entryList.entries[i].txCallback)
                	(*entryList.entries[i].txCallback)(&iResult);
                return iResult;
            }
        }
    
    return 0; // TODO: maybe return error code if we didnt get?
}


// search in the variable and store the temporary new value
void Modbus_Store_Data(uint16_t addr, uint16_t value)
{
    uint8_t i;
    for (i = 0; i < entryList.length; i++)
        if (entryList.entries[i].address == addr)
        {
            entryList.entries[i].new_value = value; // store the value temporary
            entryList.entries[i].req_update = true; // request update after CRC check
            return;
        }
}

// update all variables from temporary stored values (called if CRC matches)
void Modbus_Set_Data(uint16_t addr, uint16_t value)
{
    uint8_t i;
    for (i = 0; i < entryList.length; i++)
        if (entryList.entries[i].address == addr)
        {
            if (entryList.entries[i].pVariable)
                *entryList.entries[i].pVariable = value;

            if (entryList.entries[i].rxCallback)
            	(*entryList.entries[i].rxCallback)(value);

            return;
        }
}

// update all variables from temporary stored values (called if CRC matches)
void Modbus_Update_FromStored()
{
    uint8_t i;
    for (i = 0; i < entryList.length; i++)
        if (entryList.entries[i].req_update)
        {
        	if (entryList.entries[i].pVariable)
                *entryList.entries[i].pVariable = entryList.entries[i].new_value;

            if (entryList.entries[i].rxCallback)
            	(*entryList.entries[i].rxCallback)(entryList.entries[i].new_value);

            entryList.entries[i].req_update = false;
        }
}

// Modbus response for function codes 06 and 16 (data write)
void Modbus_Response_06_16(uint16_t start_addr, uint16_t num)
{
    uint16_t i = 0;
    uint16_t tx_data = 0;
    
    Clear_CRC16(); hal_uart_clear();

    // TODO: how about TX buffer overflow?
    
    // its not obvious, but Update_CRC16(x) returns x
    // so hal_uart_put(Update_CRC16(x)) -- updates CRC with x AND then pass x it UART
    hal_uart_put(Update_CRC16(modbusID));
    hal_uart_put(Update_CRC16(modbusFunc));

    hal_uart_put(Update_CRC16(start_addr >> 8));
    hal_uart_put(Update_CRC16(start_addr));

    if (modbusFunc == FUNC_WRITE_SINGLE_HOLDING_REGISTER)
    {
        hal_uart_put(Update_CRC16(dataToWrite >> 8));
        hal_uart_put(Update_CRC16(dataToWrite));
    }
    else
    {
        hal_uart_put(Update_CRC16(num >> 8));
        hal_uart_put(Update_CRC16(num));
    }

        
    // send CRC
    tx_data = Get_CRC16();
    hal_uart_put(Update_CRC16((tx_data & 0xFF00) >> 8));
    hal_uart_put(Update_CRC16(tx_data & 0x00FF));
    
    hal_uart_send();
}




void Modbus_Send(uint16_t start_addr, uint16_t num)
{
    uint16_t i = 0;
    uint16_t tx_data = 0;
    
    Clear_CRC16(); hal_uart_clear();

    // TODO: how about TX buffer overflow?
    
    // its not obvious, but Update_CRC16(x) returns x
    // so hal_uart_put(Update_CRC16(x)) -- updates CRC with x AND then pass x it UART
    hal_uart_put(Update_CRC16(modbusID));
    hal_uart_put(Update_CRC16(modbusFunc));
    hal_uart_put(Update_CRC16(num*2));

    for (i=0; i < num; i++)
    {
        tx_data = Modbus_Get_Data(start_addr + i); // get data for transmission

        hal_uart_put(Update_CRC16(tx_data >> 8));
        hal_uart_put(Update_CRC16(tx_data));
    }
        
    // send CRC
    tx_data = Get_CRC16();
    hal_uart_put(Update_CRC16((tx_data & 0xFF00) >> 8));
    hal_uart_put(Update_CRC16(tx_data & 0x00FF));
    
    hal_uart_send();
}


// Modbus response for function codes 03 and 04 (data read)
void Modbus_Response_03_04(uint16_t start_addr, uint16_t num)
{
    uint16_t i = 0;
    uint16_t tx_data = 0;

    Clear_CRC16(); hal_uart_clear();

    // TODO: how about TX buffer overflow?

    // its not obvious, but Update_CRC16(x) returns x
    // so hal_uart_put(Update_CRC16(x)) -- updates CRC with x AND then pass x it UART
    hal_uart_put(Update_CRC16(modbusID));
    hal_uart_put(Update_CRC16(modbusFunc));
    hal_uart_put(Update_CRC16(num*2));

    for (i=0; i < num; i++)
    {
      tx_data = Modbus_Get_Data(start_addr + i); // get data for transmission

      hal_uart_put(Update_CRC16(tx_data >> 8));
      hal_uart_put(Update_CRC16(tx_data));
    }

    // send CRC
    tx_data = Get_CRC16();
    hal_uart_put(Update_CRC16((tx_data & 0xFF00) >> 8));
    hal_uart_put(Update_CRC16(tx_data & 0x00FF));
    
    hal_uart_send();
}



// process single Modbus data byte
void Modbus_Process(uint8_t data)
{
  if (modbusState == MODBUS_STATE_ID)
  {
    if (data == modbusID) 
    {
      modbusState = MODBUS_STATE_FUNC_CODE;
      Clear_CRC16();
      Update_CRC16(data);
    }
  }
  else
  if (modbusState == MODBUS_STATE_FUNC_CODE)
  {
    if (data == FUNC_READ_INPUT_REGISTERS || data == FUNC_READ_MULTIPLE_HOLDING_REGISTERS || data == FUNC_WRITE_SINGLE_HOLDING_REGISTER || data == FUNC_WRITE_MULTIPLE_HOLDING_REGISTERS)
    {
      modbusState = MODBUS_STATE_ADDR_H;
      Update_CRC16(data);
      modbusFunc = data; // store Modbus function
    }
    else
      modbusState = MODBUS_STATE_ID;
  }
  else
  if (modbusState == MODBUS_STATE_ADDR_H)
  {
    startAddr = data << 8;
    Update_CRC16(data);
    modbusState = MODBUS_STATE_ADDR_L;
  }
  else
  if (modbusState == MODBUS_STATE_ADDR_L)
  {
    startAddr |= data;
    Update_CRC16(data);
    if (modbusFunc == FUNC_WRITE_SINGLE_HOLDING_REGISTER)
    {
      numPoints = 1;
      numPointsCounter = numPoints;

      modbusState = MODBUS_STATE_WR_DATA_H;
    }
    else
      modbusState = MODBUS_STATE_COUNT_H;
  }
  else
  if (modbusState == MODBUS_STATE_COUNT_H)
  {
    numPoints = data << 8;
    Update_CRC16(data);
    modbusState = MODBUS_STATE_COUNT_L;
  }
  else
  if (modbusState == MODBUS_STATE_COUNT_L)
  {
    numPoints |= data;
    numPointsCounter = numPoints;

    Update_CRC16(data);
    if (modbusFunc == FUNC_WRITE_MULTIPLE_HOLDING_REGISTERS)
      modbusState = MODBUS_STATE_WR_BYTE_COUNT;
    else
    if (modbusFunc == FUNC_WRITE_SINGLE_HOLDING_REGISTER)
      modbusState = MODBUS_STATE_WR_DATA_H;
    else
      modbusState = MODBUS_STATE_CRC_H;
  }
  else
  if (modbusState == MODBUS_STATE_CRC_H)
  {
    receivedCRC = data << 8;
    modbusState = MODBUS_STATE_CRC_L;
  }
  else
  if (modbusState == MODBUS_STATE_CRC_L)
  {
    receivedCRC |= data;
    // check CRC
    if (Get_CRC16() == receivedCRC)
    {
      if (modbusFunc == FUNC_READ_INPUT_REGISTERS || modbusFunc == FUNC_READ_MULTIPLE_HOLDING_REGISTERS)
        Modbus_Response_03_04(startAddr, numPoints); // send requested data
      else
      if (modbusFunc == FUNC_WRITE_SINGLE_HOLDING_REGISTER)
      {
        Modbus_Set_Data(startAddr, dataToWrite); // the last variable is not stored, update it directly
        Modbus_Response_06_16(startAddr, numPoints);
      }
      if (modbusFunc == FUNC_WRITE_MULTIPLE_HOLDING_REGISTERS)
      {
        Modbus_Update_FromStored(); // update stored variables before
        Modbus_Set_Data(startAddr, dataToWrite); // the last variable is not stored, update it directly
        Modbus_Response_06_16(startAddr, numPoints);
      }
    }
    //else
      //LED1_OFF();
    modbusState = MODBUS_STATE_ID;
  }
  else
  if (modbusState == MODBUS_STATE_WR_BYTE_COUNT)
  {
    numWrBytes = data;
    Update_CRC16(data);
    modbusState = MODBUS_STATE_WR_DATA_H;
  }
  else
  if (modbusState == MODBUS_STATE_WR_DATA_H)
  {
    dataToWrite = data << 8;
    Update_CRC16(data);
    modbusState = MODBUS_STATE_WR_DATA_L;
  }
  else
  if (modbusState == MODBUS_STATE_WR_DATA_L)
  {
    dataToWrite |= data;
    Update_CRC16(data);
    numPointsCounter--;
    
    if (numPointsCounter == 0)
      modbusState = MODBUS_STATE_CRC_H;
    else
    {
      // continue to receive data
      Modbus_Store_Data(startAddr, dataToWrite); // store data to write before CRC check
      startAddr++;
      modbusState = MODBUS_STATE_WR_DATA_H;
    }
  }
}

/*
bool Modbus_Update()
{
    uint8_t c;
    
    c = hal_uart_get();
    Modbus_Process(c);
    
    return false; 
}
*/

T_ModbusEntry* Modbus_CreateEntry(const char* name, uint16_t address, uint16_t* pVariable, T_Modbus_RxCallback rxCallback, T_Modbus_TxCallback txCallback)
{
    if (entryList.length >= MODBUS_MAX_NUM_MESSAGES)
    {
        // No more room!
        return NULL;
    }

    T_ModbusEntry* pNewEntry = &(entryList.entries[entryList.length]); //grab pointer to next free space

    if (pNewEntry == NULL)
    {
        // Uh-oh! Throw error.
        return NULL;
    }

    pNewEntry->name = name;
    pNewEntry->address = address;
    pNewEntry->req_update = false;

    pNewEntry->pVariable = pVariable;
    pNewEntry->rxCallback = rxCallback;
    pNewEntry->txCallback = txCallback;

    entryList.length++;
    
    return pNewEntry;
}


