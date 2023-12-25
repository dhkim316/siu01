/***************************************************************************//**
 * @file
 * @brief Core application logic.
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
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include "em_common.h"
#include "app_assert.h"
#include "sl_bluetooth.h"
#include "gatt_db.h"
#include "wizchip_conf.h"
#include "sl_udelay.h"
#include "w5500_tcp.h"
#include "socket.h"
#include "nvm3.h"
#include "nvm3_hal_flash.h"
// #include "sl_iostream_handles.h"
#include "sl_simple_led_instances.h"

#include "app.h"
#include "lcd.h"
#include "isl1208.h"
#include "hc165.h"
// #include "at24c256.h"
// #include "rs485.h"
#include "modbusTCP.h"
#include "menu.h"

// #include "sl_sleeptimer.h"

#include "sensor.h"
// #include "app_log.h"
#define app_log_info(...)  //disable log message
#define app_log(...)  //disable log message

#include "event_log.h"
int printBT(char *fmt, ...);

//ij electron utility
remoteIpFrame gSERVERNETINFO = { .ip = { 192,168,1,99},
                                  .port = 9000 };                            
// static void send_spp_data();
// const uint8_t serviceUUID[16] = {0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x01, 0x00, 0x40, 0x6e};
// const uint8_t charUUID[16]    = {0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x03, 0x00, 0x40, 0x6e};
void parse_cmd(char *p);


// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;

static sl_sleeptimer_timer_handle_t buttonPollTimer;
static sl_sleeptimer_timer_handle_t rtcPollTimer;

static sl_sleeptimer_timer_handle_t clientDataPushTimer;

static sl_sleeptimer_timer_handle_t tcpPollGuardTimer;
static uint8_t tcpPollBusy;

void TcpPollGuardTimeStart(void) {
  tcpPollBusy = 1;
  sl_sleeptimer_start_timer(&tcpPollGuardTimer, sl_sleeptimer_ms_to_tick(TCP_POLL_GUARD_TIME), timerCallBack, &tcpPollBusy, 0, 0);
}

int isTcpPollBusy(void) {
  return tcpPollBusy;
}

void timerCallBack(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  (void)data;
  // (void)timer;
  if(handle == &buttonPollTimer) {
    btnAndDiPoll();
    switch(m_menu.operationMode){
      case OPERATION_MODE_SHOPPING:
        menuProcess();
        break;
      case OPERATION_MODE_SETTING:
        setProcess();
        break;
    }
    autoReturnToHome();
    clientLEDprocess();
  }
  if(handle == &rtcPollTimer) sl_bt_external_signal(RTC_POLL_EVENT);
  if(handle == &rs485time.timer) setTimeout();
  if(handle == &modBusTime.timer) setModbusTimeout();
  if(handle == &sensorPollTimer) *(uint8_t *)data = 0;      //clear busy
  if(handle == &clientDataPushTimer) *(uint8_t *)data = 0;  //clear busy
  if(handle == &tcpPollGuardTimer) *(uint8_t *)data = 0;    //clear tcpPollBusy
}

/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
SL_WEAK void app_init(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once during start-up.                                    //
  /////////////////////////////////////////////////////////////////////////////
  // nvm3_open(nvm3_defaultHandle, nvm3_defaultInit);
  // nvm3_readData(nvm3_defaultHandle,0x44000,(char *)&gWIZNETINFO, sizeof(gWIZNETINFO));
  //sl_led_turn_on(&sl_led_connect);  //display socket connect

  // NVIC_SystemReset();

  lcd_init();
  // GPIO_PinModeSet(gpioPortA, 0, gpioModePushPull, 1); //LCD BACK-LIGHT ON

  if(read_hc165() == 0x8F) {  //set,enter,esc 동시누르고
    printf_lcd(1,1, "SIU-01");
    printf_lcd(1,2, "Initialize...");
    saveDefultConfig();
    sl_udelay_wait(1000000);  //wait 1 second
  } 
  else {
    loadSystemConfig();
  }

  flushrs485();
  rs485Init();
  flushrs485();

  w5500_spi_init();
  w5500_ip_init();

  // sl_led_turn_on(&sl_led_lidarpower);
  // sl_led_turn_on(&sl_led_connect);
  // sl_led_turn_on(&sl_led_disconnect);

  // You have to insert this code before TCP client test function execute.
  // wiz_NetTimeout gWIZNETTIME = {.retry_cnt = 3,         //RCR = 3
  //                                .time_100us = 2000};     //RTR = 2000
  wiz_NetTimeout gWIZNETTIME = {.retry_cnt = 1, 
                                .time_100us = 5};  //500usec

  ctlnetwork(CN_SET_TIMEOUT,(void*)&gWIZNETTIME);
  ctlnetwork(CN_GET_TIMEOUT, (void*)&gWIZNETTIME);  
  // lcd_init();
  // printf_lcd(1,1, "SIU-01");
  // printf_lcd(1,2, "WELCOME");
  isl1208_i2c_set_sr(0x10);
  // loadNetworkConfig();
  initMenu();

  // viUnitDisplay(1,1);  //10^3 m^3

 // sl_sleeptimer_start_timer(&iso7816.bulkInTimer, sl_sleeptimer_ms_to_tick(50), timerCallBack, (void *)NULL, 0, 0);
  sl_sleeptimer_start_periodic_timer(&buttonPollTimer, sl_sleeptimer_ms_to_tick(20), timerCallBack, (void *)NULL, 0, 0); 
  sl_sleeptimer_start_periodic_timer(&rtcPollTimer, sl_sleeptimer_ms_to_tick(333), timerCallBack, (void *)NULL, 0, 0); 

  // sl_led_turn_on(&sl_led_power);
  // sl_led_turn_on(&sl_led_fault);
  // sl_led_turn_on(&sl_led_comm);
  // sl_udelay_wait(100000);  //wait uicro second for comm led test
  // sl_led_turn_off(&sl_led_comm);
  // initPrs();
  initUnitStatus();
  // loadEventLogCounter();
}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
uint8_t eth_connected = 0;
uint8_t server1_connected = 0;
uint8_t server2_connected = 0;
uint8_t client_connected = 0;

uint16_t mMaskedEventData[4] = {0,0,0,0}; //for fault Led Process V-ST1, V-ST2, A-ST1, A-ST2

void clearMaskedEventData(void) {
  for(int i=0; i<4; i++) mMaskedEventData[i] = 0;
}

#define DEBUG_IJ

SL_WEAK void app_process_action(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application code here!                              //
  // This is called infinitely.                                              //
  // Do not call blocking functions from here!                               //
  /////////////////////////////////////////////////////////////////////////////
  int32_t ret;

  ethUpdate();   //update network parameter
  
  if(wizphy_getphylink()) {
    eth_connected = 1;
    // sl_led_turn_on(&sl_led_connect);  //display eth connect

    if(mModeConfig.cfg[CLIENT_INDEX] == 'O') {
      receiveModbusTcpClient();
    }
    receiveModbusTcpServer();
  }
  else {
    close(SIU_CLIENT_SOCKET);
    close(SIU_SERVER_SOCKET);
    close(SIU_SERVER_SOCKET_2);
    eth_connected = 0;
    server1_connected = 0;
    server2_connected = 0;
    client_connected = 0;
    // sl_led_turn_off(&sl_led_connect);
  }
  if(!isTcpPollBusy()) {
    pollingRTU();
  }

  if(isPollingRTUfree()) {
    uint16_t Zero2One; 
    clearMaskedEventData();
    if(isPollDone(VICONOX_ST1_LOG_INDEX)) {
      Zero2One = extractEvent(mUnitStatus.st[VICONOX_ST1_LOG_INDEX], RTU_fx[V_ST1_POLLING_INDEX].value >> 16);
      if(Zero2One) {
        mDateTimeEventData.year = rtcTime.tm_year;
        mDateTimeEventData.month = rtcTime.tm_mon-1;
        mDateTimeEventData.day = rtcTime.tm_mday-1;
        mDateTimeEventData.hour = rtcTime.tm_hour;
        mDateTimeEventData.minute = rtcTime.tm_min;
        mDateTimeEventData.index = VICONOX_ST1_LOG_INDEX;
        mDateTimeEventData.event = Zero2One;
        saveEventLogData();
      }
      mMaskedEventData[0] = (RTU_fx[V_ST1_POLLING_INDEX].value >> 16) & VICONOX_ST1_LOG_MASK;  //for fault led process
      mUnitStatus.st[VICONOX_ST1_LOG_INDEX] = RTU_fx[V_ST1_POLLING_INDEX].value >> 16;
    }

    if(isPollDone(VICONOX_ST2_LOG_INDEX)) {
      Zero2One = extractEvent(mUnitStatus.st[VICONOX_ST2_LOG_INDEX], RTU_fx[V_ST2_POLLING_INDEX].value >> 16);
      if(Zero2One) {
        mDateTimeEventData.year = rtcTime.tm_year;
        mDateTimeEventData.month = rtcTime.tm_mon-1;
        mDateTimeEventData.day = rtcTime.tm_mday-1;
        mDateTimeEventData.hour = rtcTime.tm_hour;
        mDateTimeEventData.minute = rtcTime.tm_min;
        mDateTimeEventData.index = VICONOX_ST2_LOG_INDEX;
        mDateTimeEventData.event = Zero2One;
        saveEventLogData();
      }
      mMaskedEventData[1] = (RTU_fx[V_ST2_POLLING_INDEX].value >> 16) & VICONOX_ST2_LOG_MASK;  //for fault led process
      mUnitStatus.st[VICONOX_ST2_LOG_INDEX] = RTU_fx[V_ST2_POLLING_INDEX].value >> 16;
    }

    if(isPollDone(AIRFLOW_ST1_LOG_INDEX)) {
      Zero2One = extractEvent(mUnitStatus.st[AIRFLOW_ST1_LOG_INDEX], RTU_fx[A_ST1_POLLING_INDEX].value >> 16);
      if(Zero2One) {
        mDateTimeEventData.year = rtcTime.tm_year;
        mDateTimeEventData.month = rtcTime.tm_mon-1;
        mDateTimeEventData.day = rtcTime.tm_mday-1;
        mDateTimeEventData.hour = rtcTime.tm_hour;
        mDateTimeEventData.minute = rtcTime.tm_min;
        mDateTimeEventData.index = AIRFLOW_ST1_LOG_INDEX;
        mDateTimeEventData.event = Zero2One;
        saveEventLogData();
      }
      mMaskedEventData[2] = (RTU_fx[A_ST1_POLLING_INDEX].value >> 16) & AIRFLOW_ST1_LOG_MASK;  //for fault led process
      mUnitStatus.st[AIRFLOW_ST1_LOG_INDEX] = RTU_fx[A_ST1_POLLING_INDEX].value >> 16;
    }

    if(isPollDone(AIRFLOW_ST2_LOG_INDEX)) {
      Zero2One = extractEvent(mUnitStatus.st[AIRFLOW_ST2_LOG_INDEX], RTU_fx[A_ST2_POLLING_INDEX].value >> 16);
      if(Zero2One) {
        mDateTimeEventData.year = rtcTime.tm_year;
        mDateTimeEventData.month = rtcTime.tm_mon-1;
        mDateTimeEventData.day = rtcTime.tm_mday-1;
        mDateTimeEventData.hour = rtcTime.tm_hour;
        mDateTimeEventData.minute = rtcTime.tm_min;
        mDateTimeEventData.index = AIRFLOW_ST2_LOG_INDEX;
        mDateTimeEventData.event = Zero2One;
        saveEventLogData();
      }
      mMaskedEventData[3] = (RTU_fx[A_ST2_POLLING_INDEX].value >> 16) & AIRFLOW_ST2_LOG_MASK;  //for fault led process
      mUnitStatus.st[AIRFLOW_ST2_LOG_INDEX] = RTU_fx[A_ST2_POLLING_INDEX].value >> 16;
    }
    uint32_t evtVal = faultLEDprocess();
    // printBT("Evt=%04X %02X", evtVal, modbus_comm_flag);
    // printBT("Evt=%04X %04X %04X %04X, ", mMaskedEventData[0], mMaskedEventData[1], mMaskedEventData[2], mMaskedEventData[3]);
    // sl_udelay_wait(100000);
  }
}

/*Main states */
#define DISCONNECTED  0
#define SCANNING      1
#define FIND_SERVICE  2
#define FIND_CHAR     3
#define ENABLE_NOTIF  4
#define DATA_MODE     5
#define DISCONNECTING 6

