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

#include "sensor.h"
#include "menu.h"

//f-code 3,4를 처리하는 함수
int readSensor(uint8_t slave_addr, uint8_t function_code, uint16_t reg_addr, uint16_t wLength, ModbusFrame *response)
{
    ModbusFrame request;

    request.slave_addr = slave_addr;        // Modbus slave address
    request.function_code = function_code;  // Read Holding Registers
    request.data[0] = reg_addr >> 8;        // Starting address (MSB)
    request.data[1] = reg_addr;             // Starting address (LSB)
    request.data[2] = wLength >> 8;         // Number of registers to read (MSB)
    request.data[3] = wLength;              // Number of registers to read (LSB)
    request.data_length = 4;                // number of header bytes 

    transmitModbusRequest(&request);        //send data via rs485 line

    response->slave_addr = request.slave_addr;
    response->function_code = request.function_code;
    return receiveModbusResponse(response);
}

//AIRFLOW Sensor function
int getAIRFLOW_UnitVersion(ModbusFrame *response)
{
    return readSensor(AIRFLOW_ADDRESS, FCODE_READ_INPUT_REGISTER, UNIT_VERSION, UNIT_VERSION_LENGTH, response);
}
int getAIRFLOW_Serial(ModbusFrame *response)
{
    return readSensor(AIRFLOW_ADDRESS, FCODE_READ_INPUT_REGISTER, UNIT_SERIAL, UNIT_SERIAL_LENGTH, response);
}

int getAIRFLOW_UnitCodeVersion(ModbusFrame *response)
{
    return readSensor(AIRFLOW_ADDRESS, FCODE_READ_INPUT_REGISTER, UNIT_CODE_VERSION, UNIT_CODE_VERSION_LENGTH, response);
}

int getAIRFLOW_U_AXIS(ModbusFrame *response)
{
    return readSensor(AIRFLOW_ADDRESS, FCODE_READ_INPUT_REGISTER, AIRFLOW_U_AXIS_REG, REG_LENGTH2, response);
}

int getAIRFLOW_V_AXIS(ModbusFrame *response)
{
    return readSensor(AIRFLOW_ADDRESS, FCODE_READ_INPUT_REGISTER, AIRFLOW_V_AXIS_REG, REG_LENGTH2, response);
}

int getAIRFLOW_V_AXIS_ABS(ModbusFrame *response)
{
    return readSensor(AIRFLOW_ADDRESS, FCODE_READ_INPUT_REGISTER, AIRFLOW_V_AXIS_ABS_REG, REG_LENGTH2, response);
}

int getAIRFLOW_U_AXIS_ABS(ModbusFrame *response)
{
    return readSensor(AIRFLOW_ADDRESS, FCODE_READ_INPUT_REGISTER, AIRFLOW_U_AXIS_ABS_REG, REG_LENGTH2, response);
}

int getAIRFLOW_TEMP(ModbusFrame *response)
{
    return readSensor(AIRFLOW_ADDRESS, FCODE_READ_INPUT_REGISTER, AIRFLOW_TEMP_REG, REG_LENGTH2, response);
}

int getAIRFLOW_Relay(ModbusFrame *response)
{
    return readSensor(AIRFLOW_ADDRESS, FCODE_READ_INPUT_REGISTER, AIRFLOW_RELAY_REG, REG_LENGTH1, response);
}

int getAIRFLOW_Status1(ModbusFrame *response)
{
    return readSensor(AIRFLOW_ADDRESS, FCODE_READ_INPUT_REGISTER, AIRFLOW_STATUS1_REG, REG_LENGTH1, response);
}

int getAIRFLOW_Status2(ModbusFrame *response)
{
    return readSensor(AIRFLOW_ADDRESS, FCODE_READ_INPUT_REGISTER, AIRFLOW_STATUS2_REG, REG_LENGTH1, response);
}

