#include "rs485.h"
#include "sl_simple_led_instances.h"


timeout_t rs485time;


void kickOffTimeout(uint32_t ms) {
  rs485time.timeout = 0;
  sl_sleeptimer_start_timer(&rs485time.timer, sl_sleeptimer_ms_to_tick(ms), timerCallBack, (void *)NULL, 0, 0);
} 

void setTimeout(void) {
  rs485time.timeout = 1;
}

int getTimeout(void) {
  return rs485time.timeout;
}

void rs485Init(void)
{
   GPIO_PinModeSet(gpioPortA, 4, gpioModePushPull, 1); //TX ENABLE
}

void flushrs485Buffer(int flush_count)
{
  char c;
  sl_status_t read_result;
  while (flush_count != 0) {
    read_result = sl_iostream_getchar(sl_iostream_rs485_handle, &c);
    if(read_result == SL_STATUS_OK) flush_count--;
    //app_log_info("read_result %02X\n", c);
  }
}

void flushrs485(void)
{
  char c;
  sl_status_t read_result;
  while (1) {
    read_result = sl_iostream_getchar(sl_iostream_rs485_handle, &c);
    if(read_result != SL_STATUS_OK) return;
  }
}

// #define GUARD_TIME 2000 //usec  3.5 character time @ 19200bps
#define GUARD_TIME 600          //usec 1 character time @ 19200bps
#define ONE_BYTE_WAIT_TIME 600  //usec 1 character time @ 19200bps
void rs485Send(uint8_t *p, int len, uint8_t echo)
{
  sl_status_t sc;
  uint8_t *q;
  q = p;

  // sl_led_turn_on(&sl_led_comm); //tx indigation start

  //switching to tx mode
  GPIO_PinOutSet(gpioPortA, 4); //ENABLE TXD
  sl_udelay_wait(GUARD_TIME);   //wait GUARD_TIME us(1byte send time) @ 19200bps for line stable
  flushrs485();                 //clean the receive buffer, something noise and garbage

  for(int i=0; i < len; i++) {
    sc = sl_iostream_putchar(sl_iostream_rs485_handle, *p);
    p++;
    if(sc != SL_STATUS_OK) {
      sl_udelay_wait(ONE_BYTE_WAIT_TIME);  //wait ONE_BYTE_WAIT_TIME us(1byte send time) @ 19200bps
    }
  }

  // for(int i=0; i < len; i++) {
  //   while(sl_iostream_putchar(sl_iostream_rs485_handle, *p) != SL_STATUS_OK);
  //   p++;
  // }

  if(echo == ECHO_OFF) { //for loop back, normally 0, test for 1
    flushrs485Buffer(len);
  }
  else {
    memset(q, 0, len);
    rs485Receive(q, len);
  }
  sl_udelay_wait(GUARD_TIME);          //wait GUARD_TIME us(1byte send time) @ 19200bps for line stable
  GPIO_PinOutClear(gpioPortA, 4); //DISABLE TXD

  // sl_led_turn_off(&sl_led_comm); //tx indigation end
}

void rs485Receive(uint8_t *p, int len)
{
  sl_status_t read_result;
  while (len) {
    read_result = sl_iostream_getchar(sl_iostream_rs485_handle, p);
    if(read_result == SL_STATUS_OK) {
      p++;
      len--;
    }
  }
}

extern int printBT(char *fmt, ...);

