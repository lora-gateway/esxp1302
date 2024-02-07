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


#include <stdint.h>     /* C99 types */
#include <stdio.h>      /* printf fprintf */
#include <stdlib.h>     /* malloc free */
#include <unistd.h>     /* lseek, close */
#include <fcntl.h>      /* open */
#include <string.h>     /* memset */

#include "loragw_spi.h"
#include "loragw_aux.h"


#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#if DEBUG_SPI == 1
    #define DEBUG_MSG(str)                fprintf(stdout, str)
    #define DEBUG_PRINTF(fmt, args...)    fprintf(stdout,"%s:%d: "fmt, __FUNCTION__, __LINE__, args)
    #define CHECK_NULL(a)                if(a==NULL){fprintf(stderr,"%s:%d: ERROR: NULL POINTER AS ARGUMENT\n", __FUNCTION__, __LINE__);return LGW_SPI_ERROR;}
#else
    #define DEBUG_MSG(str)
    #define DEBUG_PRINTF(fmt, args...)
    #define CHECK_NULL(a)                if(a==NULL){return LGW_SPI_ERROR;}
#endif

#define READ_ACCESS     0x0000
#define WRITE_ACCESS    0x8000
#define ADDR_MASK       0x7fff

#define PIN_NUM_MISO 12
#define PIN_NUM_MOSI 13
#define PIN_NUM_CLK  14
#define PIN_NUM_CS   15

#define DMA_CHAN    2

#define USE_SPI_TRANSACTION_EXT
//#define DEBUG_SPI


/* SPI initialization and configuration */
int lgw_spi_open(spi_device_handle_t **spi_target)
{
    esp_err_t ret;
    void *spi;

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

    spi = malloc(sizeof(spi_device_handle_t));
    if(spi == NULL){
        DEBUG_MSG("ERROR: MALLOC FAIL\n");
        return LGW_SPI_ERROR;
    }

    // Initialize the SPI bus
    ret = spi_bus_initialize(SX1302_SPI_HOST, &buscfg, DMA_CHAN);
    ESP_ERROR_CHECK(ret);

    // Attach SX1302 to the SPI bus
    ret = spi_bus_add_device(SX1302_SPI_HOST, &devcfg, spi);
    ESP_ERROR_CHECK(ret);

    *spi_target = (void *)spi;
    return LGW_SPI_SUCCESS;
}

/* SPI release */
int lgw_spi_close(spi_device_handle_t *spi)
{
    esp_err_t ret;

    CHECK_NULL(spi);
    ret = spi_bus_remove_device(*spi);
    ESP_ERROR_CHECK(ret);
    // printf("ret = %d\n", ret);

    ret = spi_bus_free(SX1302_SPI_HOST);
    ESP_ERROR_CHECK(ret);
    // printf("ret = %d\n", ret);

    free(spi);
    spi = NULL;
    return LGW_SPI_SUCCESS;
}

/* Simple write */
int lgw_spi_w(spi_device_handle_t *spi, uint8_t spi_mux_target, uint16_t address, uint8_t data)
{
    esp_err_t err;

    err = spi_device_acquire_bus(*spi, portMAX_DELAY);
    if(err != ESP_OK)
        return err;

    spi_transaction_t t = {
        .length = 8 * 4,
        .flags = SPI_TRANS_USE_TXDATA,
        .tx_data[0] = spi_mux_target,
        .tx_data[1] = ((WRITE_ACCESS | (address & ADDR_MASK)) >> 8),
        .tx_data[2] = (address & 0xFF),
        .tx_data[3] = data,
    };
    err = spi_device_polling_transmit(*spi, &t);

    spi_device_release_bus(*spi);
    return err;
}

