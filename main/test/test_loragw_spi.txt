/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2020 Semtech

Description:
    Minimum test program for the loragw_spi module

License: Revised BSD License, see LICENSE.TXT file include in the project
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "loragw_spi.h"


#define BUFF_SIZE           1024*4

#define SX1302_AGC_MCU_MEM  0x0000
#define SX1302_REG_COMMON   0x5600
#define SX1302_REG_AGC_MCU  0x5780


uint8_t test_buff[BUFF_SIZE];
uint8_t read_buff[BUFF_SIZE];

void app_main(void)
{
    uint8_t data = 0;
    int cycle_number = 0;
    int i;
    uint16_t size;

    spi_device_handle_t spi;

    printf("Beginning of test for loragw_spi.c\n");

    i = lgw_spi_open(&spi);
    if (i != 0) {
        printf("ERROR: failed to open SPI device\n");
        return;
    }

    lgw_spi_r(&spi, SX1302_REG_COMMON + 6, &data);
    printf("SX1302 version: 0x%02X\n", data);

    lgw_spi_r(&spi, SX1302_REG_AGC_MCU + 0, &data);
    lgw_spi_w(&spi, SX1302_REG_AGC_MCU + 0, 0x06); /* mcu_clear, host_prog */

    /* databuffer R/W stress test */
    for(cycle_number = 0; cycle_number < 100; cycle_number++){
        size = rand() % BUFF_SIZE;
        for (i = 0; i < size; ++i) {
            //test_buff[i] = rand() & 0xFF;
            test_buff[i] = i & 0xFF;
        }
        printf("Cycle %i (size: %d) > ", cycle_number, size);
        lgw_spi_wb(&spi, SX1302_AGC_MCU_MEM, test_buff, size);
        lgw_spi_rb(&spi, SX1302_AGC_MCU_MEM, read_buff, size);

        for (i = 0; ((i<size) && (test_buff[i] == read_buff[i])); ++i);
        if (i != size) {
            printf("error during the buffer comparison\n");
            printf("Written values:\n");
            for (i=0; i<size; ++i) {
                printf(" %02X ", test_buff[i]);
                if (i%16 == 15)
                    printf("\n");
            }
            printf("\n");
            printf("Read values:\n");
            for (i=0; i<size; ++i) {
                printf(" %02X ", read_buff[i]);
                if (i%16 == 15)
                    printf("\n");
            }
            printf("\n");
            break;
            //return EXIT_FAILURE;
        } else {
            printf("did a %i-byte R/W on a data buffer with no error\n", size);
            ++cycle_number;
        }
    }

    lgw_spi_close(&spi);
    printf("End of test for loragw_spi.c\n");

    while(true){
        printf("end\n");
        vTaskDelay(8000 / portTICK_PERIOD_MS);
    }

    return;
}
