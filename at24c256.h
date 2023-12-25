#ifndef AT24C256_H
#define AT24C256_H

#include <stdint.h>      // For uint8_t type
#include "sl_i2cspm_instances.h"
#include "sl_status.h"
#include "wizchip_conf.h"
#include "string.h"

#define EEPROM_ADDRESS 0x50
sl_status_t readEEPROM(uint16_t address, uint8_t *buffer, uint16_t length);
sl_status_t writeEEPROM(uint16_t address, uint8_t *data, uint16_t length);

//#################################################################################
//# at24c256 memory map
//#################################################################################
// 0x0000 - 0x00FF ===> network config
// 0x0100 - 0x01FF ===> sensor configuration
// 0x0200 - 0x02FF ===> mode config
// 0x0300 - 0x03FF ===> event log counter
// 0x0400 - 0x04FF ===> spare
// 0x0500 - 0x05FF ===> offset for calibration
// 0x0600 - 0x06FF ===> spare
// 0x0700 - 0x0EFF ===> password
// 0x0800 - 0x0FFF ===> spare
//---------------------------------------------------------------------------------
// 0x1000 - 0x3FFF ===> event log block 
// 0x4000 - 0x4FFF
// 0x5000 - 0x7FFF 

//define address
#define EEP_ADDR_NETWORK_CONFIG         0x0000
#define EEP_ADDR_SENSOR_CONFIG          0x0100
#define EEP_ADDR_MODE_CONFIG            0x0200
#define EEP_ADDR_SLAVE1_EVENT_COUNTER   0x0300
#define EEP_ADDR_SLAVE2_EVENT_COUNTER   0x0400
#define EEP_ADDR_OFFSET_VALUE           0x0500
#define EEP_ADDR_PASSWORD               0x0700

#define EEP_ADDR_EVENT_LOG_COUNTER      0x0300
#define EEP_ADDR_EVENT_LOG_DATA_BASE    0x1000

typedef struct {
    wiz_NetInfo net;
    uint8_t workingMode;
} localIpFrame;

typedef struct
{
   uint8_t  ip[4];   ///< Server IP Address
   uint16_t port;   ///< Subnet Mask 
} remoteIpFrame;

typedef struct {
    uint8_t slaveAddress;
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minut;
    uint8_t second;
    uint16_t statusAddress;     //sensor status memory address 10580, 10590, 10591, 50000(DI)
    uint16_t status;            //status 
    uint8_t spare[5];
} EventLogFrame;

typedef struct {
    uint8_t vi;
    uint8_t co;
    uint8_t no;
    uint8_t no2;
    uint8_t nox;
    uint8_t velocty;
    uint8_t spare[10];
} sensorConfigFrame;

typedef struct {
    uint16_t count;
    uint8_t spare[14];
} eventCountFrame;

typedef struct {
    uint8_t pass[4];
    uint8_t spare[12];
} passwordFrame;

typedef struct {
    uint8_t slaveAddress;   //99 default
    uint16_t startAddress;   //1000 default
    uint8_t spare[12];
} siuConfigFrame;

#endif
