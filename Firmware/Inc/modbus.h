#ifndef MODBUS_H
#define MODBUS_H

#include <stdint.h>

#define bool uint8_t
#define true 1
#define false (!true)
#define NULL 0

typedef enum
{
    MODBUS_FIELD_TYPE_RESERVED = 0,
    MODBUS_FIELD_TYPE_SIG,
    MODBUS_FIELD_TYPE_MEM,
    MODBUS_FIELD_TYPE_FUNC_TX,
    MODBUS_FIELD_TYPE_FUNC_RX,
    MODBUS_FIELD_TYPE_FAULT,

} __attribute__ ((__packed__)) T_Modbus_FieldType;


typedef void (*T_Modbus_TxCallback) (uint16_t* dataOut);
typedef void (*T_Modbus_RxCallback) (uint16_t dataIn);

/**
 * Describes the context a field making up the payload data of a message.
 * Many fields can be within a given message. Each field is linked to a
 * variable in software via an accessor funtion pointer.
 */
struct Struct_ModbusEntry
{    
    T_Modbus_FieldType type;
    const char* name;      // String describing message
    uint32_t address;      // Modbus address
    
    uint16_t new_value; // double bufferization for writing operation
    bool req_update; // request update variable from new_value

    T_Modbus_RxCallback rxCallback;
    T_Modbus_TxCallback txCallback;
    uint16_t* pVariable;
};


void Modbus_Init(uint8_t modbusID_);
void Modbus_SetID(uint8_t modbusID_);

void Modbus_Process(uint8_t data);
    
    

typedef struct Struct_ModbusEntry  T_ModbusEntry;

/**
 * Construct a new message with the given parameters, and add it to the LL.
 * @param name
 * @param msgId
 * @param portID
 * @param txPeriod_ms (0ms = one-shot)
 * @param txOffset_ms
 * @param opts
 * @return  Pointer to new message object
 */
T_ModbusEntry* Modbus_CreateEntry(const char* name, uint16_t address, uint16_t* pVariable, T_Modbus_RxCallback rxCallback, T_Modbus_TxCallback txCallback);





    
    
#endif