#define STATE_ADVERTISING 1
#define STATE_CONNECTED   2
#define STATE_SPP_MODE    3


static uint8_t _conn_handle = 0xFF;
static uint8_t _main_state;
static uint32_t _service_handle;
static uint16_t _char_handle;

static uint8_t _max_packet_size = 20;
static uint8_t _min_packet_size = 20;  // Target minimum bytes for one packet

static void reset_variables()
{
  _conn_handle = 0xFF;
  _main_state = STATE_ADVERTISING;
  _service_handle = 0;
  _char_handle = 0;
}

static uint8_t btMac[5];
/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  bd_addr address;
  uint8_t address_type;
  uint8_t system_id[8];

  uint16_t max_mtu_out;

  switch (SL_BT_MSG_ID(evt->header)) {

    case sl_bt_evt_system_boot_id:
      reset_variables();


      // Extract unique ID from BT Address.
      sc = sl_bt_system_get_identity_address(&address, &address_type);
      app_assert_status(sc);

      memcpy(btMac, address.addr, 6);  //save address

#if 0
      // Pad and reverse unique ID to get System ID.
      system_id[0] = address.addr[5];
      system_id[1] = address.addr[4];
      system_id[2] = address.addr[3];
      system_id[3] = 0xFF;
      system_id[4] = 0xFE;
      system_id[5] = address.addr[2];
      system_id[6] = address.addr[1];
      system_id[7] = address.addr[0];

      sc = sl_bt_gatt_server_write_attribute_value(gattdb_system_id,
                                                   0,
                                                   sizeof(system_id),
                                                   system_id);
      app_assert_status(sc);
#endif

      sc = sl_bt_gatt_server_set_max_mtu(251, &max_mtu_out);
      app_assert_status(sc);

      // Create an advertising set.
      sc = sl_bt_advertiser_create_set(&advertising_set_handle);
      app_assert_status(sc);

      // Set advertising interval to 100ms.
      // sc = sl_bt_advertiser_set_timing(
      //   advertising_set_handle,
      //   160, // min. adv. interval (milliseconds * 1.6)
      //   160, // max. adv. interval (milliseconds * 1.6)
      //   0,   // adv. duration
      //   0);  // max. num. adv. events

      // Set advertising interval to 100ms.
      sc = sl_bt_advertiser_set_timing(
        advertising_set_handle,
        160*2, // min. adv. interval (milliseconds * 1.6)  // approx. 200ms
        160*2, // max. adv. interval (milliseconds * 1.6)  // approx. 200ms
        0,   // adv. duration
        0);  // max. num. adv. events

      app_assert_status(sc);
      // Start general advertising and enable connections.
      sc = sl_bt_advertiser_start(
        advertising_set_handle,
        sl_bt_advertiser_general_discoverable,
        sl_bt_advertiser_connectable_scannable);
      app_assert_status(sc);
      break;

    case sl_bt_evt_connection_opened_id:
      _conn_handle = evt->data.evt_connection_opened.connection;
      app_log_info("Connection opened^^\r\n");

      /* Request connection parameter update.
       * conn.interval min 20ms, max 40ms, slave latency 4 intervals,
       * supervision timeout 2 seconds
       * (These should be compliant with Apple Bluetooth Accessory Design Guidelines, both R7 and R8) */
      // sl_bt_connection_set_parameters(_conn_handle, 24, 40, 0, 200, 0, 0xFFFF);
      break;

    case sl_bt_evt_connection_parameters_id:
      app_log_info("Conn.parameters: interval %u units, txsize %u\r\n", evt->data.evt_connection_parameters.interval, evt->data.evt_connection_parameters.txsize);
      break;

    case sl_bt_evt_gatt_mtu_exchanged_id:
      /* Calculate maximum data per one notification / write-without-response, this depends on the MTU.
       * up to ATT_MTU-3 bytes can be sent at once  */
      _max_packet_size = evt->data.evt_gatt_mtu_exchanged.mtu - 3;
      _min_packet_size = _max_packet_size; /* Try to send maximum length packets whenever possible */
      app_log_info("MTU exchanged: %d\r\n", evt->data.evt_gatt_mtu_exchanged.mtu);
      break;

    case sl_bt_evt_connection_closed_id:
      // printStats(&_sCounters);
      // if (STATE_SPP_MODE == _main_state){
      //   sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
      // }
      reset_variables();
      // Restart advertising after client has disconnected.
      sl_bt_advertiser_start(advertising_set_handle,
                                  advertiser_general_discoverable,
                                  advertiser_connectable_scannable);
      break;

    case sl_bt_evt_gatt_server_characteristic_status_id:
      {
        sl_bt_evt_gatt_server_characteristic_status_t char_status;
        char_status = evt->data.evt_gatt_server_characteristic_status;

        if (char_status.characteristic == gattdb_spp_data_tx) {
          // if (char_status.status_flags == gatt_server_client_config) {
          //   // Characteristic client configuration (CCC) for spp_data has been changed
          //   if (char_status.client_config_flags == gatt_notification) {
          //     _main_state = STATE_SPP_MODE;
          //     sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
          //     app_log_info("SPP Mode ON\r\n");
          //     }
          //   else {
          //     app_log_info("SPP Mode OFF\r\n");
          //     _main_state = STATE_CONNECTED;
          //     sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
          //     }
          //   }
          }
      }
      break;

    case sl_bt_evt_gatt_server_attribute_value_id:
    {
      parse_cmd(evt->data.evt_gatt_server_attribute_value.value.data);
    }
    break;

    case sl_bt_evt_gatt_server_user_write_request_id:
      if (evt->data.evt_gatt_server_user_write_request.characteristic
          == gattdb_ota_control) {
            // app_log("ota request\n\r", evt->data.evt_system_external_signal.extsignals);
          }

    break;

    case sl_bt_evt_system_soft_timer_id:
      // if (evt->data.evt_system_soft_timer.handle == LIDAR_SOFT_TIMER) {
      //   sl_led_turn_on(&sl_led_lidarpower);
      // }
    break;

    case  sl_bt_evt_system_external_signal_id:
      if(evt->data.evt_system_external_signal.extsignals == RTC_POLL_EVENT) {
        timeDisplay();
      }
    default:
      break;
  }
}