//VICONOX Sensor function
int getVICONOX_UnitVersion(ModbusFrame *response)
{
    return readSensor(VICONOX_ADDRESS, FCODE_READ_INPUT_REGISTER, UNIT_VERSION, UNIT_VERSION_LENGTH, response);
}

int getVICONOX_Serial(ModbusFrame *response)
{
    return readSensor(VICONOX_ADDRESS, FCODE_READ_INPUT_REGISTER, UNIT_SERIAL, UNIT_SERIAL_LENGTH, response);
}

int getVICONOX_UnitCodeVersion(ModbusFrame *response)
{
    return readSensor(VICONOX_ADDRESS, FCODE_READ_INPUT_REGISTER, UNIT_CODE_VERSION, UNIT_CODE_VERSION_LENGTH, response);
}

int getVICONOX_VI(ModbusFrame *response)
{
    return readSensor(VICONOX_ADDRESS, FCODE_READ_INPUT_REGISTER, VICONOX_VI_REG, REG_LENGTH2, response);
}

int getVICONOX_CO(ModbusFrame *response)
{
    return readSensor(VICONOX_ADDRESS, FCODE_READ_INPUT_REGISTER, VICONOX_CO_REG, REG_LENGTH2, response);
}

int getVICONOX_TEMP(ModbusFrame *response)
{
    return readSensor(VICONOX_ADDRESS, FCODE_READ_INPUT_REGISTER, VICONOX_TEMP_REG, REG_LENGTH2, response);
}

int getVICONOX_HUMI(ModbusFrame *response)
{
    return readSensor(VICONOX_ADDRESS, FCODE_READ_INPUT_REGISTER, VICONOX_HUMI_REG, REG_LENGTH2, response);
}

int getVICONOX_NO(ModbusFrame *response)
{
    return readSensor(VICONOX_ADDRESS, FCODE_READ_INPUT_REGISTER, VICONOX_NO_REG, REG_LENGTH2, response);
}

int getVICONOX_NO2(ModbusFrame *response)
{
    return readSensor(VICONOX_ADDRESS, FCODE_READ_INPUT_REGISTER, VICONOX_NO2_REG, REG_LENGTH2, response);
}

int getVICONOX_NOX(ModbusFrame *response)
{
    return readSensor(VICONOX_ADDRESS, FCODE_READ_INPUT_REGISTER, VICONOX_NOX_REG, REG_LENGTH2, response);
}

int getVICONOX_Relay(ModbusFrame *response)
{
    return readSensor(VICONOX_ADDRESS, FCODE_READ_INPUT_REGISTER, VICONOX_RELAY_REG, REG_LENGTH1, response);
}

int getVICONOX_Status1(ModbusFrame *response)
{
    return readSensor(VICONOX_ADDRESS, FCODE_READ_INPUT_REGISTER, VICONOX_STATUS1_REG, REG_LENGTH1, response);
}

int getVICONOX_Status2(ModbusFrame *response)
{
    return readSensor(VICONOX_ADDRESS, FCODE_READ_INPUT_REGISTER, VICONOX_STATUS2_REG, REG_LENGTH1, response);
}

FunctionInfo_t RTU_fx[] = { {'O', getVICONOX_VI,         0, 0.0},   //0
                            {'X', getVICONOX_CO,         0, 0.0},   //1
                            {'X', getVICONOX_NO,         0, 0.0},   //2
                            {'X', getVICONOX_NO2,        0, 0.0},   //3
                            {'X', getVICONOX_NOX,        0, 0.0},   //4
                            {'O', getAIRFLOW_U_AXIS,     0, 0.0},   //5
                            {'X', getVICONOX_TEMP,       0, 0.0},   //6
                            {'X', getVICONOX_HUMI,       0, 0.0},   //7
                            {'X', getAIRFLOW_V_AXIS,     0, 0.0},   //8
                            {'X', getAIRFLOW_U_AXIS_ABS, 0, 0.0},   //9
                            {'X', getAIRFLOW_V_AXIS_ABS, 0, 0.0},   //10
                            {'X', getAIRFLOW_TEMP,       0, 0.0},   //11
                            {'X', getVICONOX_Status1,    0, 0.0},   //12
                            {'X', getVICONOX_Status2,    0, 0.0},   //13
                            {'O', getAIRFLOW_Status1,    0, 0.0},   //14
                            {'O', getAIRFLOW_Status2,    0, 0.0}    //15
};

