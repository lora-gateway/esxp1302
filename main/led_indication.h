/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2023 Semtech

Description:
    LED head file for controlling the led to indicate packets RX/TX and backhaul status

License: Revised BSD License, see LICENSE.TXT file include in the project
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <stdint.h>
#include "driver/gpio.h"

#ifndef LED_BLUE_GPIO
#define LED_BLUE_GPIO   33
#endif

#ifndef LED_GREEN_GPIO
#define LED_GREEN_GPIO  26
#endif

#ifndef LED_RED_GPIO
#define LED_RED_GPIO    27
#endif

void vDaemonLedIndication( void );

void vUplinkFlash ( uint16_t period );
void vDownlinkFlash ( uint16_t period );
void vBackhaulFlash ( uint16_t period );

