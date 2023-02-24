/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2023 Semtech

Description:
    Http service head file

License: Revised BSD License, see LICENSE.TXT file include in the project
*/

#ifndef _HTTP_SERVER_H
#define _HTTP_SERVER_H


#define BASIC_AUTH_USERNAME "iot"
#define BASIC_AUTH_PASSWORD "lora"

#define HTTPD_401      "401 UNAUTHORIZED"           /*!< HTTP Response 401 */

void http_server_task(void *pvParameters);

#endif