//대표되는 폴링을 참조하여 하위 폴링 여부를 결정하는 플래그 세팅
void getSensorConfigForPollingAndDisplay(void)
{
    for(int i=0; i < OFFSET_APPLY_MEMBER; i++)
        RTU_fx[i].flag = mSensorConfig.cfg[i];

    if(RTU_fx[V_VI_POLLING_INDEX].flag == 'O') {  //followed by pre setting of VICONOX
        RTU_fx[V_ST1_POLLING_INDEX].flag = 'O';
        RTU_fx[V_ST2_POLLING_INDEX].flag = 'O';
        RTU_fx[V_TEMP_POLLING_INDEX].flag = 'O';
        RTU_fx[V_HUMI_POLLING_INDEX].flag = 'O';
    }    
    else {
        RTU_fx[V_ST1_POLLING_INDEX].flag = 'X';
        RTU_fx[V_ST2_POLLING_INDEX].flag = 'X';
        RTU_fx[V_TEMP_POLLING_INDEX].flag = 'X';
        RTU_fx[V_HUMI_POLLING_INDEX].flag = 'X';
        mMaskedEventData[0] = 0;    //for fault LED Process viconox st1
        mMaskedEventData[1] = 0;    //for fault LED Process viconox st2
    }

    if(RTU_fx[A_U_AXIS_POLLING_INDEX].flag == 'O') { //followed by pre setting of AIRFLOW
        RTU_fx[A_V_AXIS_POLLING_INDEX].flag = 'O';
        RTU_fx[A_U_AXIS_ABS_POLLING_INDEX].flag = 'O';
        RTU_fx[A_V_AXIS_ABS_POLLING_INDEX].flag = 'O';
        RTU_fx[A_ST1_POLLING_INDEX].flag = 'O';
        RTU_fx[A_ST2_POLLING_INDEX].flag = 'O';
        RTU_fx[A_TEMP_POLLING_INDEX].flag = 'O';
    }
    else {
        RTU_fx[A_V_AXIS_POLLING_INDEX].flag = 'X';
        RTU_fx[A_U_AXIS_ABS_POLLING_INDEX].flag = 'X';
        RTU_fx[A_V_AXIS_ABS_POLLING_INDEX].flag = 'X';
        RTU_fx[A_ST1_POLLING_INDEX].flag = 'X';
        RTU_fx[A_ST2_POLLING_INDEX].flag = 'X';
        RTU_fx[A_TEMP_POLLING_INDEX].flag = 'X';
        mMaskedEventData[2] = 0;    //for fault LED Process airflow st1
        mMaskedEventData[3] = 0;    //for fault LED Process airflow st2
    }
}

sl_sleeptimer_timer_handle_t sensorPollTimer;
uint8_t modbus_comm_flag = 0; 
static uint8_t busy = 0;

//rtu 와 폴링 완료후 대기 시간중임을 확인하는 함수
//통신 진행이 아닐때 수행해야 하는 다른 함수를 처리할 때 참고하는 함수
int isPollingRTUfree(void) {    
    return busy;
}

//unit status만 폴링이 완료 되었는지 확인하는 함수
//이벤트 처리를 위하여 최소한 한번은 폴링이 완료 되어야 한다.
static uint8_t unitStatusPollDone[4] = {0,0,0,0};
int isPollDone(int index) {
    return unitStatusPollDone[index];
}

