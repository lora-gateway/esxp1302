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

#include <stdint.h>     /* C99 types */
#include <stdio.h>      /* printf fprintf */
#include <unistd.h>     /* lseek, close */
#include <fcntl.h>      /* open */
#include <string.h>     /* memset */

#include "loragw_spi.h"
#include "loragw_aux.h"
#include "sx1250_spi.h"


#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#if DEBUG_RAD == 1
    #define DEBUG_MSG(str)                fprintf(stdout, str)
    #define DEBUG_PRINTF(fmt, args...)    fprintf(stdout,"%s:%d: "fmt, __FUNCTION__, __LINE__, args)
    #define CHECK_NULL(a)                if(a==NULL){fprintf(stderr,"%s:%d: ERROR: NULL POINTER AS ARGUMENT\n", __FUNCTION__, __LINE__);return LGW_SPI_ERROR;}
#else
    #define DEBUG_MSG(str)
    #define DEBUG_PRINTF(fmt, args...)
    #define CHECK_NULL(a)                if(a==NULL){return LGW_SPI_ERROR;}
#endif

#define WAIT_BUSY_SX1250_MS  1


int sx1250_spi_w(spi_device_handle_t *spi, uint8_t spi_mux_target, sx1250_op_code_t op_code, uint8_t *data, uint16_t size)
{
    /* wait BUSY */
    wait_ms(WAIT_BUSY_SX1250_MS);

    /* check input variables */
    CHECK_NULL(spi);
    CHECK_NULL(data);

    return radio_spi_wb(spi, spi_mux_target, (uint8_t)op_code, data, size);
}


int sx1250_spi_r(spi_device_handle_t *spi, uint8_t spi_mux_target, sx1250_op_code_t op_code, uint8_t *data, uint16_t size)
{
    /* wait BUSY */
    wait_ms(WAIT_BUSY_SX1250_MS);

    /* check input variables */
    CHECK_NULL(spi);
    CHECK_NULL(data);

    return radio_spi_rb(spi, spi_mux_target, (uint8_t)op_code, data, size);
}
