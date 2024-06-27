/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2023 Semtech

Description:
    Head file for processing configure settings

License: Revised BSD License, see LICENSE.TXT file include in the project
*/


#ifndef _WEB_CONFIG_H
#define _WEB_CONFIG_H

#include "esp_log.h"
#include "esp_err.h"


#define NTP_SERVER_ADDR  "cn.pool.ntp.org"

typedef enum {
    WIFI_SSID  = 0,
    WIFI_PASSWORD,
    NS_HOST,
    NS_PORT,
    GW_ID,
    WIFI_MODE,
    FREQ_REGION,
    FREQ_RADIO0,
    FREQ_RADIO1,
    NTP_SERVER,
    CONFIG_NUM,
    CONFIG_END = 254,
    CONFIG_ERR = 255
} tag_e;

typedef enum {
    WIFI_MODE_SOFT_AP  = 0,
    WIFI_MODE_STATION
} wifi_mode_e;

typedef struct {
    tag_e tag;
    char name[16];
    char *val;
    int len;  // length of the string pointed by val
} config_s;

tag_e name2tag(char *name);
esp_err_t init_config_storage(void);
int update_config(char *str, int len);
void dump_config(void);
esp_err_t read_config(void);
int save_config(void);
void extract_data_items(char *str);
int config_wifi_mode(wifi_mode_e mode);

#endif