// int printBT(char *fmt, ...)
// {
//   uint8_t psbuf[128];
//   int result;
//   va_list args;
//   if( _conn_handle == 0xFF) return 0;
//   va_start(args, fmt);
//   result = vsprintf((char *)psbuf, fmt, args);
//   va_end(args);
//   do {
//     result = sl_bt_gatt_server_send_notification(_conn_handle, gattdb_spp_data_tx, strlen((const char*)psbuf), psbuf);
//   } while(result == SL_STATUS_NO_MORE_RESOURCE);
//   return result;
// }

int vprintBT(char *fmt, va_list args) {
    uint8_t psbuf[128];
    int result;
    if(_conn_handle == 0xFF) return 0;

    result = vsprintf((char *)psbuf, fmt, args);

    do {
        result = sl_bt_gatt_server_send_notification(_conn_handle, gattdb_spp_data_tx, strlen((const char*)psbuf), psbuf);
    } while(result == SL_STATUS_NO_MORE_RESOURCE);

    return result;
}

int printBT(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int result = vprintBT(fmt, args);
    va_end(args);
    return result;
}

void Debug_printBT(char *fmt, ...) {
#if DEBUG_PRINT_BT
    va_list args;
    va_start(args, fmt);
    vprintBT(fmt, args);
    va_end(args);
#endif
}

