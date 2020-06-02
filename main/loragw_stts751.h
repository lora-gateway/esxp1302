/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2019 Semtech

Description:
    Basic driver for ST ts751 temperature sensor

License: Revised BSD License, see LICENSE.TXT file include in the project
*/

#ifndef _LORAGW_STTS751_H
#define _LORAGW_STTS751_H


#include <stdint.h>     /* C99 types */
#include <stdbool.h>    /* bool type */
#include "config.h"


#define I2C_PORT_TEMP_SENSOR_0    0x39  /* STTS751-0DP3F */
#define I2C_PORT_TEMP_SENSOR_1    0x3B  /* STTS751-1DP3F */


int stts751_configure(i2c_port_t i2c_num, uint8_t i2c_addr);
int stts751_get_temperature(i2c_port_t i2c_num, uint8_t i2c_addr, float * temperature);

#endif