// Calculate the CRC-16 for Modbus
uint16_t calculateCRC16(uint8_t *data, uint16_t length) {
  uint16_t crc = 0xFFFF;
  uint8_t i, j;

  for (i = 0; i < length; i++) {
    crc ^= data[i];
    for (j = 0; j < 8; j++) {
      if (crc & 0x0001) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

// Transmit Modbus request
int transmitModbusRequest(ModbusFrame *request) {
  // Prepare the request frame
  uint8_t frame[256]; // Adjust the buffer size as per your requirements
  int frame_length = 0;
  
  frame[frame_length++] = request->slave_addr;
  frame[frame_length++] = request->function_code;
  for (int i = 0; i < request->data_length; i++) {
      frame[frame_length++] = request->data[i];
  }
  
  // Calculate the CRC
  uint16_t crc = calculateCRC16(frame, frame_length);
  frame[frame_length++] = crc & 0xFF;
  frame[frame_length++] = (crc >> 8) & 0xFF;

#if 0
  printBT("tx:%02X%02X%02X%02X%02X%02X%02X%02X",frame[0],frame[1],frame[2],frame[3],frame[4],frame[5],frame[6],frame[7]);
#endif

  // TODO: Send the request frame over the Modbus interface
  rs485Send(frame, frame_length, ECHO_OFF);
  
  // TODO: Receive the response frame from the Modbus interface

  // TODO: Verify the response frame and extract the response data

  return 0;
}

// Receive Modbus response
int receiveModbusResponse(ModbusFrame *response) {
    // TODO: Receive the response frame from the Modbus interface byte by byte

    uint8_t buffer[256]; // Adjust the buffer size as per your requirements
    int buffer_length = 0;
    bool frame_started = false;
    bool frame_completed = false;
    uint16_t received_crc = 0;
    uint16_t calculated_crc = 0;
    uint8_t expected_length = 0;
    uint8_t received_byte ;

    sl_status_t read_result;

    kickOffTimeout(MODBUS_TIMEOUT);
    while (!frame_completed) {
        // TODO: Receive a byte from the Modbus interface
        read_result = sl_iostream_getchar(sl_iostream_rs485_handle, &received_byte);

        // Check if a byte is received
        if (read_result == SL_STATUS_OK) {
            sl_led_turn_on(&sl_led_comm); //rx indigat≈ion start
            // Check if the frame has started
            if (!frame_started) {
                // Check for slave address and function code
                if (buffer_length == 0 && received_byte == response->slave_addr) {
                    buffer[buffer_length++] = received_byte;
                }  
                
                if (buffer_length == 1 && received_byte == response->function_code) {
                    buffer[buffer_length++] = received_byte;
                    frame_started = true;
                }  
            } else {
                buffer[buffer_length] = received_byte;
                // Check if the complete frame is received
                if (buffer_length == 2) {
                    expected_length = received_byte + 5;  // Data length + Slave address + Function code + CRC
                }
                else if (buffer_length == expected_length-1) {
                    // Verify the CRC
                    received_crc = (buffer[buffer_length] << 8) | buffer[buffer_length-1];
                    calculated_crc = calculateCRC16(buffer, buffer_length - 1);
                    // printBT("crc:%04X %04X",received_crc, calculated_crc);

                    // frame_completed = true;
                    if (received_crc == calculated_crc) {
                        frame_completed = true;
                    } else {
                        // CRC check failed
                        flushrs485(); //clean the garbage data
                        sl_led_turn_off(&sl_led_comm); //rx indigation end
                        // printBT("crc error");
                        return 0;
                    }
                }
                buffer_length++;
            }
        }
        else {
          sl_udelay_wait(ONE_BYTE_WAIT_TIME);  //wait ONE_BYTE_WAIT_TIME us(1byte send time) @ 19200bps
        }
        // TODO: Wait for the next byte or handle timeouts if necessary
        if(getTimeout()) {
          flushrs485(); //clean the garbage data
          sl_led_turn_off(&sl_led_comm); //rx indigation end
          // printBT("time out %02X", response->slave_addr);
          return 0;  
        }
    }

    // Extract the response data
    response->slave_addr = buffer[0];
    response->function_code = buffer[1];
    response->data_length = buffer[2];
    memcpy(response->data, &buffer[3], response->data_length);
  
    // printBT("rx:%02X%02X%02X%02X%02X%02X%02X%02X",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7]);
    // printBT("crc:%02X%02X",buffer[buffer_length-1],buffer[buffer_length-2]);
    // printBT("LEN:%d",buffer_length);

    sl_led_turn_off(&sl_led_comm); //rx indigation end
    if(frame_completed) {
      // printBT("buffer len=%d", buffer_length);
      return buffer_length;
    } else {
      // printBT("frame not complete", buffer_length);
      return 0;
    }
}

uint32_t copyArrayToUint32(const unsigned char* array)
{
    uint32_t value = 0;

    value |= array[3];
    value |= array[2] << 8;
    value |= array[1] << 16;
    value |= array[0] << 24;

    return value;
}

void uint32_to_bytearray(uint32_t value, uint8_t* array) {
    for (int i = 0; i < 4; i++) {
        array[3 - i] = (value >> (8 * i)) & 0xFF;
    }
}

void uint16_to_bytearray(uint16_t value, uint8_t* array) {
    for (int i = 0; i < 2; i++) {
        array[1 - i] = (value >> (8 * i)) & 0xFF;
    }
}

uint16_t bytearray_to_uint16(uint8_t bytes[2]) {
    return (bytes[0] << 8) | bytes[1];
}

float convertToFloat(uint32_t value)
{
    union
    {
        uint32_t intValue;
        float floatValue;
    } u;

    u.intValue = value;
    return u.floatValue;
}

uint32_t floatToUint32(float value) {
    union
    {
        uint32_t intValue;
        float floatValue;
    } u;

    u.floatValue = value;
    return u.intValue;
}

float arrayToFloat(const unsigned char* array)
{
  uint32_t value;
  value = copyArrayToUint32(array);
  return convertToFloat(value);
}

uint32_t calibrateSensorValue(uint32_t value, float offset) {
  float tmp = convertToFloat(value) + offset;
  return floatToUint32(tmp);
}
