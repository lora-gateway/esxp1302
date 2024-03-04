/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2019 Semtech

Description:
    Functions used to handle LoRa concentrator SX1250 radios.

License: Revised BSD License, see LICENSE.TXT file include in the project
*/

#ifndef _SX1250_SPI_H
#define _SX1250_SPI_H

#include <stdint.h>     /* C99 types*/

#include "loragw_spi.h"
#include "sx1250_defs.h"
#include "config.h"     /* library configuration options (dynamically generated) */


int sx1250_spi_w(spi_device_handle_t *spi, uint8_t spi_mux_target, sx1250_op_code_t op_code, uint8_t *data, uint16_t size);
int sx1250_spi_r(spi_device_handle_t *spi, uint8_t spi_mux_target, sx1250_op_code_t op_code, uint8_t *data, uint16_t size);

#endif
