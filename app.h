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

#ifndef APP_H
#define APP_H
#include "sl_sleeptimer.h"
#include "at24c256.h"
#include "stdint.h"

#define DEBUG_PRINT_BT 0

void Debug_printBT(char *fmt, ...);

/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
void app_init(void);

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
void app_process_action(void);

int printBT(char *fmt, ...);

void timerCallBack(sl_sleeptimer_timer_handle_t *handle, void *data);
void printFloatBT(char *p);
void printHexBT(char *p);

#define BUTTON_PRESSED_EVENT 50
#define RTC_POLL_EVENT 60

extern remoteIpFrame gSERVERNETINFO;

#define TCP_HEADER_SIZE 7
#define MODBUS_TCP_HEADER_SIZE 6

#define MODBUS_PORT 502

#define MAX_BUFFER_SIZE 256 // 최대 저장 가능한 바이트 배열의 크기

typedef struct {
    sl_sleeptimer_timer_handle_t timer;
    uint8_t timeout;
} timeout_t;

typedef struct {
    unsigned char data[MAX_BUFFER_SIZE];
    size_t length; // 현재 저장된 데이터 길이
} ByteBuffer_t;

extern timeout_t modBusTime;
extern ByteBuffer_t byteBuffer;

extern uint8_t tid[2];

void modbusTcpTimeoutStart(uint32_t ms);
void setModbusTimeout(void);
uint8_t getModbusTimeout(void);
int receiveModbusTcpClient(void);
int receiveModbusTcpServer(void);
void TcpReceiveAndReply(uint8_t sn);

#define SIU_CLIENT_SOCKET 0
#define SIU_SERVER_SOCKET 1
#define SIU_SERVER_SOCKET_2 2

#define CLIENT_DATA_PUSH_TIME 1000 //ms
#define Write_Multiple_Registers 0x10

#define TCP_POLL_GUARD_TIME 100  //ms

void clientLEDprocess(void);

extern uint16_t mMaskedEventData[4]; //for fault Led Process

#endif // APP_H
