#ifndef ISL1208_H
#define ISL1208_H
#include "stdint.h"
#include "sl_status.h"

/*
 * Intersil ISL1208 rtc class driver
 */

#define ISL1208_SLAVE_ADDRESS 0x6F

/* Register map */
/* rtc section */
#define ISL1208_REG_SC  0x00
#define ISL1208_REG_MN  0x01
#define ISL1208_REG_HR  0x02
#define ISL1208_REG_HR_MIL     (1<<7) /* 24h/12h mode */
#define ISL1208_REG_HR_PM      (1<<5) /* PM/AM bit in 12h mode */
#define ISL1208_REG_DT  0x03
#define ISL1208_REG_MO  0x04
#define ISL1208_REG_YR  0x05
#define ISL1208_REG_DW  0x06
#define ISL1208_RTC_SECTION_LEN 7

/* control/status section */
#define ISL1208_REG_SR  0x07
#define ISL1208_REG_SR_ARST    (1<<7) /* auto reset */
#define ISL1208_REG_SR_XTOSCB  (1<<6) /* crystal oscillator */
#define ISL1208_REG_SR_WRTC    (1<<4) /* write rtc */
#define ISL1208_REG_SR_EVT     (1<<3) /* event */
#define ISL1208_REG_SR_ALM     (1<<2) /* alarm */
#define ISL1208_REG_SR_BAT     (1<<1) /* battery */
#define ISL1208_REG_SR_RTCF    (1<<0) /* rtc fail */
#define ISL1208_REG_INT 0x08
#define ISL1208_REG_INT_ALME   (1<<6)   /* alarm enable */
#define ISL1208_REG_INT_IM     (1<<7)   /* interrupt/alarm mode */
#define ISL1219_REG_EV  0x09
#define ISL1219_REG_EV_EVEN    (1<<4)   /* event detection enable */
#define ISL1219_REG_EV_EVIENB  (1<<7)   /* event in pull-up disable */
#define ISL1208_REG_ATR 0x0a
#define ISL1208_REG_DTR 0x0b

/* alarm section */
#define ISL1208_REG_SCA 0x0c
#define ISL1208_REG_MNA 0x0d
#define ISL1208_REG_HRA 0x0e
#define ISL1208_REG_DTA 0x0f
#define ISL1208_REG_MOA 0x10
#define ISL1208_REG_DWA 0x11
#define ISL1208_ALARM_SECTION_LEN 6

/* user section */
#define ISL1208_REG_USR1 0x12
#define ISL1208_REG_USR2 0x13
#define ISL1208_USR_SECTION_LEN 2

/* event section */
#define ISL1219_REG_SCT 0x14
#define ISL1219_REG_MNT 0x15
#define ISL1219_REG_HRT 0x16
#define ISL1219_REG_DTT 0x17
#define ISL1219_REG_MOT 0x18
#define ISL1219_REG_YRT 0x19
#define ISL1219_EVT_SECTION_LEN 6


// #define   const_bcd2bin(x)   (((x) & 0x0f) + ((x) >> 4) * 10)
// #define   const_bin2bcd(x)   ((((x) / 10) << 4) + (x) % 10)
#define   bcd2bin(x)   (((x) & 0x0f) + ((x) >> 4) * 10)
#define   bin2bcd(x)   ((((x) / 10) << 4) + (x) % 10)

struct rtc_time {
  uint8_t tm_sec;
  uint8_t tm_min;
  uint8_t tm_hour;
  uint8_t tm_mday;
  uint8_t tm_mon;
  uint16_t tm_year;
  uint8_t tm_wday;
  uint8_t tm_yday;
  uint8_t tm_isdst;
};

sl_status_t ISL1208_read_register(uint8_t addr, uint8_t *data);
sl_status_t ISL1208_write_register(uint8_t addr, uint8_t data);

uint8_t isl1208_i2c_get_sr(void);
void isl1208_i2c_set_sr(uint8_t val);

int isl1208_i2c_read_time(struct rtc_time *tm);
int isl1208_i2c_set_time(struct rtc_time const *tm);

uint32_t getTimeStamp(struct rtc_time date);

#endif // ISL1208_H