//센서 설정에 따른 폴링 스케쥴러
//폴링하여 데이터를 개별 저장소에 저장한다 
void pollingRTU(void) {
    static uint32_t sequence = 0;
    ModbusFrame response;
    static uint32_t viconox_comm_err_counter = 0;
    static uint32_t airflow_comm_err_counter = 0;
    if(busy) return;
    // printBT("seq=%d", sequence);

    getSensorConfigForPollingAndDisplay();
    if (RTU_fx[sequence].flag=='O') {
        if(RTU_fx[sequence].function(&response)) {
            if(sequence == VICONOX_COMM_ERROR_CHECK) {
                // printBT("V COMM OK");
                viconox_comm_err_counter = 0;
                modbus_comm_flag &= ~(0x01); 
            }
            if(sequence == AIRFLOW_COMM_ERROR_CHECK) {
                // printBT("A COMM OK");
                airflow_comm_err_counter = 0;
                modbus_comm_flag &= ~(0x02); 
            }

            if(sequence < OFFSET_APPLY_MEMBER) {
                RTU_fx[sequence].value = calibrateSensorValue(copyArrayToUint32(response.data), RTU_fx[sequence].offset);  //save feedback value with offset calibration
            }
            else {
                RTU_fx[sequence].value = copyArrayToUint32(response.data);  //save feedback value
                if(sequence == V_ST1_POLLING_INDEX) unitStatusPollDone[0] = 1;  //POLLING DONE AT LEAST ON TIME
                if(sequence == V_ST2_POLLING_INDEX) unitStatusPollDone[1] = 1;  //POLLING DONE AT LEAST ON TIME
                if(sequence == A_ST1_POLLING_INDEX) unitStatusPollDone[2] = 1;  //POLLING DONE AT LEAST ON TIME
                if(sequence == A_ST2_POLLING_INDEX) unitStatusPollDone[3] = 1;  //POLLING DONE AT LEAST ON TIME
            }
            busy = 1;
            sl_sleeptimer_start_timer(&sensorPollTimer, sl_sleeptimer_ms_to_tick(SENSOR_POLLING_TIME), timerCallBack, &busy, 0, 0);
        }
        else {
            if(sequence == VICONOX_COMM_ERROR_CHECK) { 
                // printBT("V COMM ERR");

                viconox_comm_err_counter++;
                if(viconox_comm_err_counter >= 1) {
                    modbus_comm_flag |= 0x01; 
                }
            }
            if(sequence == AIRFLOW_COMM_ERROR_CHECK) {
                // printBT("A COMM ERR");
                airflow_comm_err_counter++;
                if(airflow_comm_err_counter >= 1) {
                    modbus_comm_flag |= 0x02; 
                }
            }

            busy = 1;
            sl_sleeptimer_start_timer(&sensorPollTimer, sl_sleeptimer_ms_to_tick(SENSOR_POLLING_TIME/5), timerCallBack, &busy, 0, 0);
        
            if((sequence >= V_ST1_POLLING_INDEX) && (sequence <= A_ST2_POLLING_INDEX)) {
                sequence++;
                sequence %= RTU_fx_MAX_NUMBER;
                return;
            }
            RTU_fx[sequence].value = NaN;   //value is Invalid
            sequence++;
            sequence %= RTU_fx_MAX_NUMBER;
            return;
        }
        // busy = 1;
        // sl_sleeptimer_start_timer(&sensorPollTimer, sl_sleeptimer_ms_to_tick(SENSOR_POLLING_TIME), timerCallBack, &busy, 0, 0);
    } 
    else {
        RTU_fx[sequence].value = 0;
        if(sequence == VICONOX_COMM_ERROR_CHECK) {
            viconox_comm_err_counter = 0;
            modbus_comm_flag &= ~(0x01); 
        }
        if(sequence == AIRFLOW_COMM_ERROR_CHECK) {
            airflow_comm_err_counter = 0;
            modbus_comm_flag &= ~(0x20); 
        }
    } 
    sequence++;
    sequence %= RTU_fx_MAX_NUMBER;
}



