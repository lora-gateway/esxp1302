/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2023 Semtech

Description:
    LED implementation for controlling the led to indicate packets RX/TX and backhaul status

License: Revised BSD License, see LICENSE.TXT file include in the project
*/

#include "led_indication.h"

static SemaphoreHandle_t mx_led_up;         /* control access to the counter of LED indicator of upstream */
static SemaphoreHandle_t mx_led_down;       /* control access to the counter of LED indicator of downstream */
static SemaphoreHandle_t mx_led_backhaul;   /* control access to the counter of LED indicator of backhaul */

static int16_t sUplinkCounter = 0;
static int16_t sDownlinkCounter = 0;
static int16_t sBackhaulCounter = 0;

void led_set_level(gpio_num_t gpio_num, unsigned int level){
    if (gpio_num != GPIO_NUM_NC){
        gpio_set_level( gpio_num, level );
    }
}

void vDaemonLedIndication( void )
{
    // init all mutexes
    mx_led_up = xSemaphoreCreateMutex();
    mx_led_down = xSemaphoreCreateMutex();
    mx_led_backhaul = xSemaphoreCreateMutex();

    for( ;;) {
        vTaskDelay( 30 / portTICK_PERIOD_MS);

        if ( sUplinkCounter > 0 ) {
            xSemaphoreTake(mx_led_up, portMAX_DELAY);
            sUplinkCounter --;
            xSemaphoreGive(mx_led_up);

            led_set_level( LED_GREEN_GPIO, 0 );

        } else if ( sUplinkCounter == 0 ) {
            // reset green led
            led_set_level( LED_GREEN_GPIO, 1 );

        } else {
            xSemaphoreTake(mx_led_up, portMAX_DELAY);
            sUplinkCounter = 0;
            xSemaphoreGive(mx_led_up);

            // reset green led
            led_set_level( LED_GREEN_GPIO, 1 );
        }

        if ( sDownlinkCounter > 0 ) {
            xSemaphoreTake(mx_led_down, portMAX_DELAY);
            sDownlinkCounter --;
            xSemaphoreGive(mx_led_down);

            led_set_level( LED_RED_GPIO, 0 );

        } else if ( sDownlinkCounter == 0 ) {
            // reset green led
            led_set_level( LED_RED_GPIO, 1 );

        } else {
            xSemaphoreTake(mx_led_down, portMAX_DELAY);
            sDownlinkCounter = 0;
            xSemaphoreGive(mx_led_down);

            // reset green led
            led_set_level( LED_RED_GPIO, 1 );
        }

        if ( sBackhaulCounter > 0 ) {
            xSemaphoreTake(mx_led_backhaul, portMAX_DELAY);
            sBackhaulCounter --;
            xSemaphoreGive(mx_led_backhaul);

            led_set_level( LED_BLUE_GPIO, 0 );

        } else if ( sBackhaulCounter == 0 ) {
            // reset green led
            led_set_level( LED_BLUE_GPIO, 1 );

        } else {
            xSemaphoreTake(mx_led_backhaul, portMAX_DELAY);
            sBackhaulCounter = 0;
            xSemaphoreGive(mx_led_backhaul);

            // reset green led
            led_set_level( LED_BLUE_GPIO, 1 );
        }
    }
}

void vUplinkFlash ( uint16_t period )
{
    xSemaphoreTake(mx_led_up, portMAX_DELAY);
    sUplinkCounter += period;
    xSemaphoreGive(mx_led_up);
}

void vDownlinkFlash ( uint16_t period )
{
    xSemaphoreTake(mx_led_down, portMAX_DELAY);
    sDownlinkCounter += period;
    xSemaphoreGive(mx_led_down);
}

void vBackhaulFlash ( uint16_t period )
{
    xSemaphoreTake(mx_led_backhaul, portMAX_DELAY);
    sBackhaulCounter += period;
    xSemaphoreGive(mx_led_backhaul);
}
