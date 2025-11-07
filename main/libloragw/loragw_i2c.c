/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2019 Semtech

Description:
    Host specific functions to address the LoRa concentrator I2C peripherals.

License: Revised BSD License, see LICENSE.TXT file include in the project
*/


#include <stdint.h>     /* C99 types */
#include <stdio.h>      /* printf fprintf */
#include <unistd.h>     /* lseek, close */
#include <fcntl.h>      /* open */
#include <errno.h>      /* errno */

#include "loragw_i2c.h"
#include "loragw_aux.h"


#if DEBUG_I2C == 1
    #define DEBUG_MSG(str)                fprintf(stdout, str)
    #define DEBUG_PRINTF(fmt, args...)    fprintf(stdout,"%s:%d: "fmt, __FUNCTION__, __LINE__, args)
    #define CHECK_NULL(a)                if(a==NULL){fprintf(stderr,"%s:%d: ERROR: NULL POINTER AS ARGUMENT\n", __FUNCTION__, __LINE__);return LGW_I2C_ERROR;}
#else
    #define DEBUG_MSG(str)
    #define DEBUG_PRINTF(fmt, args...)
    #define CHECK_NULL(a)                if(a==NULL){return LGW_I2C_ERROR;}
#endif


static i2c_port_t i2c_num = I2C_MASTER_NUM;
static bool i2c_opened = false;

esp_err_t i2c_esp32_open(void)
{
    i2c_config_t conf;
    int i2c_master_port = i2c_num;
    esp_err_t ret;

    if(i2c_opened == true)
        return ESP_OK;

    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    conf.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;
    i2c_param_config(i2c_master_port, &conf);

    ret = i2c_driver_install(i2c_master_port, conf.mode,
            I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
    if(ret == ESP_OK){
        DEBUG_PRINTF("INFO: I2C port(%d) opened successfully\n", i2c_num);
        i2c_opened = true;
    } else {
        DEBUG_PRINTF("INFO: I2C port(%d) opened failed\n", i2c_num);
    }

    return ret;
}

esp_err_t i2c_esp32_read(uint8_t device_addr, uint8_t reg_addr, uint8_t *data)
{
    esp_err_t ret;

    // write first, provide device address and register address
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);

    // now change to read, and save one byte to 'data'
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_READ, ACK_CHECK_EN);
    i2c_master_read_byte(cmd, data, NACK_VAL);
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

esp_err_t i2c_esp32_write(uint8_t device_addr, uint8_t reg_addr, uint8_t data)
{
    uint8_t buf[2];

    buf[0] = reg_addr;
    buf[1] = data;

    return i2c_esp32_write_buf(device_addr, buf, 2);
}

esp_err_t i2c_esp32_write_buf(uint8_t device_addr, uint8_t *data, size_t size)
{
    esp_err_t ret;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write(cmd, data, size, ACK_CHECK_EN);
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

esp_err_t i2c_esp32_close(void)
{
    if(i2c_opened == true)
        return i2c_driver_delete(i2c_num);
    else
        return ESP_OK;
}
