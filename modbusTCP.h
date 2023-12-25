#ifndef MODBUS_TCP_H
#define MODBUS_TCP_H
#include "stdint.h"
#include "w5500.h"
#include "rs485.h"
#include "w5500_tcp.h"
#include "hc165.h"
#include "sensor.h"
#include "app.h"
#include "menu.h"

// Modbus TCP frame structure
typedef struct {
    uint16_t tid;
    uint16_t pid;
    uint16_t length;
    uint8_t  slave_addr;
    uint8_t  function_code;
    uint8_t  data[256]; // Adjust the buffer size as per your requirements
    uint16_t start_address;
} ModbusTcpFrame;

void transmitModbusTcpRespons(uint8_t sn, ModbusTcpFrame *response);
void transmitModbus485toTcp(uint8_t sn, ModbusFrame *response, uint16_t transID);
void transmitModbus485toTcpWriteMultipleRegisters(uint8_t sn, ModbusFrame *response, uint16_t transID);

void mergeVICONOXdata(ModbusFrame *response, uint8_t offset);
void mergeAIRFLOWdata(ModbusFrame *response, uint8_t offset);

#define SIU_F04_DATA_LENGTH 40  //bytes

#define DATA_OFFSET_00 0
#define DATA_OFFSET_40 40

void merge_VICONOX_AIRFLOW_Data(ModbusFrame *response);
uint32_t faultLEDprocess(void);

#endif