/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2019 Semtech

Description:
    LoRa concentrator HAL common auxiliary functions

License: Revised BSD License, see LICENSE.TXT file include in the project
*/

#ifndef _WEB_CONFIG_H
#define _WEB_CONFIG_H


typedef enum {
    WIFI_USERNAME = 0,
    WIFI_PASSWORD,
    NS_HOST,
    NS_PORT,
    GW_ID,
    CONFIG_NUM,
    CONFIG_END = 254,
    CONFIG_ERR = 255
} tag_e;


tag_e name2tag(char *name);
int update_config(char *str, int len);
void dump_config(void);
void read_config(char *buf, int buf_len);
int save_config(void);
void extract_data_items(char *str);

#endif
