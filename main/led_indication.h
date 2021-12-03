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

