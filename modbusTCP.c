#include "modbusTCP.h"
#include "event_log.h"
#include "sl_simple_led_instances.h"
#include "app.h"
// Transmit Modbus response
void transmitModbusTcpRespons(uint8_t sn, ModbusTcpFrame *response) {
    uint8_t frame[256]; // Adjust the buffer size as per your requirements
    int frame_length = 0;
    frame[frame_length++] = 0;  //TID
    frame[frame_length++] = 0;  //TID
    frame[frame_length++] = 0;  //PID
    frame[frame_length++] = 0;  //PID
    frame[frame_length++] = response->length << 8;
    frame[frame_length++] = response->length+2; //slave address + function code + data
    frame[frame_length++] = response->slave_addr;
    frame[frame_length++] = response->function_code;
    frame[frame_length++] = response->length;

    for (int i = 0; i < response->length; i++) {
        frame[frame_length++] = response->data[i];
    }
    // sendtoServer(sn, frame, frame_length);
    sendTo(sn, frame, frame_length);
}

void transmitModbus485toTcp(uint8_t sn, ModbusFrame *response, uint16_t transID) {
    uint8_t frame[256]; // Adjust the buffer size as per your requirements
    int frame_length = 0;
    frame[frame_length++] = transID>>8;  //TID
    frame[frame_length++] = transID;     //TID
    frame[frame_length++] = 0;  //PID
    frame[frame_length++] = 0;  //PID
    frame[frame_length++] = (response->data_length+3) << 8;
    frame[frame_length++] = (response->data_length+3); //slave address + function code + data
    frame[frame_length++] = response->slave_addr;
    frame[frame_length++] = response->function_code;
    frame[frame_length++] = response->data_length;

    for (int i = 0; i < response->data_length; i++) {
        frame[frame_length++] = response->data[i];
    }
    sendTo(sn, frame, frame_length);
}

void transmitModbus485toTcpWriteMultipleRegisters(uint8_t sn, ModbusFrame *response, uint16_t transID) {
    uint8_t frame[256]; // Adjust the buffer size as per your requirements
    int frame_length = 0;
    frame[frame_length++] = transID>>8;  //TID
    frame[frame_length++] = transID;     //TID
    frame[frame_length++] = 0;  //PID
    frame[frame_length++] = 0;  //PID
    frame[frame_length++] = (response->data_length+7) << 8;
    frame[frame_length++] = (response->data_length+7); //slave address:1 + function code:1 + start addr:2 + words:2 + datalen:1 + payload:xxx
    frame[frame_length++] = response->slave_addr;
    frame[frame_length++] = response->function_code;
    frame[frame_length++] = mNetworkConfig.modbus[REMOTE_MODEBUS_REGISTER_ADDRESS_INDEX] >> 8;
    frame[frame_length++] = mNetworkConfig.modbus[REMOTE_MODEBUS_REGISTER_ADDRESS_INDEX];
    frame[frame_length++] = ((response->data_length/2) << 8);
    frame[frame_length++] = (response->data_length/2);
    frame[frame_length++] = response->data_length;

    for (int i = 0; i < response->data_length; i++) {
        frame[frame_length++] = response->data[i];
    }
    sendTo(sn, frame, frame_length);
}

