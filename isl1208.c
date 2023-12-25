#include "sl_i2cspm_instances.h"
#include "string.h"
#include "isl1208.h"

/***************************************************************************//**
* Read register value
*******************************************************************************/
sl_status_t ISL1208_read_register(uint8_t addr, uint8_t *data)
{
  I2C_TransferSeq_TypeDef seq;
  uint8_t i2c_write_data;

  if (data == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  } else {
    seq.addr = ISL1208_SLAVE_ADDRESS << 1;
    seq.flags = (uint16_t)I2C_FLAG_WRITE_READ;

    i2c_write_data = addr;

    /*Write buffer*/
    seq.buf[0].data = &i2c_write_data;
    seq.buf[0].len = 1;

    /*Read buffer*/
    seq.buf[1].data = data;
    seq.buf[1].len = 1;

    if (I2CSPM_Transfer(sl_i2cspm_BBi2c, &seq) != i2cTransferDone) {
      return SL_STATUS_TRANSMIT;
    }
    return SL_STATUS_OK;
  }
}

/***************************************************************************//**
* Write into register
*******************************************************************************/
sl_status_t ISL1208_write_register(uint8_t addr, uint8_t data)
{
  I2C_TransferSeq_TypeDef seq;
  uint8_t i2c_write_data[2];

  seq.addr = ISL1208_SLAVE_ADDRESS << 1;
  seq.flags = (uint16_t)I2C_FLAG_WRITE;

  i2c_write_data[0] = addr;
  i2c_write_data[1] = data;

  /*Write buffer*/
  seq.buf[0].data = i2c_write_data;
  seq.buf[0].len = 2;

  if (I2CSPM_Transfer(sl_i2cspm_BBi2c, &seq) != i2cTransferDone) {
    return SL_STATUS_TRANSMIT;
  }
  return SL_STATUS_OK;
}

uint8_t isl1208_i2c_get_sr(void)
{
  uint8_t data;
  ISL1208_read_register(ISL1208_REG_SR, &data);
  return data;
}

#if 0
static uint8_t isl1208_i2c_get_atr(void)
{
  uint8_t data;
  uint8_t atr;

  ISL1208_read_register(ISL1208_REG_ATR, &data);
  atr = data;
  // if (atr < 0)
  //   return atr;

  /* The 6bit value in the ATR register controls the load
   * capacitance C_load * in steps of 0.25pF
   *
   * bit (1<<5) of the ATR register is inverted
   *
   * C_load(ATR=0x20) =  4.50pF
   * C_load(ATR=0x00) = 12.50pF
   * C_load(ATR=0x1f) = 20.25pF
   *
   */

  atr &= 0x3f;    /* mask out lsb */
  atr ^= 1 << 5;    /* invert 6th bit */
  atr += 2 * 9;   /* add offset of 4.5pF; unit[atr] = 0.25pF */

  return atr;
}
#endif

void isl1208_i2c_set_sr(uint8_t val) 
{
  ISL1208_write_register(ISL1208_REG_SR, val);
}

#if 0
/* returns adjustment value + 100 */
static int isl1208_i2c_get_dtr(void)
{
  int dtr;
  uint8_t data;
  ISL1208_read_register(ISL1208_REG_DTR, &data);
  if (dtr < 0)
    return -1;

  /* dtr encodes adjustments of {-60,-40,-20,0,20,40,60} ppm */
  dtr = ((dtr & 0x3) * 20) * (dtr & (1 << 2) ? -1 : 1);

  return dtr + 100;
}
#endif

int isl1208_i2c_read_time(struct rtc_time *tm)
{
  uint8_t reg[1];

  isl1208_i2c_get_sr(); //just read
  
  ISL1208_read_register(ISL1208_REG_SC, reg);
  tm->tm_sec = bcd2bin(reg[0]);

  ISL1208_read_register(ISL1208_REG_MN, reg);
  tm->tm_min = bcd2bin(reg[0]);

  /* HR field has a more complex interpretation */
  {
    uint8_t _hr;
    ISL1208_read_register(ISL1208_REG_HR, &_hr);  //always 24H time
    tm->tm_hour = bcd2bin(_hr & 0x3f);

    // if (_hr & ISL1208_REG_HR_MIL) /* 24h format */
    //   tm->tm_hour = bcd2bin(_hr & 0x3f);
    // else {
    //   /* 12h format */
    //   tm->tm_hour = bcd2bin(_hr & 0x1f);
    //   if (_hr & ISL1208_REG_HR_PM)  /* PM flag set */
    //     tm->tm_hour += 12;
    // }
  }

  ISL1208_read_register(ISL1208_REG_DT, reg);
  tm->tm_mday = bcd2bin(reg[0]);
  ISL1208_read_register(ISL1208_REG_MO, reg);
  tm->tm_mon = bcd2bin(reg[0]);

  ISL1208_read_register(ISL1208_REG_YR, reg);
  tm->tm_year = bcd2bin(reg[0]);
  ISL1208_read_register(ISL1208_REG_DW, reg);
  tm->tm_wday = bcd2bin(reg[0]);

  return 0;
}


int isl1208_i2c_set_time(struct rtc_time const *tm)
{
  uint8_t sr;
  /* set WRTC */
  sr = isl1208_i2c_get_sr();
  ISL1208_write_register(ISL1208_REG_SR, sr | 0x10);

  ISL1208_write_register(ISL1208_REG_SC, bin2bcd(tm->tm_sec));
  ISL1208_write_register(ISL1208_REG_MN, bin2bcd(tm->tm_min));
  ISL1208_write_register(ISL1208_REG_HR, bin2bcd(tm->tm_hour) | 0x80);  //24hour

  ISL1208_write_register(ISL1208_REG_DT, bin2bcd(tm->tm_mday));
  ISL1208_write_register(ISL1208_REG_MO, bin2bcd(tm->tm_mon));
  ISL1208_write_register(ISL1208_REG_YR, bin2bcd(tm->tm_year));

  // ISL1208_write_register(ISL1208_REG_DW, bin2bcd(tm->tm_wday));

  /* clear WRTC again */
  sr = isl1208_i2c_get_sr();
  ISL1208_write_register(ISL1208_REG_SR, sr & ~(0x10));
  return 0;
}
uint32_t getTimeStamp(struct rtc_time date)
{
  uint16_t y;
  uint8_t m;
  uint8_t d;
  uint32_t t;

  //Year
  y = date.tm_year;
  //Month of year
  m = date.tm_mon;
  //Day of month
  d = date.tm_mday;

  //January and February are counted as months 13 and 14 of the previous year
  if(m <= 2)
  {
      m += 12;
      y -= 1;
  }

  //Convert years to days
  t = (365 * y) + (y / 4) - (y / 100) + (y / 400);
  //Convert months to days
  t += (30 * m) + (3 * (m + 1) / 5) + d;
  //Unix time starts on January 1st, 1970
  t -= 719561;
  //Convert days to seconds
  t *= 86400;
  //Add hours, minutes and seconds
  t += (3600 * date.tm_hour) + (60 * date.tm_min) + date.tm_sec;

  //Return Unix time
  return t;
}
