/***************************************************************************//**
 * @file
 * @brief Application interface provided to main().
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#ifndef SENSOR_H
#define SENSOR_H
#include "rs485.h"
#include "modbusTCP.h"
#include "stdint.h"

#define REG_LENGTH1 1
#define REG_LENGTH2 2

#define UNIT_VERSION_LENGTH 13
#define UNIT_SERIAL_LENGTH  6
#define UNIT_CODE_VERSION_LENGTH 13 

#define VICONOX_ADDRESS 1

#define UNIT_VERSION        10000
#define UNIT_SERIAL         10013
#define UNIT_CODE_VERSION   10019
#define VICONOX_VI_REG      10500
#define VICONOX_CO_REG      10502
#define VICONOX_TEMP_REG    10504
#define VICONOX_HUMI_REG    10506
#define VICONOX_NO_REG      10508
#define VICONOX_NO2_REG     10510
#define VICONOX_NOX_REG     10512
//-------------------------------------------------------
#define VICONOX_RELAY_REG   10580
//                          "1234567890123456"
#define VICONOX_RELAY_BIT0  "RELAY1 ALARM/DV "
#define VICONOX_RELAY_BIT1  "RELAY2 ALARM/DV "
#define VICONOX_RELAY_BIT2  "RELAY3 ALARM/DV "
#define VICONOX_RELAY_BIT3  "RELAY4 ALARM/DV "
#define VICONOX_RELAY_BIT4  "RELAY5 ALARM/DV "
#define VICONOX_RELAY_BIT5  "SPARE           "
#define VICONOX_RELAY_BIT6  "SPARE           "
#define VICONOX_RELAY_BIT7  "SPARE           "
#define VICONOX_RELAY_BIT8  "RELAY1 RELAY    "
#define VICONOX_RELAY_BIT9  "RELAY2 RELAY    "
#define VICONOX_RELAY_BIT10 "RELAY3 RELAY    "
#define VICONOX_RELAY_BIT11 "RELAY4 RELAY    "
#define VICONOX_RELAY_BIT12 "RELAY5 RELAY    "
#define VICONOX_RELAY_BIT13 "SPARE           "
#define VICONOX_RELAY_BIT14 "SPARE           "
#define VICONOX_RELAY_BIT15 "SPARE           "
//-------------------------------------------------------
#define VICONOX_STATUS1_REG 10590
//                            "1234567890123456"
#define VICONOX_STATUS1_BIT0  "OVERRIDE ACTIVE "
#define VICONOX_STATUS1_BIT1  "MEMORY FAULT    "
#define VICONOX_STATUS1_BIT2  "SETUP ACTIVE    "
#define VICONOX_STATUS1_BIT3  "RTD FAULT       "
#define VICONOX_STATUS1_BIT4  "TEMP O.O.R      "
#define VICONOX_STATUS1_BIT5  "HS FAULT        "
#define VICONOX_STATUS1_BIT6  "COMMS CRC FAULT "
#define VICONOX_STATUS1_BIT7  "HI-NO2 OFFSET   "
#define VICONOX_STATUS1_BIT8  "STEPPER MOTER F "
#define VICONOX_STATUS1_BIT9  "CHOPPER SPEED E "
#define VICONOX_STATUS1_BIT10 "HI IR SIGNAL    "
#define VICONOX_STATUS1_BIT11 "LO IR SIGNAL    "
#define VICONOX_STATUS1_BIT12 "GAS WHEEL FAULT "
#define VICONOX_STATUS1_BIT13 "VIS FAULT       "
#define VICONOX_STATUS1_BIT14 "IR FAULT        "
#define VICONOX_STATUS1_BIT15 "TX NOT DETECTED "
//-------------------------------------------------------
#define VICONOX_STATUS2_REG 10591
//                            "1234567890123456"
#define VICONOX_STATUS2_BIT0  "HIGH HUMIDITY   "
#define VICONOX_STATUS2_BIT1  "SIGNAL SAT NOW  "
#define VICONOX_STATUS2_BIT2  "SIGNAL SATURATED"
#define VICONOX_STATUS2_BIT3  "DATA INVALID    "
#define VICONOX_STATUS2_BIT4  "ALARM ACTIVE    "
#define VICONOX_STATUS2_BIT5  "HI MEAS-VIS     "
#define VICONOX_STATUS2_BIT6  "HI REF-VIS/NO2  "
#define VICONOX_STATUS2_BIT7  "LO REF-VIS/NO2  "
#define VICONOX_STATUS2_BIT8  "HI MEAS-NO2     "
#define VICONOX_STATUS2_BIT9  "HI REF GAIN     "
#define VICONOX_STATUS2_BIT10 "MAINTENANCE MODE"
#define VICONOX_STATUS2_BIT11 "HIGH OPACITY    "
#define VICONOX_STATUS2_BIT12 "GAS WHEEL FAULT "
#define VICONOX_STATUS2_BIT13 "DIRTY OPTICS    "
#define VICONOX_STATUS2_BIT14 "STABILISING     "
#define VICONOX_STATUS2_BIT15 "CO/NO BEAM BLOCK"

#define AIRFLOW_ADDRESS 2
#define AIRFLOW_U_AXIS_REG       10500
#define AIRFLOW_V_AXIS_REG       10502
#define AIRFLOW_U_AXIS_ABS_REG   10504
#define AIRFLOW_V_AXIS_ABS_REG   10506
#define AIRFLOW_TEMP_REG         10508

//-------------------------------------------------------
#define AIRFLOW_RELAY_REG   10580
//                          "1234567890123456"
#define AIRFLOW_RELAY_BIT0  "RELAY1 ALARM/DV "
#define AIRFLOW_RELAY_BIT1  "RELAY2 ALARM/DV "
#define AIRFLOW_RELAY_BIT2  "RELAY3 ALARM/DV "
#define AIRFLOW_RELAY_BIT3  "RELAY4 ALARM/DV "
#define AIRFLOW_RELAY_BIT4  "RELAY5 ALARM/DV "
#define AIRFLOW_RELAY_BIT5  "SPARE           "
#define AIRFLOW_RELAY_BIT6  "SPARE           "
#define AIRFLOW_RELAY_BIT7  "SPARE           "
#define AIRFLOW_RELAY_BIT8  "RELAY1 RELAY    "
#define AIRFLOW_RELAY_BIT9  "RELAY2 RELAY    "
#define AIRFLOW_RELAY_BIT10 "RELAY3 RELAY    "
#define AIRFLOW_RELAY_BIT11 "RELAY4 RELAY    "
#define AIRFLOW_RELAY_BIT12 "RELAY5 RELAY    "
#define AIRFLOW_RELAY_BIT13 "SPARE           "
#define AIRFLOW_RELAY_BIT14 "SPARE           "
#define AIRFLOW_RELAY_BIT15 "SPARE           "
//-------------------------------------------------------
#define AIRFLOW_STATUS1_REG 10590
//                            "1234567890123456"
#define AIRFLOW_STATUS1_BIT0  "SPARE           "
#define AIRFLOW_STATUS1_BIT1  "SPARE           "
#define AIRFLOW_STATUS1_BIT2  "SPARE           "
#define AIRFLOW_STATUS1_BIT3  "MEMORY FAULT    "
#define AIRFLOW_STATUS1_BIT4  "TEMP O.O.R      "
#define AIRFLOW_STATUS1_BIT5  "RTD FAULT       "
#define AIRFLOW_STATUS1_BIT6  "Gill Checksum E "
#define AIRFLOW_STATUS1_BIT7  "Gill Timeout E  "
#define AIRFLOW_STATUS1_BIT8  "Gill internal E "
#define AIRFLOW_STATUS1_BIT9  "COMMS CRC FAULT "
#define AIRFLOW_STATUS1_BIT10 "SPARE           "
#define AIRFLOW_STATUS1_BIT11 "SPARE           "
#define AIRFLOW_STATUS1_BIT12 "GAS WHEEL FAULT "
#define AIRFLOW_STATUS1_BIT13 "VIS FAULT       "
#define AIRFLOW_STATUS1_BIT14 "SPARE           "
#define AIRFLOW_STATUS1_BIT15 "SPARE           "
//-------------------------------------------------------
#define AIRFLOW_STATUS2_REG 10591
//                            "1234567890123456"
#define AIRFLOW_STATUS2_BIT0  "SPARE           "
#define AIRFLOW_STATUS2_BIT1  "SPARE           "
#define AIRFLOW_STATUS2_BIT2  "SPARE           "
#define AIRFLOW_STATUS2_BIT3  "DATA INVALID    "
#define AIRFLOW_STATUS2_BIT4  "ALARM ACTIVE    "
#define AIRFLOW_STATUS2_BIT5  "SPARE           "
#define AIRFLOW_STATUS2_BIT6  "SPARE           "
#define AIRFLOW_STATUS2_BIT7  "SPARE           "
#define AIRFLOW_STATUS2_BIT8  "SPARE           "
#define AIRFLOW_STATUS2_BIT9  "SPARE           "
#define AIRFLOW_STATUS2_BIT10 "SPARE           "
#define AIRFLOW_STATUS2_BIT11 "SPARE           "
#define AIRFLOW_STATUS2_BIT12 "SPARE           "
#define AIRFLOW_STATUS2_BIT13 "SPARE           "
#define AIRFLOW_STATUS2_BIT14 "SPARE           "
#define AIRFLOW_STATUS2_BIT15 "SPARE           "

//Basic
int readSensor(uint8_t slave_addr, uint8_t function_code, uint16_t reg_addr, uint16_t wLength, ModbusFrame *respons);

#define MAX_POLLING_NUMBER   16
#define OFFSET_APPLY_MEMBER  6 //VI, CO, NO, NO2, NOX, U-AXIS

//AIRFLOW Sensor function
int getAIRFLOW_UnitVersion(ModbusFrame *response);
int getAIRFLOW_Serial(ModbusFrame *response);
int getAIRFLOW_UnitCodeVersion(ModbusFrame *response);
int getAIRFLOW_U_AXIS(ModbusFrame *response);
int getAIRFLOW_V_AXIS(ModbusFrame *response);
int getAIRFLOW_U_AXIS_ABS(ModbusFrame *response);
int getAIRFLOW_V_AXIS_ABS(ModbusFrame *response);
int getAIRFLOW_TEMP(ModbusFrame *response);
int getAIRFLOW_Relay(ModbusFrame *response);
int getAIRFLOW_Status1(ModbusFrame *response);
int getAIRFLOW_Status2(ModbusFrame *response);

//VICONOX Sensor function
int getVICONOX_UnitVersion(ModbusFrame *response);
int getVICONOX_Serial(ModbusFrame *response);
int getVICONOX_UnitCodeVersion(ModbusFrame *response);
int getVICONOX_VI(ModbusFrame *response);
int getVICONOX_CO(ModbusFrame *response);
int getVICONOX_NO(ModbusFrame *response);
int getVICONOX_NOX(ModbusFrame *response);
int getVICONOX_TEMP(ModbusFrame *response);
int getVICONOX_HUMI(ModbusFrame *response);
int getVICONOX_Relay(ModbusFrame *response);
int getVICONOX_Status1(ModbusFrame *response);
int getVICONOX_Status2(ModbusFrame *response);

typedef struct {
    char flag;
    int(*function)(ModbusFrame *);  //sensor function pointer
    uint32_t value;
    float offset;
} FunctionInfo_t;



extern FunctionInfo_t RTU_fx[];
extern sl_sleeptimer_timer_handle_t sensorPollTimer;

#define SENSOR_POLLING_TIME 100 //ms
void pollingRTU(void);

#define RTU_fx_MAX_NUMBER MAX_POLLING_NUMBER

// FunctionInfo_t RTU_fx[] = { {'X', getVICONOX_VI,         0, 0.0},   //0
//                             {'X', getVICONOX_CO,         0, 0.0},   //1
//                             {'X', getVICONOX_NO,         0, 0.0},   //2
//                             {'X', getVICONOX_NO2,        0, 0.0},   //3
//                             {'X', getVICONOX_NOX,        0, 0.0},   //4
//                             {'O', getAIRFLOW_U_AXIS,     0, 0.0},   //5
//                             {'X', getVICONOX_TEMP,       0, 0.0},   //6
//                             {'X', getVICONOX_HUMI,       0, 0.0},   //7
//                             {'X', getAIRFLOW_V_AXIS,     0, 0.0},   //8
//                             {'X', getAIRFLOW_U_AXIS_ABS, 0, 0.0},   //9
//                             {'X', getAIRFLOW_V_AXIS_ABS, 0, 0.0},   //10
//                             {'X', getAIRFLOW_TEMP,       0, 0.0},   //11
//                             {'X', getVICONOX_Status1,    0, 0.0},   //12
//                             {'X', getVICONOX_Status2,    0, 0.0},   //13
//                             {'O', getAIRFLOW_Status1,    0, 0.0},   //14
//                             {'O', getAIRFLOW_Status2,    0, 0.0}    //15
// };

//prefix V --> VICONOX
//prefix A --> AIRFLOW

#define V_VI_POLLING_INDEX     0
#define V_CO_POLLING_INDEX     1
#define V_NO_POLLING_INDEX     2
#define V_NO2_POLLING_INDEX    3
#define V_NOX_POLLING_INDEX    4
#define A_U_AXIS_POLLING_INDEX 5

#define V_TEMP_POLLING_INDEX 6
#define V_HUMI_POLLING_INDEX 7

#define A_V_AXIS_POLLING_INDEX      8
#define A_U_AXIS_ABS_POLLING_INDEX  9
#define A_V_AXIS_ABS_POLLING_INDEX 10
#define A_TEMP_POLLING_INDEX       11

#define V_ST1_POLLING_INDEX 12
#define V_ST2_POLLING_INDEX 13
#define A_ST1_POLLING_INDEX 14
#define A_ST2_POLLING_INDEX 15

#define VICONOX_COMM_ERROR_CHECK V_VI_POLLING_INDEX
#define AIRFLOW_COMM_ERROR_CHECK A_U_AXIS_POLLING_INDEX

void getSensorConfigForPollingAndDisplay(void);

extern uint8_t modbus_comm_flag; 

int isPollingRTUfree(void);
int isPollDone(int index);

#define NaN 0xFFFFFFFF  //Not a Number

#endif // SENSOR_H
