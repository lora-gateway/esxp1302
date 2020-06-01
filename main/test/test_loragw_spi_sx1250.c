/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2019 Semtech

Description:
    Minimum test program for the sx1250 module

License: Revised BSD License, see LICENSE.TXT file include in the project
*/


/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */

/* Fix an issue between POSIX and C99 */
#if __STDC_VERSION__ >= 199901L
    #define _XOPEN_SOURCE 600
#else
    #define _XOPEN_SOURCE 500
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "loragw_spi.h"
#include "loragw_aux.h"
#include "loragw_reg.h"
#include "loragw_hal.h"
#include "loragw_sx1250.h"
#include "loragw_sx1302.h"


#define BUFF_SIZE           16


void app_main(void)
{
    uint8_t test_buff[BUFF_SIZE];
    uint8_t read_buff[BUFF_SIZE];
    uint32_t test_val, read_val;
    int cycle_number = 0;
    int x;

    for(int i = 0; i < 5; i++){
        printf("waiting %d...\n", i+1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("\n!!! Note !!!\nPlease Reset SX1302 board first to run this test.\n");
    printf("You can just power off then power on the whole system\n\n");

    for(int i = 5; i > 0; i--){
        printf("waiting %d...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }


    x = lgw_connect();
    if (x != LGW_REG_SUCCESS) {
        printf("ERROR: Failed to connect to the concentrator using SPI\n");
        return;
    }

    /* Reset radios */
    for (i = 0; i < LGW_RF_CHAIN_NB; i++) {
        sx1302_radio_reset(i, LGW_RADIO_TYPE_SX1250);
        sx1302_radio_set_mode(i, LGW_RADIO_TYPE_SX1250);
    }

    /* Select the radio which provides the clock to the sx1302 */
    sx1302_radio_clock_select(0);

    /* Ensure we can control the radio */
    lgw_reg_w(SX1302_REG_COMMON_CTRL0_HOST_RADIO_CTRL, 0x01);

    /* Ensure PA/LNA are disabled */
    lgw_reg_w(SX1302_REG_AGC_MCU_CTRL_FORCE_HOST_FE_CTRL, 1);
    lgw_reg_w(SX1302_REG_AGC_MCU_RF_EN_A_PA_EN, 0);
    lgw_reg_w(SX1302_REG_AGC_MCU_RF_EN_A_LNA_EN, 0);

    /* Set Radio in Standby mode */
    test_buff[0] = (uint8_t)STDBY_XOSC;
    sx1250_write_command(0, SET_STANDBY, test_buff, 1);
    sx1250_write_command(1, SET_STANDBY, test_buff, 1);
    wait_ms(10);

    test_buff[0] = 0x00;
    sx1250_read_command(0, GET_STATUS, test_buff, 1);
    printf("Radio0: get_status: 0x%02X\n", test_buff[0]);
    sx1250_read_command(1, GET_STATUS, test_buff, 1);
    printf("Radio1: get_status: 0x%02X\n", test_buff[0]);

    /* databuffer R/W stress test */
    while(cycle_number < 20) {
        test_buff[0] = rand() & 0x7F;
        test_buff[1] = rand() & 0xFF;
        test_buff[2] = rand() & 0xFF;
        test_buff[3] = rand() & 0xFF;
        test_val = (test_buff[0] << 24) | (test_buff[1] << 16) | (test_buff[2] << 8) | (test_buff[3] << 0);
        sx1250_write_command(0, SET_RF_FREQUENCY, test_buff, 4);

        read_buff[0] = 0x08;
        read_buff[1] = 0x8B;
        read_buff[2] = 0x00;
        read_buff[3] = 0x00;
        read_buff[4] = 0x00;
        read_buff[5] = 0x00;
        read_buff[6] = 0x00;
        sx1250_read_command(0, READ_REGISTER, read_buff, 7);
        read_val = (read_buff[3] << 24) | (read_buff[4] << 16) | (read_buff[5] << 8) | (read_buff[6] << 0);

        printf("Cycle %i > ", cycle_number);
        if (read_val != test_val) {
            printf("error during the buffer comparison\n");
            printf("Written value: %08X\n", test_val);
            printf("Read value:    %08X\n", read_val);
            return;
        } else {
            printf("did a %i-byte R/W on a register with no error\n", 4);
            ++cycle_number;
        }

    }

    lgw_disconnect();
    printf("End of test for loragw_spi_sx1250.c\n");

    return;
}