void showNetInfo(void)
{
  printBT("=========================");
  printBT("IP=%03d.%03d.%03d.%03d",gWIZNETINFO.ip[0],gWIZNETINFO.ip[1],gWIZNETINFO.ip[2],gWIZNETINFO.ip[3]);
  printBT("GW=%03d.%03d.%03d.%03d",gWIZNETINFO.gw[0],gWIZNETINFO.gw[1],gWIZNETINFO.gw[2],gWIZNETINFO.gw[3]);
  printBT("SN=%03d.%03d.%03d.%03d",gWIZNETINFO.sn[0],gWIZNETINFO.sn[1],gWIZNETINFO.sn[2],gWIZNETINFO.sn[3]);
  printBT("SERVER_IP=%03d.%03d.%03d.%03d",gSERVERNETINFO.ip[0],gSERVERNETINFO.ip[1],gSERVERNETINFO.ip[2],gSERVERNETINFO.ip[3]);
  printBT("PORT=%d",gSERVERNETINFO.port);
  printBT("MAC=%02X:%02X:%02X:%02X:%02X:%02X",gWIZNETINFO.mac[0],gWIZNETINFO.mac[1],gWIZNETINFO.mac[2],gWIZNETINFO.mac[3],gWIZNETINFO.mac[4],gWIZNETINFO.mac[5]);
  if(eth_connected) printBT("EtherNet CONNECTED");
  else printBT("EtherNet DISCONNECTED");
  if(server1_connected) printBT("Server1 CONNECTED");
  if(server2_connected) printBT("Server2 CONNECTED");
  if(client_connected) printBT("Client CONNECTED");
  printBT("=========================");
  printBT(" ");
}

