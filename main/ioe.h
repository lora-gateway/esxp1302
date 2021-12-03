// Copyright (C) 2016-2019 Semtech (International) AG. All rights reserved.
//
// This file is subject to the terms and conditions defined in file 'LICENSE',
// which is part of this source code package.

#ifndef _ioe_h_
#define _ioe_h_

#include "driver/gpio.h"
#include "loragw_i2c.h"
#include "loragw_aux.h"

#define IOE_LED_RED         26
#define IOE_LED_GREEN       33
#define IOE_LED_BLUE        27
#define IOE_FOURCE_RST      23
#define LED_MIN_INTV_MS     50   // [ms]
#define LED_MIN_DC          10   // [%]

#define BTN_SETUP           23
#define BTN_FACTORY_RESET   25

#define ioe_set_mode(port,mode)     gpio_set_direction(port,mode)

#define ioe_get(pin)                gpio_get_level(pin)
#define ioe_set(pin,value)          gpio_set_level(pin,value)

// LED ----------------------------------

enum { LED_OFF, LED_GREEN, LED_RED, LED_RG };

void ioe_init();
void led_mode(int mode, int intv, int duty_cycle);
void oled_init();
void OLED_ShowStr(uint8_t x, uint8_t y, char ch[], uint8_t TextSize);
void OLED_OFF(void);
void OLED_ON(void);
void OLED_CLS(void);
void OLED_Fill(uint8_t fill_Data);
void OLED_SetPos(uint8_t x, uint8_t y);

#endif // _ioe_h_
