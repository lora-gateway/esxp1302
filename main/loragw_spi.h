/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2019 Semtech

Description:
    Host specific functions to address the LoRa concentrator registers through
    a SPI interface.
    Single-byte read/write and burst read/write.
    Could be used with multiple SPI ports in parallel (explicit file descriptor)

License: Revised BSD License, see LICENSE.TXT file include in the project
*/


#ifndef _LORAGW_SPI_H
#define _LORAGW_SPI_H

/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */

#include <stdint.h>        /* C99 types*/
#include "driver/spi_master.h"

//#include "config.h"    /* library configuration options (dynamically generated) */

/* -------------------------------------------------------------------------- */
/* --- PUBLIC CONSTANTS ----------------------------------------------------- */

#define LGW_SPI_SUCCESS     0
#define LGW_SPI_ERROR       -1
#define LGW_BURST_CHUNK     1024

#define SPI_SPEED       2000000

#define LGW_SPI_MUX_TARGET_SX1302   0x00
#define LGW_SPI_MUX_TARGET_RADIOA   0x01
#define LGW_SPI_MUX_TARGET_RADIOB   0x02

/* -------------------------------------------------------------------------- */
/* --- PUBLIC FUNCTIONS PROTOTYPES ------------------------------------------ */

/**
@brief LoRa concentrator SPI setup (configure I/O and peripherals)
*/
//int lgw_spi_open(spi_device_handle_t *spi);
int lgw_spi_open(void **spi);

/**
@brief LoRa concentrator SPI close
*/
int lgw_spi_close(spi_device_handle_t *spi);

/**
@brief LoRa concentrator SPI single-byte write
*/
int lgw_spi_w(spi_device_handle_t *spi, uint8_t spi_mux_target, uint16_t address, uint8_t data);

/**
@brief LoRa concentrator SPI single-byte read
*/
int lgw_spi_r(spi_device_handle_t *spi, uint8_t spi_mux_target, uint16_t address, uint8_t *data);

/**
@brief LoRa concentrator SPI burst (multiple-byte) write
*/
int lgw_spi_wb(spi_device_handle_t *spi, uint8_t spi_mux_target, uint16_t address, const uint8_t *data, uint16_t size);

/**
@brief LoRa concentrator SPI burst (multiple-byte) read
*/
int lgw_spi_rb(spi_device_handle_t *spi, uint8_t spi_mux_target, uint16_t address, uint8_t *data, uint16_t size);

#endif

/* --- EOF ------------------------------------------------------------------ */
