/* SPI Testing for ESXP1302 Pkt-Fwd

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
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


// get sx1302 version
uint8_t spi_get_version(spi_device_handle_t *spi)
{
    esp_err_t ret;
    spi_transaction_t t;
    uint8_t buf[] = {0x00, 0x56, 0x06, 0x00, 0x00};
    uint8_t rbuf[16];

    memset(&t, 0, sizeof(t));
    t.length = 8 * 5;
    t.tx_buffer = buf;
    t.rx_buffer = rbuf;

    ret = spi_device_polling_transmit(*spi, &t);
    assert(ret == ESP_OK);

    // printf("version: 0x%x\n", rbuf[4]);
    return rbuf[4];
}


void app_main(void)
{
    spi_device_handle_t spi;
    uint8_t version;

    printf("Testing ESXP1302 SPI...\n");

    for(int i = 0; i < 2; i++){
        printf("waiting %d...\n", i+1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    lgw_spi_open(&spi);

    // Get SX1302 Versioin
    version = spi_get_version(&spi);
    printf("version = 0x%x\n", version);

    while(true){
        printf("hello\n");
        vTaskDelay(8000 / portTICK_PERIOD_MS);
    }
}
