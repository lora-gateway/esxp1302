/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2023 Semtech

Description:
    The command line interface for all test programs.

License: Revised BSD License, see LICENSE.TXT file include in the project
*/

#include "esp_console.h"
#include "test_libloragw.h"


void app_main(void)
{
    // start command line
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    repl_config.task_stack_size = 4096 * 2;
    repl_config.prompt = "ESXP1302_GW>";

    register_test_loragw_toa();
    register_test_loragw_hal_tx();
    register_test_loragw_hal_rx();

    // initialize console REPL environment
    esp_console_repl_t *repl = NULL;
    esp_console_dev_uart_config_t uart_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&uart_config, &repl_config, &repl));
    ESP_ERROR_CHECK(esp_console_start_repl(repl));
}