void mergeVICONOXdata(ModbusFrame *response, uint8_t offset) {
    uint32_to_bytearray(RTU_fx[V_VI_POLLING_INDEX].value,   &response->data[0 +offset]);
    uint32_to_bytearray(RTU_fx[V_CO_POLLING_INDEX].value,   &response->data[4 +offset]);
    uint32_to_bytearray(RTU_fx[V_TEMP_POLLING_INDEX].value, &response->data[8 +offset]);
    uint32_to_bytearray(RTU_fx[V_HUMI_POLLING_INDEX].value, &response->data[12+offset]);
    uint32_to_bytearray(RTU_fx[V_NO_POLLING_INDEX].value,   &response->data[16+offset]);
    uint32_to_bytearray(RTU_fx[V_NO2_POLLING_INDEX].value,  &response->data[20+offset]);
    uint32_to_bytearray(RTU_fx[V_NOX_POLLING_INDEX].value,  &response->data[24+offset]);
  
    response->data[28+offset] = (RTU_fx[V_ST1_POLLING_INDEX].value >> 24) & 0xFF;
    response->data[29+offset] = (RTU_fx[V_ST1_POLLING_INDEX].value >> 16) & 0xFF;

    response->data[30+offset] = (RTU_fx[V_ST2_POLLING_INDEX].value >> 24) & 0xFF;
    response->data[31+offset] = (RTU_fx[V_ST2_POLLING_INDEX].value >> 16) & 0xFF;

    for(int i=32+offset; i < 38+offset; i++) {    //ZERO PADDING
        response->data[i] = 0;    
    }
    response->data[38+offset] = modbus_comm_flag;
    response->data[39+offset] = pack_DI_bytes();
}

void mergeAIRFLOWdata(ModbusFrame *response, uint8_t offset) {
    uint32_to_bytearray(RTU_fx[A_U_AXIS_POLLING_INDEX].value,      &response->data[0+offset]);
    uint32_to_bytearray(RTU_fx[A_V_AXIS_POLLING_INDEX].value,      &response->data[4+offset]);
    uint32_to_bytearray(RTU_fx[A_U_AXIS_ABS_POLLING_INDEX].value,  &response->data[8+offset]);
    uint32_to_bytearray(RTU_fx[A_V_AXIS_ABS_POLLING_INDEX].value, &response->data[12+offset]);
    uint32_to_bytearray(RTU_fx[A_TEMP_POLLING_INDEX].value,       &response->data[16+offset]);

    response->data[20+offset] = (RTU_fx[A_ST1_POLLING_INDEX].value >> 24) & 0xFF;
    response->data[21+offset] = (RTU_fx[A_ST1_POLLING_INDEX].value >> 16) & 0xFF;

    response->data[22+offset] = (RTU_fx[A_ST2_POLLING_INDEX].value >> 24) & 0xFF;
    response->data[23+offset] = (RTU_fx[A_ST2_POLLING_INDEX].value >> 16) & 0xFF;

    for(int i=24+offset; i < 38+offset; i++) {    //ZERO PADDING
        response->data[i] = 0;    
    }
    response->data[38+offset] = modbus_comm_flag;
    response->data[39+offset] = pack_DI_bytes();
}

void merge_VICONOX_AIRFLOW_Data(ModbusFrame *response) {
    mergeVICONOXdata(response, DATA_OFFSET_00);
    mergeAIRFLOWdata(response, DATA_OFFSET_40);
}


uint32_t faultLEDprocess(void) {
    bool fault = false;
    uint32_t result=0;
    int eventIndex[6]={0,0,0,0,0,0};
    uint16_t mMergedEventData[6]={0,0,0,0,0,0};   //0:VICONOX ST1&MASK, 1:VICONOX ST2&MASK, 2:AIRFLOW ST1&MASK, 3:AIRFLOW ST2&MASK, 4:COMM ERROR, 5:DI 
    mMergedEventData[0] = mMaskedEventData[0];
    mMergedEventData[1] = mMaskedEventData[1];
    mMergedEventData[2] = mMaskedEventData[2];
    mMergedEventData[3] = mMaskedEventData[3];
    mMergedEventData[4] = modbus_comm_flag;
    mMergedEventData[5] = pack_DI_bytes();

    for(int i=0; i<6; i++) {
        if(mMergedEventData[i]) { 
            fault = true;
            eventIndex[i] = i + 1;  // prevent 0
        }
    }

    //fault LED output
    if(fault) sl_led_turn_on(&sl_led_fault);
    else sl_led_turn_off(&sl_led_fault);

    //save event index
    for (int i = 0; i < 6; ++i) {
        result |= (uint32_t)(eventIndex[i]) << (4 * (5 - i));
    }
    return result;  //return error index 1=VICONOX ST1, ....5=DI
}