void parse_cmd(char *p)
{
  char *s = p;
  while (*s) {
    *s = toupper((unsigned char) *s);
    s++;
  }
  *s=0;

  if(strncmp("VER", p, 3) == 0) {
    printBT(DEVICE__INFO___VER);
    return;
  }

  if(strncmp("?", p, 1) == 0) {
    printBT("==< Settings >==");
    printBT("IP n.n.n.n  => ip");
    printBT("GW n.n.n.n  => gateway");
    printBT("SN n.n.n.n  => subnet");
    printBT("SV n.n.n.n  => server ip");
    printBT("PO n        => port");
    printBT("NET         => SHOW NETWORK STATUS");
    printBT("DEF         => SET DEFAULT");
    printBT("REBOOT      => REBOOT the DEVICE");
    return;
  }

  if(strncmp("NET", p, 3) == 0) {
    showNetInfo();
    return;
  }

  // nvm3_open(nvm3_defaultHandle, nvm3_defaultInit);

  if(strncmp("IP ", p, 3) == 0) {
    uint8_t ip[4];
    sscanf(&p[3],"%hhd.%hhd.%hhd.%hhd", &ip[0], &ip[1], &ip[2], &ip[3]);
    memcpy(gWIZNETINFO.ip, ip, 4);
    // nvm3_writeData(nvm3_defaultHandle,0x44000,(char *)&gWIZNETINFO, sizeof(gWIZNETINFO));

    memcpy(&mNetworkConfig.eth, &gWIZNETINFO, sizeof(wiz_NetInfo));
    saveSystemConfig();
    showNetInfo();
    return;
  }
  if(strncmp("SN ", p, 3) == 0) {
    uint8_t ip[4];
    sscanf(&p[3],"%hhd.%hhd.%hhd.%hhd", &ip[0], &ip[1], &ip[2], &ip[3]);
    memcpy(gWIZNETINFO.sn, ip, 4);
    // nvm3_writeData(nvm3_defaultHandle,0x44000,(char *)&gWIZNETINFO, sizeof(gWIZNETINFO));
    memcpy(&mNetworkConfig.eth, &gWIZNETINFO, sizeof(wiz_NetInfo));
    saveSystemConfig();
    showNetInfo();
  }
  if(strncmp("GW ", p, 3) == 0) {
    uint8_t ip[4];
    sscanf(&p[3],"%hhd.%hhd.%hhd.%hhd", &ip[0], &ip[1], &ip[2], &ip[3]);
    memcpy(gWIZNETINFO.gw, ip, 4);
//    nvm3_writeData(nvm3_defaultHandle,0x44000,(char *)&gWIZNETINFO, sizeof(gWIZNETINFO));
    memcpy(&mNetworkConfig.eth, &gWIZNETINFO, sizeof(wiz_NetInfo));
    saveSystemConfig();
    showNetInfo();
  }
  
  if(strncmp("SV ", p, 3) == 0) {
    uint8_t ip[4];
    sscanf(&p[3],"%hhd.%hhd.%hhd.%hhd", &ip[0], &ip[1], &ip[2], &ip[3]);
    memcpy(gSERVERNETINFO.ip, ip, 4);
    // nvm3_writeData(nvm3_defaultHandle,0x44001,(char *)&gSERVERNETINFO, sizeof(gSERVERNETINFO));
    memcpy(mNetworkConfig.remote_ip, gSERVERNETINFO.ip, sizeof(4));
    saveSystemConfig();
    showNetInfo();
  }

  if(strncmp("PO ", p, 3) == 0) {	//port seting
    uint16_t port;
    sscanf(&p[3],"%hd",  &port);
    gSERVERNETINFO.port = port;
    // nvm3_writeData(nvm3_defaultHandle,0x44001,(char *)&gSERVERNETINFO, sizeof(gSERVERNETINFO));
    mNetworkConfig.port = gSERVERNETINFO.port;
    saveSystemConfig();
    showNetInfo();
  }

  if(strncmp("MAC", p, 3) == 0) {
    gWIZNETINFO.mac[0]=btMac[5];
    gWIZNETINFO.mac[1]=btMac[4];
    gWIZNETINFO.mac[2]=btMac[3];
    gWIZNETINFO.mac[3]=btMac[2];
    gWIZNETINFO.mac[4]=btMac[1];
    gWIZNETINFO.mac[5]=btMac[0];
    // nvm3_writeData(nvm3_defaultHandle,0x44000,(char *)&gWIZNETINFO, sizeof(gWIZNETINFO));
    memcpy(&mNetworkConfig.eth, &gWIZNETINFO, sizeof(wiz_NetInfo));
    saveSystemConfig();
    showNetInfo();
  }

  if(strncmp("DEF", p, 3) == 0) {
    saveDefultConfig();
    showNetInfo();
  }

  if(strncmp("REBOOT", p, 5) == 0) {	//port seting
    // nvm3_close(nvm3_defaultHandle);
    sl_bt_system_stop_bluetooth();
    sl_bt_system_reset(0);
  }

  if(strncmp("ST", p, 2) == 0) {		//set time STyymmddhhmmss
    for(int i=2; i<13; i++) {
      if((p[i] >= '0') && (p[i] <= '9')) continue;
      else return;
    }
    struct rtc_time stime;
    stime.tm_year = (p[2] - '0') * 10 + (p[3] - '0');
    stime.tm_mon  = (p[4] - '0') * 10 + (p[5] - '0');
    stime.tm_mday = (p[6] - '0') * 10 + (p[7] - '0');
    stime.tm_wday = 1;
    stime.tm_hour = (p[8] - '0') * 10 + (p[9] - '0');
    stime.tm_min  = (p[10] - '0') * 10 + (p[11] - '0');
    stime.tm_sec  = (p[12] - '0') * 10 + (p[13] - '0');
    isl1208_i2c_set_time(&stime);
    sl_udelay_wait(1000);
    printBT("OK");
    return;
  }

  if(strncmp("OFVI", p, 4) == 0) {
    float ;
    RTU_fx[0].offset = atof(&p[4]);
    saveOffsetToEEPROM();
    printBT("VI offset %0.2f", RTU_fx[0].offset);
    return;
  }


  if(strncmp("OFCO", p, 4) == 0) {
    float ;
    RTU_fx[1].offset = atof(&p[4]);
    saveOffsetToEEPROM();
    printBT("CO offset %0.2f", RTU_fx[1].offset);
    return;
  }

  if(strncmp("OFNO2", p, 5) == 0) {
    float ;
    RTU_fx[3].offset = atof(&p[5]);
    saveOffsetToEEPROM();
    printBT("NO2 offset %0.2f", RTU_fx[3].offset);
    return;
  }

  if(strncmp("OFNOX", p, 5) == 0) {
    float ;
    RTU_fx[4].offset = atof(&p[5]);
    saveOffsetToEEPROM();
    printBT("NOX offset %0.2f", RTU_fx[4].offset);
    return;
  }


  if(strncmp("OFNO", p, 4) == 0) {
    float ;
    RTU_fx[2].offset = atof(&p[4]);
    saveOffsetToEEPROM();
    printBT("NO offset %0.2f", RTU_fx[2].offset);
    return;
  }

  if(strncmp("OFVEL", p, 5) == 0) {
    float ;
    RTU_fx[5].offset = atof(&p[5]);
    saveOffsetToEEPROM();
    printBT("Velocity offset %0.2f", RTU_fx[5].offset);
    return;
  }

  if(strncmp("OFFSET", p, 6) == 0) {
    printBT("VI offset %0.2f", RTU_fx[0].offset);
    printBT("CO offset %0.2f", RTU_fx[1].offset);
    printBT("NO2 offset %0.2f", RTU_fx[3].offset);
    printBT("NOX offset %0.2f", RTU_fx[4].offset);
    printBT("NO offset %0.2f", RTU_fx[2].offset);
    printBT("Velocity offset %0.2f", RTU_fx[5].offset);
    return;
  }

  if(strncmp("PASSCODELV1=", p, 12) == 0) {
    strncpy(mPassCode.level1, &p[12], 4);
    writeEEPROM(EEP_ADDR_PASSWORD, &mPassCode, sizeof(mPassCode));
    printBT("PASSCODE LV1 CHANGED");
    return;
  }

  if(strncmp("PASSCODELV2=", p, 12) == 0) {
    strncpy(mPassCode.level2, &p[12], 4);
    writeEEPROM(EEP_ADDR_PASSWORD, &mPassCode, sizeof(mPassCode));
    printBT("PASSCODE LV2 CHANGED");
    return;
  }

  if(strncmp("GT", p, 2) == 0) {
    struct rtc_time time;
    isl1208_i2c_read_time(&time);
    printBT("<%02d:%02d:%02d>", time.tm_hour, time.tm_min, time.tm_sec);
    printBT("<%04d/%02d/%02d>", time.tm_year+2000, time.tm_mon, time.tm_mday);
    return;
  }

  ModbusFrame response;

  if(strncmp("VI", p, 2) == 0){
    if(getVICONOX_VI(&response)) {
      printFloatBT(response.data);
      transmitModbus485toTcp(SIU_CLIENT_SOCKET, &response, 0);
      printBT("OK");
    }
    else {
      printBT("timeout");
    }
  }

  if(strncmp("CO", p, 2) == 0){
    if(getVICONOX_CO(&response)) {
      printFloatBT(response.data);
      transmitModbus485toTcp(SIU_CLIENT_SOCKET, &response, 0);
      printBT("OK");
    }
    else {
      printBT("timeout");
    }
  }

  if(strncmp("NO ", p, 3) == 0){
    if(getVICONOX_NO(&response)) {
      printFloatBT(response.data);
      transmitModbus485toTcp(SIU_CLIENT_SOCKET, &response, 0);
      printBT("OK");
    }
    else {
      printBT("timeout");
    }
  }

  if(strncmp("NO2", p, 3) == 0){
    if(getVICONOX_NO2(&response)) {
      printFloatBT(response.data);
      transmitModbus485toTcp(SIU_CLIENT_SOCKET, &response, 0);
      printBT("OK");
    }
    else {
      printBT("timeout");
    }
  }

  if(strncmp("NOX", p, 3) == 0) {
    if(getVICONOX_NOX(&response)) {
      printFloatBT(response.data);
      transmitModbus485toTcp(SIU_CLIENT_SOCKET, &response, 0);
      printBT("OK");
    }
    else {
      printBT("timeout");
    }
  }

  if(strncmp("VELOCITY", p, 8) == 0) {
    if(getAIRFLOW_U_AXIS(&response)) {
      printFloatBT(response.data);
      transmitModbus485toTcp(SIU_CLIENT_SOCKET, &response, 0);
      printBT("OK");
    }
    else {
      printBT("timeout");
    }
  }

  if(strncmp("EVENT", p, 5) == 0){
    loadEventLogCounter();
    printBT("EVENT %d %04X %04X %04X %04X" , mEventLogCounter, mUnitStatus.st[0], mUnitStatus.st[1], mUnitStatus.st[2], mUnitStatus.st[3]);
  }

  if(strncmp("ECLEAR", p, 6) == 0){
    clearAllEventLog();  
    printBT("CLEAR EVENT %d" , mEventLogCounter);
  }

  if(strncmp("EVIEW", p, 5) == 0){
    int index = atoi(&p[5]);
    
    DateTimeEventData_t eventData;
    loadEventLogData(index, &eventData);

    printBT("id=%d data=%04X", eventData.index, eventData.event);
    return;
  }


  if(strncmp("UV2", p, 3) == 0){  
    getAIRFLOW_UnitVersion(&response);
    transmitModbus485toTcp(SIU_CLIENT_SOCKET, &response, 0);
  }

  if(strncmp("IL", p, 2) == 0){
      printBT("bfbfdmsbfmdsbfmdsnbfmdsbfmndsbsdmnfb%d\n\r", _max_packet_size);
  }

  if(strncmp("US2", p, 3) == 0){  
    getAIRFLOW_Serial(&response);
    transmitModbus485toTcp(SIU_CLIENT_SOCKET, &response, 0);
  }

  if(strncmp("UCV2", p, 4) == 0){  
    getAIRFLOW_UnitCodeVersion(&response);
    transmitModbus485toTcp(SIU_CLIENT_SOCKET, &response, 0);
  }
  if(strncmp("MTU", p, 3) == 0){  
    printBT("MTU = %d", _max_packet_size);
    return;
  }
}

