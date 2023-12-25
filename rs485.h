#ifndef RS485_H
#define RS485_H

#include "stdint.h"
#include <stdbool.h>
#include "sl_iostream_handles.h"
#include "em_gpio.h"
#include "sl_udelay.h"
#include <string.h>  
#include "sl_sleeptimer.h"
#include "app.h"

#define ECHO_ON 1
#define ECHO_OFF 0

#define MODBUS_TIMEOUT 100  //ms

#define FCODE_READ_INPUT_REGISTER 4

void rs485Receive(uint8_t *p, int len);
void flushrs485Buffer(int flush_count);
void rs485Send(uint8_t *p, int len, uint8_t echo);
void flushrs485(void);
void rs485Init(void);

// Modbus RTU frame structure
typedef struct {
    uint8_t slave_addr;
    uint8_t function_code;
    uint8_t data[256]; // Adjust the buffer size as per your requirements
    uint16_t data_length;
    uint16_t crc;
} ModbusFrame;

uint16_t calculateCRC16(uint8_t *data, uint16_t length);
int transmitModbusRequest(ModbusFrame *request);
int receiveModbusResponse(ModbusFrame *response);

uint32_t copyArrayToUint32(const unsigned char* array); //for float
float convertToFloat(uint32_t value);
float arrayToFloat(const unsigned char* array);

extern timeout_t rs485time;

void kickOffTimeout(uint32_t ms);
void setTimeout(void);
int getTimeout(void);

uint32_t floatToUint32(float value);
uint32_t calibrateSensorValue(uint32_t value, float offset);

void uint32_to_bytearray(uint32_t value, uint8_t* array);
void uint16_to_bytearray(uint16_t value, uint8_t* array);

#endif