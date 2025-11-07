/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2019 Semtech

Description:
    Functions to reset LoRa concentrator from GPIO Pins.

License: Revised BSD License, see LICENSE.TXT file include in the project
*/

#include "loragw_gpio.h"
#include "loragw_aux.h"


void lgw_reset(void)
{
    gpio_config_t gpio_conf;
    gpio_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_conf.mode = GPIO_MODE_OUTPUT;
    gpio_conf.pin_bit_mask = SX1302_GPIO_PIN_SEL;
    gpio_conf.pull_down_en = 0;
    gpio_conf.pull_up_en = 0;
    gpio_config(&gpio_conf);
    if (SX1302_POWER_EN_PIN != GPIO_NUM_NC) {
      gpio_set_level(SX1302_POWER_EN_PIN, 1);
      wait_ms(100);
    }
    gpio_set_level(SX1302_RESET_PIN, 1);
    wait_ms(100);
    gpio_set_level(SX1302_RESET_PIN, 0);
    wait_ms(100);
}
