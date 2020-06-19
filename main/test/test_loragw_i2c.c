/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2019 Semtech

Description:
    Minimum test program for the loragw_i2c module

License: Revised BSD License, see LICENSE.TXT file include in the project
*/


/* Fix an issue between POSIX and C99 */
#if __STDC_VERSION__ >= 199901L
    #define _XOPEN_SOURCE 600
#else
    #define _XOPEN_SOURCE 500
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     /* getopt, access */
#include <time.h>

#include "loragw_i2c.h"
#include "loragw_stts751.h"
#include "loragw_aux.h"
#include "loragw_hal.h"


#define STTS751_REG_TEMP_H      0x00
#define STTS751_REG_TEMP_L      0x02
#define STTS751_REG_CONF        0x03
#define STTS751_REG_RATE        0x04
#define STTS751_REG_PROD_ID     0xFD
#define STTS751_REG_MAN_ID      0xFE
#define STTS751_REG_REV_ID      0xFF

#define STTS751_0_PROD_ID       0x00
#define STTS751_1_PROD_ID       0x01
#define ST_MAN_ID               0x53


void app_main(void)
{
    int i, err;
    uint8_t val;
    uint8_t high_byte, low_byte;
    int8_t h;
    float temperature;

    printf( "+++ Start of I2C test program +++\n" );

    // I've two choices to skip when something goes wrong before the 'for()' loop:
    //   1. use 'goto'.
    //   2. put all of i2c_esp32_open/read/write() in a big while loop and use 'break'.
    // Since I don't like add too many indents, I choose the 1st one here.

    /* Open I2C port expander */
    err = i2c_esp32_open();
    if (err != 0)
    {
        printf( "ERROR: failed to open I2C port(err=%i)\n", err);
        goto out;
    }

    /* Get temperature sensor product ID */
    err = i2c_esp32_read(I2C_PORT_STTS751, STTS751_REG_PROD_ID, &val);
    if ( err != 0 )
    {
        printf( "ERROR: failed to read I2C device 0x%x (err=%i)\n", I2C_PORT_STTS751, err );
        goto out;
    }
    switch( val )
    {
        case STTS751_0_PROD_ID:
            printf("INFO: Product ID: STTS751-0\n");
            break;
        case STTS751_1_PROD_ID:
            printf("INFO: Product ID: STTS751-1\n");
            break;
        default:
            printf("ERROR: Product ID: UNKNOWN\n");
            goto out;
    }

    /* Get temperature sensor  Manufacturer ID */
    err = i2c_esp32_read(I2C_PORT_STTS751, STTS751_REG_MAN_ID, &val );
    if ( err != 0 )
    {
        printf( "ERROR: failed to read I2C device 0x%x (err=%i)\n", I2C_PORT_STTS751, err );
        goto out;
    }
    if ( val != ST_MAN_ID )
    {
        printf( "ERROR: Manufacturer ID: UNKNOWN\n" );
        goto out;
    }
    else
    {
        printf("INFO: Manufacturer ID: 0x%02X\n", val);
    }

    /* Get temperature sensor  revision number */
    err = i2c_esp32_read(I2C_PORT_STTS751, STTS751_REG_REV_ID, &val);
    if ( err != 0 )
    {
        printf( "ERROR: failed to read I2C device 0x%x (err=%i)\n", I2C_PORT_STTS751, err );
        goto out;
    }
    printf("INFO: Revision number: 0x%02X\n", val);

    /* Set conversion resolution to 12 bits */
    err = i2c_esp32_write(I2C_PORT_STTS751, STTS751_REG_CONF, 0x8C); /* TODO: do not hardcode the whole byte */
    if ( err != 0 )
    {
        printf( "ERROR: failed to write I2C device 0x%02X (err=%i)\n", I2C_PORT_STTS751, err );
        goto out;
    }

    /* Set conversion rate to 1 / second */
    err = i2c_esp32_write(I2C_PORT_STTS751, STTS751_REG_RATE, 0x04);
    if ( err != 0 )
    {
        printf( "ERROR: failed to write I2C device 0x%02X (err=%i)\n", I2C_PORT_STTS751, err );
        goto out;
    }

    for(i=0; i<100; i++) {
        /* Read Temperature LSB */
        err = i2c_esp32_read(I2C_PORT_STTS751, STTS751_REG_TEMP_L, &low_byte);
        if ( err != 0 )
        {
            printf( "ERROR: failed to read I2C device 0x%02X (err=%i)\n", I2C_PORT_STTS751, err );
            break;
        }

        /* Read Temperature MSB */
        err = i2c_esp32_read(I2C_PORT_STTS751, STTS751_REG_TEMP_H, &high_byte);
        if ( err != 0 )
        {
            printf( "ERROR: failed to read I2C device 0x%02X (err=%i)\n", I2C_PORT_STTS751, err );
            break;
        }

        h = (int8_t)high_byte;
        temperature =  ((h << 8) | low_byte) / 256.0;

        printf( "Temperature: %f C (h:0x%02X l:0x%02X)\n", temperature, high_byte, low_byte );
        wait_ms( 1000 );
    }

    /* Terminate */
    printf( "+++ End of I2C test program +++\n" );

    err = i2c_esp32_close();
    if ( err != 0 )
    {
        printf( "ERROR: failed to close I2C device (err=%i)\n", err );
    }

out:
    while(true){
        printf("end\n");
        vTaskDelay(8000 / portTICK_PERIOD_MS);
    }

    return;
}
