/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2019 Semtech

Description:
    Host specific functions to address the LoRa concentrator I2C peripherals.

License: Revised BSD License, see LICENSE.TXT file include in the project
*/

#ifndef _LORAGW_I2C_H
#define _LORAGW_I2C_H

#include <stdint.h>        /* C99 types*/
#include "config.h"


#define LGW_I2C_SUCCESS      0
#define LGW_I2C_ERROR       -1

#define ACK_CHECK_EN       0x1      /* I2C master will check ack from slave*/
#define ACK_CHECK_DIS      0x0      /* I2C master will not check ack from slave */
#define ACK_VAL            0x0      /* I2C ack value */
#define NACK_VAL           0x1      /* I2C nack value */

#define I2C_MASTER_SCL_IO   19      /* gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO   18      /* gpio number for I2C master data  */
#define I2C_MASTER_NUM       0      /* I2C port number for master dev */

#define I2C_MASTER_TX_BUF_DISABLE  0        /* I2C master do not need buffer */
#define I2C_MASTER_RX_BUF_DISABLE  0        /* I2C master do not need buffer */
#define I2C_MASTER_FREQ_HZ         100000   /* I2C master clock frequency */


/**
@brief Open I2C port
@param i2c_num      I2C port number
@return 0 if I2C port was open successfully, -1 else
*/
esp_err_t i2c_esp32_open(i2c_port_t i2c_num);

/**
@brief Close I2C port
@param i2c_num      I2C port number
@return 0 if I2C port was closed successfully, -1 else
*/
esp_err_t i2c_esp32_close(i2c_port_t i2c_num);

/**
@brief Read data from an I2C port
@param i2c_num      I2C port number
@param device_addr  I2C device address
@param reg_addr     Address of the register to be read
@param data         Pointer to a buffer to store read data
@return 0 if I2C data read is successful, -1 else
*/
esp_err_t i2c_esp32_read(i2c_port_t i2c_num, uint8_t device_addr, uint8_t reg_addr, uint8_t *data);

/**
@brief Write data to an I2C port
@param i2c_num      I2C port number
@param device_addr  I2C device address
@param reg_addr     Address of the register to write to
@param data         byte to write in the register
@return 0 if I2C data write is successful, -1 else
*/
esp_err_t i2c_esp32_write(i2c_port_t i2c_num, uint8_t device_addr, uint8_t reg_addr, uint8_t data);

#endif