//input 4byte , enable float at compile option
void printFloatBT(char *p)
{
  char str[32];
  uint32_t value = copyArrayToUint32(p); //4-byte value
  float floatValue = convertToFloat(value);
  sprintf(str, "%0.2f", floatValue);
  printBT("%s", str);
}

void printHexBT(char *p)
{
  printBT("%02X%02X",p[0],p[1]);
  printBT("%04X",p[0]);
}

// 바이트 배열 초기화 함수
void initByteBuffer(ByteBuffer_t *buffer) {
    buffer->length = 0;
}

// 바이트 배열에 데이터 추가 함수
void appendToByteBuffer(ByteBuffer_t *buffer, unsigned char *data, size_t dataLength) {
    if (buffer->length + dataLength <= MAX_BUFFER_SIZE) {
        for (size_t i = 0; i < dataLength; i++) {
            buffer->data[buffer->length] = data[i];
            buffer->length++;
        }
    } else {
      initByteBuffer(buffer);
    }
}

timeout_t modBusTime;
ByteBuffer_t byteBuffer;

void modbusTcpTimeoutStart(uint32_t ms) {
  modBusTime.timeout = 0;
  sl_sleeptimer_start_timer(&modBusTime.timer, sl_sleeptimer_ms_to_tick(ms), timerCallBack, (void *)NULL, 0, 0);
} 