#ifdef USE_SPI_TRANSACTION_EXT
/* Simple read */
int lgw_spi_r(spi_device_handle_t *spi, uint8_t spi_mux_target, uint16_t address, uint8_t *data)
{
    spi_transaction_ext_t et;

    memset(&et, 0, sizeof(et));
    et.command_bits = 8 * 1;
    et.address_bits = 8 * 2;
    et.base.cmd = spi_mux_target;
    et.base.addr = READ_ACCESS | (address & ADDR_MASK);
    et.base.flags = SPI_TRANS_VARIABLE_CMD | SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA;
    et.base.length = 8 * 2;
    et.base.tx_data[0] = 0x00;
    et.base.tx_data[1] = 0x00;

    esp_err_t err = spi_device_polling_transmit(*spi, (spi_transaction_t *)&et);
    if(err!= ESP_OK)
        return err;

#ifdef DEBUG_SPI
    for(int i = 0; i < 4; i++)
        printf("0x%02x ", et.base.rx_data[i]);
    printf("\n");
#endif

    *data = et.base.rx_data[1];
    return ESP_OK;
}

#else

int lgw_spi_r(spi_device_handle_t *spi, uint8_t spi_mux_target, uint16_t address, uint8_t *data)
{
    uint8_t rbuf[5];
    uint8_t tbuf[5];

    tbuf[0] = spi_mux_target;
    tbuf[1] = ((READ_ACCESS | (address & ADDR_MASK)) >> 8);
    tbuf[2] = (address & 0xFF);
    tbuf[3] = 0x00;
    tbuf[4] = 0x00;

    spi_transaction_t t = {
        .length = 40,
        .rx_buffer = rbuf,
        .tx_buffer = tbuf,
    };

    esp_err_t err = spi_device_polling_transmit(*spi, &t);
    if(err!= ESP_OK)
        return err;

#ifdef DEBUG_SPI
    for(int i = 0; i < 5; i++)
        printf("0x%02x ", rbuf[i]);
    printf("\n");
#endif

    *data = rbuf[4];
    return ESP_OK;
}
#endif

