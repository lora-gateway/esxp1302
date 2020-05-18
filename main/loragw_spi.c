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


/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */

#include <stdint.h>     /* C99 types */
#include <stdio.h>      /* printf fprintf */
#include <stdlib.h>     /* malloc free */
#include <unistd.h>     /* lseek, close */
#include <fcntl.h>      /* open */
#include <string.h>     /* memset */

//#include <sys/ioctl.h>
//#include <linux/spi/spidev.h>

#include "loragw_spi.h"
//#include "loragw_aux.h"

/* -------------------------------------------------------------------------- */
/* --- PRIVATE MACROS ------------------------------------------------------- */

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#if DEBUG_SPI == 1
    #define DEBUG_MSG(str)                fprintf(stderr, str)
    #define DEBUG_PRINTF(fmt, args...)    fprintf(stderr,"%s:%d: "fmt, __FUNCTION__, __LINE__, args)
    #define CHECK_NULL(a)                if(a==NULL){fprintf(stderr,"%s:%d: ERROR: NULL POINTER AS ARGUMENT\n", __FUNCTION__, __LINE__);return LGW_SPI_ERROR;}
#else
    #define DEBUG_MSG(str)
    #define DEBUG_PRINTF(fmt, args...)
    #define CHECK_NULL(a)                if(a==NULL){return LGW_SPI_ERROR;}
#endif

/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS ---------------------------------------------------- */

#define READ_ACCESS     0x0000
#define WRITE_ACCESS    0x8000
#define ADDR_MASK       0x7fff

#define PIN_NUM_MISO 12
#define PIN_NUM_MOSI 13
#define PIN_NUM_CLK  14
#define PIN_NUM_CS   15

#define SX1302_SPI_HOST    HSPI_HOST
#define DMA_CHAN    2

/* -------------------------------------------------------------------------- */
/* --- PUBLIC FUNCTIONS DEFINITION ------------------------------------------ */

/* SPI initialization and configuration */
int lgw_spi_open(spi_device_handle_t *spi)
{
    esp_err_t ret;

    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LGW_BURST_CHUNK,
    };
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = SPI_SPEED,
        .mode = 0,
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 8,
    };

    // Initialize the SPI bus
    ret = spi_bus_initialize(SX1302_SPI_HOST, &buscfg, DMA_CHAN);
    ESP_ERROR_CHECK(ret);

    // Attach SX1302 to the SPI bus
    ret = spi_bus_add_device(SX1302_SPI_HOST, &devcfg, spi);
    ESP_ERROR_CHECK(ret);

    return LGW_SPI_SUCCESS;
}

/* SPI release */
int lgw_spi_close(spi_device_handle_t *spi)
{
    esp_err_t ret;

    // esp_err_t spi_bus_remove_device(spi_device_handle_t handle);
    ret = spi_bus_remove_device(*spi);
    ESP_ERROR_CHECK(ret);
    // printf("ret = %d\n", ret);

    // esp_err_t spi_bus_free(spi_host_device_t host_id);
    ret = spi_bus_free(SX1302_SPI_HOST);
    ESP_ERROR_CHECK(ret);
    // printf("ret = %d\n", ret);

    return LGW_SPI_SUCCESS;
}

/* Simple write */
int lgw_spi_w(spi_device_handle_t *spi, uint16_t address, uint8_t data)
{
    esp_err_t err;

    err = spi_device_acquire_bus(*spi, portMAX_DELAY);
    if (err != ESP_OK) return err;

    spi_transaction_t t = {
        .cmd = WRITE_ACCESS | (address & ADDR_MASK),
        .length = 8,
        .flags = SPI_TRANS_USE_TXDATA,
        .tx_data = {data},
    };
    err = spi_device_polling_transmit(*spi, &t);

    spi_device_release_bus(*spi);
    return err;
}

