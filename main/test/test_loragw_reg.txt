/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2019 Semtech

Description:
    Minimum test program for the loragw_reg module

License: Revised BSD License, see LICENSE.TXT file include in the project
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"

#include "loragw_spi.h"
#include "loragw_reg.h"


extern const struct lgw_reg_s loregs[LGW_TOTALREGS+1];
uint8_t rand_values[LGW_TOTALREGS];
bool reg_ignored[LGW_TOTALREGS]; /* store register to be ignored */

void app_main(void)
{
    int x, i;
    int32_t val;
    bool error_found = false;
    uint8_t reg_val;
    uint8_t reg_max;

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
        printf("ERROR: failed to connect\n");
        return;
    }

    /* The following registers cannot be tested this way */
    memset(reg_ignored, 0, sizeof reg_ignored);
    reg_ignored[SX1302_REG_COMMON_CTRL0_CLK32_RIF_CTRL] = true; /* all test fails if we set this one to 1 */

    /* Test 1: read all registers and check default value for non-read-only registers */
    printf("## TEST#1: read all registers and check default value for non-read-only registers\n");
    error_found = false;
    for (i = 0; i < LGW_TOTALREGS; i++) {
        if (loregs[i].rdon == 0) {
            x = lgw_reg_r(i, &val);
            if (x != LGW_REG_SUCCESS) {
                printf("ERROR: failed to read register at index %d\n", i);
                return;
            }
            if (val != loregs[i].dflt) {
                printf("ERROR: default value for register at index %d is %d, should be %d\n", i, val, loregs[i].dflt);
                error_found = true;
            }
        }
    }
    printf("------------------\n");
    printf(" TEST#1 %s\n", (error_found == false) ? "PASSED" : "FAILED");
    printf("------------------\n\n");

    /* Test 2: read/write test on all non-read-only, non-pulse, non-w0clr, non-w1clr registers */
    printf("## TEST#2: read/write test on all non-read-only, non-pulse, non-w0clr, non-w1clr registers\n");
    /* Write all registers with a random value */
    error_found = false;
    for (i = 0; i < LGW_TOTALREGS; i++) {
        if ((loregs[i].rdon == 0) && (reg_ignored[i] == false)) {
            /* Peek a random value different form the default reg value */
            reg_max = pow(2, loregs[i].leng) - 1;
            if (loregs[i].leng == 1) {
                reg_val = !loregs[i].dflt;
            } else {
                /* ensure random value is not the default one */
                do {
                    if (loregs[i].sign == 1) {
                        reg_val = rand() % (reg_max / 2);
                    } else {
                        reg_val = rand() % reg_max;
                    }
                } while (reg_val == loregs[i].dflt);
            }
            /* Write selected value */
            x = lgw_reg_w(i, reg_val);
            if (x != LGW_REG_SUCCESS) {
                printf("ERROR: failed to read register at index %d\n", i);
                return;
            }
            /* store value for later check */
            rand_values[i] = reg_val;
        }
    }
    /* Read all registers and check if we got proper random value back */
    for (i = 0; i < LGW_TOTALREGS; i++) {
        if ((loregs[i].rdon == 0) && (loregs[i].chck == 1) && (reg_ignored[i] == false)) {
            x = lgw_reg_r(i, &val);
            if (x != LGW_REG_SUCCESS) {
                printf("ERROR: failed to read register at index %d\n", i);
                return;
            }
            /* check value */
            if (val != rand_values[i]) {
                printf("ERROR: value read from register at index %d differs from the written value (w:%u r:%d)\n", i, rand_values[i], val);
                error_found = true;
            } else {
                //printf("INFO: MATCH reg %d (%u, %u)\n", i, rand_values[i], (uint8_t)val);
            }
        }
    }
    printf("------------------\n");
    printf(" TEST#2 %s\n", (error_found == false) ? "PASSED" : "FAILED");
    printf("------------------\n\n");

    x = lgw_disconnect();
    if (x != LGW_REG_SUCCESS) {
        printf("ERROR: failed to disconnect\n");
        return;
    }
    printf("Tests for Registers are done\n");

    while(true){
        printf("hello\n");
        vTaskDelay(8000 / portTICK_PERIOD_MS);
    }

    return;
}
