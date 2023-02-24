/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2023 Semtech

Description:
    The (older) SPI function test

License: Revised BSD License, see LICENSE.TXT file include in the project
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"


#define PIN_NUM_MISO 12
#define PIN_NUM_MOSI 13
#define PIN_NUM_CLK  14
#define PIN_NUM_CS   15

#define SX1302_SPI_HOST    HSPI_HOST
#define DMA_CHAN    2

// get sx1302 version
uint8_t spi_get_version(spi_device_handle_t spi, const uint8_t *buf, int len)
{
    esp_err_t ret;
    spi_transaction_t t;
    uint8_t rbuf[16];

    memset(&t, 0, sizeof(t));
    t.length = 8 * len;
    t.tx_buffer = buf;
    t.rx_buffer = rbuf;

    ret = spi_device_polling_transmit(spi, &t);
    assert(ret == ESP_OK);

    // printf("version: 0x%x\n", rbuf[4]);
    return rbuf[4];
}


void app_main(void)
{
    uint8_t version;
    esp_err_t ret;
    spi_device_handle_t spi;
    uint8_t buf[] = {0x00, 0x56, 0x06, 0x00, 0x00};

    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 10*1000*1000,   // clock = 10 MHz
        .mode = 0,
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 8,
    };

    printf("Testing ESXP1302 SPI...\n");

    for(int i = 0; i < 6; i++){
        printf("waiting %d...\n", i+1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    // Initialize the SPI bus
    ret = spi_bus_initialize(SX1302_SPI_HOST, &buscfg, DMA_CHAN);
    ESP_ERROR_CHECK(ret);

    // Attach SX1302 to the SPI bus
    ret = spi_bus_add_device(SX1302_SPI_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);

    // Get SX1302 Versioin
    version = spi_get_version(spi, buf, 5);
    printf("version = 0x%x\n", version);

    while(true){
        printf("hello\n");
        vTaskDelay(8000 / portTICK_PERIOD_MS);
    }
}