void setModbusTimeout(void)
{
  modBusTime.timeout = 1;
}

uint8_t getModbusTimeout(void)
{
  return modBusTime.timeout;
}

uint8_t tid[2];

int receiveModbusTcp(uint8_t sn)
{
  initByteBuffer(&byteBuffer);
  // TCP 헤더 수신
  uint8_t tcp_rcv[256];
  int bytes_received = rcvfrom(sn, tcp_rcv);
  if (bytes_received < TCP_HEADER_SIZE) return; // "Error receiving TCP header"


  //save tid for return
  tid[0] = tcp_rcv[0];
  tid[1] = tcp_rcv[1];

  // Protocol Identifier 확인 (2번째와 3번째 바이트)
  uint16_t protocol_identifier = (tcp_rcv[2] << 8) + tcp_rcv[3];
  if (protocol_identifier != 0x0000) return; // "Received non-Modbus TCP data

  // 데이터 길이 확인 (4번째와 5번째 바이트)
  uint16_t data_length = (tcp_rcv[4] << 8) + tcp_rcv[5];


  if(bytes_received == data_length + MODBUS_TCP_HEADER_SIZE) {  //한번에 데이터 수신이 완료된경우
    memcpy(byteBuffer.data, &tcp_rcv[6], bytes_received - 6);
    // appendToByteBuffer(&byteBuffer, &tcp_rcv[6], bytes_received-6); //save modbus date, except header
    // sendtoServer(0, byteBuffer.data, byteBuffer.length); //echo test
    return 1;
  }
  else {

    return 0;
  }

  #if 0
  //덜 받은 데이터 수신하기
  // Modbus TCP 헤더를 제외한 데이터 길이 계산
  int remaining_data_length = data_length - MODBUS_TCP_HEADER_SIZE;

  uint8_t modbus_tcp_data[remaining_data_length];
  int modbus_tcp_data_received = 0;

  modbusTcpTimeoutStart(MODBUS_TIMEOUT);
  while (modbus_tcp_data_received < remaining_data_length) {
    int bytes_received = rcvfrom(sn, modbus_tcp_data);
    appendToByteBuffer(&byteBuffer, modbus_tcp_data, bytes_received);
    modbus_tcp_data_received += bytes_received;
    if(getModbusTimeout()) return 0;
  }
  // sendtoServer(0, byteBuffer.data, byteBuffer.length); //echo test
  return 1; //modbus data received
  #endif
}

