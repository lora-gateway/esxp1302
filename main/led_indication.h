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

#define LED_BLUE_GPIO   33
#define LED_GREEN_GPIO  26
#define LED_RED_GPIO    27

void vDaemonLedIndication( void );

void vUplinkFlash ( uint16_t period );
void vDownlinkFlash ( uint16_t period );
void vBackhaulFlash ( uint16_t period );

