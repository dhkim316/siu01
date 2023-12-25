/***************************************************************************//**
 * @file
 * @brief SPIDRV Config
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef SL_SPIDRV_W5500_CONFIG_H
#define SL_SPIDRV_W5500_CONFIG_H

#include "spidrv.h"

// <<< Use Configuration Wizard in Context Menu >>>
// <h> SPIDRV settings

// <o SL_SPIDRV_W5500_BITRATE> SPI bitrate
// <i> Default: 1000000
#define SL_SPIDRV_W5500_BITRATE           1000000

// <o SL_SPIDRV_W5500_FRAME_LENGTH> SPI frame length <4-16>
// <i> Default: 8
#define SL_SPIDRV_W5500_FRAME_LENGTH      8

// <o SL_SPIDRV_W5500_TYPE> SPI mode
// <spidrvMaster=> Master
// <spidrvSlave=> Slave
#define SL_SPIDRV_W5500_TYPE              spidrvMaster

// <o SL_SPIDRV_W5500_BIT_ORDER> Bit order on the SPI bus
// <spidrvBitOrderLsbFirst=> LSB transmitted first
// <spidrvBitOrderMsbFirst=> MSB transmitted first
#define SL_SPIDRV_W5500_BIT_ORDER         spidrvBitOrderMsbFirst

// <o SL_SPIDRV_W5500_CLOCK_MODE> SPI clock mode
// <spidrvClockMode0=> SPI mode 0: CLKPOL=0, CLKPHA=0
// <spidrvClockMode1=> SPI mode 1: CLKPOL=0, CLKPHA=1
// <spidrvClockMode2=> SPI mode 2: CLKPOL=1, CLKPHA=0
// <spidrvClockMode3=> SPI mode 3: CLKPOL=1, CLKPHA=1
#define SL_SPIDRV_W5500_CLOCK_MODE        spidrvClockMode0

// <o SL_SPIDRV_W5500_CS_CONTROL> SPI master chip select (CS) control scheme.
// <spidrvCsControlAuto=> CS controlled by the SPI driver
// <spidrvCsControlApplication=> CS controlled by the application
#define SL_SPIDRV_W5500_CS_CONTROL        spidrvCsControlAuto

// <o SL_SPIDRV_W5500_SLAVE_START_MODE> SPI slave transfer start scheme
// <spidrvSlaveStartImmediate=> Transfer starts immediately
// <spidrvSlaveStartDelayed=> Transfer starts when the bus is idle (CS deasserted)
// <i> Only applies if instance type is spidrvSlave
#define SL_SPIDRV_W5500_SLAVE_START_MODE  spidrvSlaveStartImmediate
// </h>
// <<< end of configuration section >>>

// <<< sl:start pin_tool >>>
// <usart signal=TX,RX,CLK,(CS)> SL_SPIDRV_W5500
// $[USART_SL_SPIDRV_W5500]
#warning "SPIDRV USART peripheral not configured"
 #define SL_SPIDRV_W5500_PERIPHERAL        USART1
 #define SL_SPIDRV_W5500_PERIPHERAL_NO     0

 #define SL_SPIDRV_W5500_TX_PORT           gpioPortA
 #define SL_SPIDRV_W5500_TX_PIN            0

 #define SL_SPIDRV_W5500_RX_PORT           gpioPortA
 #define SL_SPIDRV_W5500_RX_PIN            1

 #define SL_SPIDRV_W5500_CLK_PORT          gpioPortA
 #define SL_SPIDRV_W5500_CLK_PIN           2

 #define SL_SPIDRV_W5500_CS_PORT           gpioPortA
 #define SL_SPIDRV_W5500_CS_PIN            3
// [USART_SL_SPIDRV_W5500]$
// <<< sl:end pin_tool >>>

#endif // SL_SPIDRV_W5500_CONFIG_H