/* Burst (multiple-byte) write */
int lgw_spi_wb(spi_device_handle_t *spi, uint8_t spi_mux_target, uint16_t address, const uint8_t *data, uint16_t size)
{
    esp_err_t err;
    spi_transaction_ext_t et;
    int size_to_do, chunk_size, offset;
    int byte_transfered = 0;
    uint8_t rbuf[LGW_BURST_CHUNK] = {0x00};

    err = spi_device_acquire_bus(*spi, portMAX_DELAY);
    if(err != ESP_OK)
        return err;

    memset(&et, 0, sizeof(et));
    et.command_bits = 8;
    et.address_bits = 16;
    et.base.cmd = spi_mux_target;
    et.base.addr = WRITE_ACCESS | (address & ADDR_MASK);
    et.base.flags = SPI_TRANS_VARIABLE_CMD | SPI_TRANS_VARIABLE_ADDR;
    et.base.rx_buffer = rbuf;

    size_to_do = size;
    for(int i = 0; size_to_do > 0; ++i) {
        chunk_size = (size_to_do < LGW_BURST_CHUNK) ? size_to_do : LGW_BURST_CHUNK;
        offset = i * LGW_BURST_CHUNK;
        et.base.tx_buffer = (unsigned long *)(data + offset);
        et.base.length = chunk_size * 8;
        et.base.rxlength = chunk_size * 8;
        err = spi_device_polling_transmit(*spi, (spi_transaction_t *)&et);
        if(err != ESP_OK)
            return err;

        byte_transfered += chunk_size;
        DEBUG_PRINTF("BURST WRITE: to trans %d # chunk %d # transferred %d \n", size_to_do, chunk_size, byte_transfered);
        size_to_do -= chunk_size;
    }

    /* TODO: check transfered bits, and determine return code */
    /*
    if(byte_transfered != size) {
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
int lgw_spi_rb(spi_device_handle_t *spi, uint8_t spi_mux_target, uint16_t address, uint8_t *data, uint16_t size)
{
    esp_err_t err;
    spi_transaction_ext_t et;
    int size_to_do, chunk_size, offset;
    int byte_transfered = 0;
    uint8_t tbuf[LGW_BURST_CHUNK] = {0x00};

    err = spi_device_acquire_bus(*spi, portMAX_DELAY);
    if(err != ESP_OK)
        return err;

    memset(&et, 0, sizeof(et));
    et.command_bits = 8;
    et.address_bits = 8 * 3;
    et.base.cmd = spi_mux_target;
    et.base.addr = ((READ_ACCESS | (address & ADDR_MASK)) << 8) | 0x00;
    et.base.flags = SPI_TRANS_VARIABLE_CMD | SPI_TRANS_VARIABLE_ADDR;
    //et.base.tx_data[0] = 0x00;
    et.base.tx_buffer = tbuf;
    //et.base.length = 8;

    size_to_do = size;
    for(int i = 0; size_to_do > 0; ++i) {
        chunk_size = (size_to_do < LGW_BURST_CHUNK) ? size_to_do : LGW_BURST_CHUNK;
        offset = i * LGW_BURST_CHUNK;
        et.base.rx_buffer = (unsigned long *)(data + offset);
        et.base.length = chunk_size * 8;
        et.base.rxlength = chunk_size * 8;
        err = spi_device_polling_transmit(*spi, (spi_transaction_t *)&et);
        if(err != ESP_OK)
            return err;

        byte_transfered += chunk_size;
        DEBUG_PRINTF("BURST WRITE: to trans %d # chunk %d # transferred %d \n", size_to_do, chunk_size, byte_transfered);
        size_to_do -= chunk_size;
    }

    /* TODO: check transfered bits, and determine return code */
    /*
    if(byte_transfered != size) {
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

/* Burst (multiple-byte) write for radio */
int radio_spi_wb(spi_device_handle_t *spi, uint8_t spi_mux_target, uint8_t op_code, const uint8_t *data, uint16_t size)
{
    esp_err_t err;
    spi_transaction_ext_t et;
    int cmd_size = 2; /* header + op_code */
    uint8_t rbuf[LGW_BURST_CHUNK] = {0x00};

    if(cmd_size + size > LGW_BURST_CHUNK) {
        DEBUG_PRINTF("size (%d) > LGW_BURST_CHUNK - %d, which is too big!\n", size, cmd_size);
        return LGW_SPI_ERROR;
    }

    err = spi_device_acquire_bus(*spi, portMAX_DELAY);
    if(err != ESP_OK)
        return err;

    memset(&et, 0, sizeof(et));
    et.command_bits = 8;
    et.address_bits = 8;
    et.base.cmd = spi_mux_target;
    et.base.addr = WRITE_ACCESS | (op_code & ADDR_MASK);
    et.base.flags = SPI_TRANS_VARIABLE_CMD | SPI_TRANS_VARIABLE_ADDR;
    et.base.rx_buffer = rbuf;

    et.base.tx_buffer = (unsigned long *)data;
    et.base.length = size * 8;
    et.base.rxlength = size * 8;
    err = spi_device_polling_transmit(*spi, (spi_transaction_t *)&et);
    if(err != ESP_OK)
        return err;

    spi_device_release_bus(*spi);
    return err;
}

/* Burst (multiple-byte) read for radio */
int radio_spi_rb(spi_device_handle_t *spi, uint8_t spi_mux_target, uint8_t op_code, uint8_t *data, uint16_t size)
{
    esp_err_t err;
    spi_transaction_ext_t et;
    int cmd_size = 2; /* header + op_code */
    uint8_t tbuf[LGW_BURST_CHUNK] = {0x00};

    if(cmd_size + size > LGW_BURST_CHUNK) {
        DEBUG_PRINTF("size (%d) > LGW_BURST_CHUNK - %d, which is too big!\n", size, cmd_size);
        return LGW_SPI_ERROR;
    }

    err = spi_device_acquire_bus(*spi, portMAX_DELAY);
    if(err != ESP_OK)
        return err;

    memset(&et, 0, sizeof(et));
    et.command_bits = 8;
    et.address_bits = 8 * 2;
    et.base.cmd = spi_mux_target;
    et.base.addr = ((READ_ACCESS | (op_code & ADDR_MASK)) << 8) | 0x00;
    et.base.flags = SPI_TRANS_VARIABLE_CMD | SPI_TRANS_VARIABLE_ADDR;
    et.base.tx_buffer = tbuf;

    et.base.rx_buffer = (unsigned long *)data;
    et.base.length = size * 8;
    et.base.rxlength = size * 8;
    err = spi_device_polling_transmit(*spi, (spi_transaction_t *)&et);
    if(err != ESP_OK)
        return err;

    spi_device_release_bus(*spi);
    return err;
}


uint16_t lgw_spi_chunk_size(void) {
    return (uint16_t)LGW_BURST_CHUNK;
}
