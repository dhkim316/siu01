#include "at24c256.h"
#include "sl_udelay.h"

sl_status_t readEEPROM(uint16_t address, uint8_t *buffer, uint16_t length) {
    // Create an I2C transaction structure
    I2C_TransferSeq_TypeDef seq;

    // Create an I2C transfer structure
    sl_status_t ret;

    // Set up the read operation
    uint8_t txBuffer[2];  // Memory address
    txBuffer[0] = (address >> 8) & 0xFF;  // High byte of memory address
    txBuffer[1] = address & 0xFF;         // Low byte of memory address

    // Initialize the I2C transaction structure
    seq.addr = EEPROM_ADDRESS << 1;  // Shift EEPROM address to the left by 1 to accommodate R/W bit
    seq.flags = I2C_FLAG_WRITE_READ; // Set write-read operation flag
    seq.buf[0].data = txBuffer;       // Set the transmit buffer
    seq.buf[0].len = 2;               // Length of transmit buffer
    seq.buf[1].data = buffer;         // Set the receive buffer
    seq.buf[1].len = length;          // Length of receive buffer

    // Perform the I2C transfer
    ret = I2CSPM_Transfer(sl_i2cspm_BBi2c, &seq);

    // Check if the transfer was successful
    if (ret != i2cTransferDone) {
        // Handle error condition
        // ...
    }
    return ret;
}

sl_status_t writeEEPROM(uint16_t address, uint8_t *data, uint16_t length) {
    // Create an I2C transaction structure
    I2C_TransferSeq_TypeDef seq;

    // Create an I2C transfer structure
    sl_status_t ret;

    // Set up the write operation
    uint8_t txBuffer[length + 2];  // Include memory address
    txBuffer[0] = (address >> 8) & 0xFF;  // High byte of memory address
    txBuffer[1] = address & 0xFF;         // Low byte of memory address
    memcpy(&txBuffer[2], data, length);   // Copy data to transmit buffer

    // Initialize the I2C transaction structure
    seq.addr = EEPROM_ADDRESS << 1;  // Shift EEPROM address to the left by 1 to accommodate R/W bit
    seq.flags = I2C_FLAG_WRITE;      // Set write operation flag
    seq.buf[0].data = txBuffer;       // Set the transmit buffer
    seq.buf[0].len = length + 2;      // Length of transmit buffer

    // Perform the I2C transfer
    ret = I2CSPM_Transfer(sl_i2cspm_BBi2c, &seq);

    sl_udelay_wait(5000);   //wait 5ms
    // Check if the transfer was successful
    if (ret != i2cTransferDone) {
        // Handle error condition
        // ...
    }
    return ret;
}