int receiveModbusTcpClient(void)  //siu01 = client mode
{
  int r = 0;
  ModbusFrame response;
  static uint8_t client_busy = 0;
  static uint16_t txid = 0;

  r = tcp_client(SIU_CLIENT_SOCKET, gSERVERNETINFO.ip, gSERVERNETINFO.port);

  if(r == SOCK_ESTABLISHED) { //check socket connected
    client_connected = 1;
    if(client_busy == 0) {
      // sl_led_turn_on(&sl_led_comm); //rx indigation start
      response.slave_addr = mNetworkConfig.modbus[REMOTE_MODEBUS_SLAVE_ADDRESS_INDEX];
      response.function_code = Write_Multiple_Registers;
      response.data_length = SIU_F04_DATA_LENGTH * 2;
      merge_VICONOX_AIRFLOW_Data(&response);            

      transmitModbus485toTcpWriteMultipleRegisters(SIU_CLIENT_SOCKET, &response, txid);
      txid++;     
      // sl_led_turn_off(&sl_led_comm); //rx indigation start
      client_busy = 1;
      sl_sleeptimer_start_timer(&clientDataPushTimer, sl_sleeptimer_ms_to_tick(CLIENT_DATA_PUSH_TIME), timerCallBack, &client_busy, 0, 0);
    }     
  }
  else {
    // server1_connected = 1;
    client_connected = 0;
    return 0;
  }
}

int receiveModbusTcpServer(void)  //siu01 = Server mode
{
  int ret = 0;
  int r = 0;

  r= tcp_server(SIU_SERVER_SOCKET, gSERVERNETINFO.port);

  if(r == SOCK_ESTABLISHED) { //check socket connected
    server1_connected = 1;
    ret = receiveModbusTcp(SIU_SERVER_SOCKET);
    if(ret) { 
      TcpReceiveAndReply(SIU_SERVER_SOCKET);
      TcpPollGuardTimeStart();
    }
  }
  else {
    server1_connected = 0;
  }

  r = tcp_server(SIU_SERVER_SOCKET_2, gSERVERNETINFO.port);

  if(r == SOCK_ESTABLISHED) { //check socket connected
    server2_connected = 1;
    ret = receiveModbusTcp(SIU_SERVER_SOCKET_2);
    if(ret) { 
      TcpReceiveAndReply(SIU_SERVER_SOCKET_2);
      TcpPollGuardTimeStart();
    }
  }
  else {
    server2_connected = 0;
  }

  return ret;
}

// #include "em_prs.h"
// #include "em_cmu.h"
// #include "sl_simple_led_comm_config.h"
// #define PRS_CH_A   6
// #define PRS_CH_B   1
// void initPrs(void)
// {
//   // Enable PRS clock
//   CMU_ClockEnable(cmuClock_PRS, true);

//   PRS_SourceAsyncSignalSet(PRS_CH_B, PRS_ASYNC_CH_CTRL_SOURCESEL_USART1, PRS_ASYNC_CH_CTRL_SIGSEL_USART1TX);
//   PRS_SourceAsyncSignalSet(PRS_CH_A, PRS_ASYNC_CH_CTRL_SOURCESEL_USART1, PRS_ASYNC_CH_CTRL_SIGSEL_USART1RXDATA);

//   // Configure PRS logic
//   PRS_Combine(PRS_CH_A, PRS_CH_B, prsLogic_A_OR_NOT_B);

//   // Route output to LED1
//   PRS_PinOutput(PRS_CH_A, prsTypeAsync, SL_SIMPLE_LED_COMM_PORT, SL_SIMPLE_LED_COMM_PIN);
// }

void TcpReceiveAndReply(uint8_t sn)
{
  ModbusFrame response;

  if(byteBuffer.data[0] == mNetworkConfig.siuAddress) { //check slave address
    // sl_led_turn_on(&sl_led_comm); //rx indigation start
    response.slave_addr = byteBuffer.data[0];
    response.function_code = byteBuffer.data[1];
    
    response.data_length = bytearray_to_uint16(&byteBuffer.data[4]);  //get word length
    response.data_length = response.data_length << 1; //x2 for byte count 

    if(bytearray_to_uint16(&byteBuffer.data[2]) == 10000) { //VICONOX REMAP ADDRESS
      if(response.data_length <= SIU_F04_DATA_LENGTH)
        mergeVICONOXdata(&response, DATA_OFFSET_00);
      else
        merge_VICONOX_AIRFLOW_Data(&response);            

      if(response.data_length > SIU_F04_DATA_LENGTH*2)
        response.data_length = SIU_F04_DATA_LENGTH; //limit 80byte
    }

    if(bytearray_to_uint16(&byteBuffer.data[2]) == 10020) { //AIRFLOW REMAP ADDRESS
      if(response.data_length > SIU_F04_DATA_LENGTH)
        response.data_length = SIU_F04_DATA_LENGTH; //limit 40byte
      mergeAIRFLOWdata(&response, DATA_OFFSET_00);
    }

    transmitModbus485toTcp(sn, &response, tid[0]<<8 | tid[1]);  //요청된 트랜잭션 번호로 회신
    // sl_led_turn_off(&sl_led_comm); //rx indigation start
  }
  else {  //bypass
    // readSensor(uint8_t slave_addr, uint8_t function_code, uint16_t reg_addr, uint16_t wLength, ModbusFrame *response)
    if(readSensor(byteBuffer.data[0], byteBuffer.data[1], (byteBuffer.data[2]<<8 | byteBuffer.data[3]), (byteBuffer.data[4]<<8 | byteBuffer.data[5]), &response)){
      //send RTU response to server
      transmitModbus485toTcp(sn, &response, tid[0]<<8 | tid[1]); //요청된 트랜잭션 번호로 회신
    }
  }
}

void clientLEDprocess(void) { //this function run in 20ms timer interrupt
  static uint8_t blink=0;
  if(mModeConfig.cfg[1] == 'O') { //CHECK CLIENT MODE
    if(client_connected) {
      sl_led_turn_on(&sl_led_client);
    } else {
      blink %= 5; //every 100ms
      if(blink == 0)
        sl_led_toggle(&sl_led_client);
      blink++;
    }
  } else {
    sl_led_turn_off(&sl_led_client);
  }
}