/* Simple read */
int lgw_spi_r(spi_device_handle_t *spi, uint16_t address, uint8_t *data)
{
    spi_transaction_t t = {
        .cmd = READ_ACCESS | (address & ADDR_MASK),
        .rxlength = 8,
        .flags = SPI_TRANS_USE_RXDATA,
    };

    esp_err_t err = spi_device_polling_transmit(*spi, &t);
    if (err!= ESP_OK) return err;

    *data = t.rx_data[0];
    return ESP_OK;
}

/* Burst (multiple-byte) write */
int lgw_spi_wb(spi_device_handle_t *spi, uint16_t address, const uint8_t *data, uint16_t size)
{
    esp_err_t err;
    spi_transaction_t t;
    int size_to_do, chunk_size, offset;
    int byte_transfered = 0;

    err = spi_device_acquire_bus(*spi, portMAX_DELAY);
    if (err != ESP_OK) return err;

    memset(&t, 0, sizeof(t));
    t.cmd = WRITE_ACCESS | (address & ADDR_MASK);
    size_to_do = size;
    for(int i = 0; size_to_do > 0; ++i) {
        chunk_size = (size_to_do < LGW_BURST_CHUNK) ? size_to_do : LGW_BURST_CHUNK;
        offset = i * LGW_BURST_CHUNK;
        t.tx_buffer = (unsigned long *)(data + offset);
        t.length = chunk_size * 8;
        err = spi_device_polling_transmit(*spi, &t);
        if (err != ESP_OK)
            return err;

        byte_transfered += chunk_size;
        DEBUG_PRINTF("BURST WRITE: to trans %d # chunk %d # transferred %d \n", size_to_do, chunk_size, byte_transfered);
        size_to_do -= chunk_size; /* subtract the quantity of data already transferred */
    }

    /* TODO: check transfered bits, and determine return code */
    /*
    if (byte_transfered != size) {
        DEBUG_MSG("ERROR: SPI BURST WRITE FAILURE\n");
        return LGW_SPI_ERROR;
    } else {
        DEBUG_MSG("Note: SPI burst write success\n");
        return LGW_SPI_SUCCESS;
    }
    */

    spi_device_release_bus(*spi);
    return err;
}

/* Burst (multiple-byte) read */
int lgw_spi_rb(spi_device_handle_t *spi, uint16_t address, uint8_t *data, uint16_t size)
{
    esp_err_t err;
    spi_transaction_t t;
    int size_to_do, chunk_size, offset;
    int byte_transfered = 0;

    memset(&t, 0, sizeof(t));
    t.cmd = READ_ACCESS | (address & ADDR_MASK);

    if(size == 0)
        return ESP_OK;

    err = spi_device_acquire_bus(*spi, portMAX_DELAY);
    if (err != ESP_OK) return err;

    t.cmd = WRITE_ACCESS | (address & ADDR_MASK);
    size_to_do = size;
    for(int i = 0; size_to_do > 0; ++i) {
        chunk_size = (size_to_do < LGW_BURST_CHUNK) ? size_to_do : LGW_BURST_CHUNK;
        offset = i * LGW_BURST_CHUNK;
        t.rx_buffer = (unsigned long *)(data + offset);
        t.rxlength = chunk_size * 8;

        err = spi_device_polling_transmit(*spi, &t);
        if (err != ESP_OK)
            return err;

        byte_transfered += chunk_size;
        DEBUG_PRINTF("BURST WRITE: to trans %d # chunk %d # transferred %d \n", size_to_do, chunk_size, byte_transfered);
        size_to_do -= chunk_size; /* subtract the quantity of data already transferred */
    }

    /* TODO: check transfered bits, and determine return code */
    /*
    if (byte_transfered != size) {
        DEBUG_MSG("ERROR: SPI BURST WRITE FAILURE\n");
        return LGW_SPI_ERROR;
    } else {
        DEBUG_MSG("Note: SPI burst write success\n");
        return LGW_SPI_SUCCESS;
    }
    */

    return ESP_OK;
}
