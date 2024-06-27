/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2019 Semtech

Description:
    LoRa concentrator HAL auxiliary functions

License: Revised BSD License, see LICENSE.TXT file include in the project
*/


/* fix an issue between POSIX and C99 */
#if __STDC_VERSION__ >= 199901L
    #define _XOPEN_SOURCE 600
#else
    #define _XOPEN_SOURCE 500
#endif

#include <stdio.h>  /* printf fprintf */
#include <math.h>  /* printf fprintf */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "loragw_aux.h"
#include "loragw_hal.h"


#if DEBUG_AUX == 1
    #define DEBUG_MSG(str)                fprintf(stdout, str)
    #define DEBUG_PRINTF(fmt, args...)    fprintf(stdout,"%s:%d: "fmt, __FUNCTION__, __LINE__, args)
#else
    #define DEBUG_MSG(str)
    #define DEBUG_PRINTF(fmt, args...)
#endif


// TODO: need to have a better implementation for this function
void wait_us(unsigned long delay_us) {
    vTaskDelay(delay_us / portTICK_PERIOD_MS / 1000 );
}


void wait_ms(unsigned long delay_ms) {
    vTaskDelay(delay_ms / portTICK_PERIOD_MS);
}


unsigned int lora_packet_time_on_air(const uint8_t bw, const uint8_t sf, const uint8_t cr, const uint16_t n_symbol_preamble,
                                 const bool no_header, const bool no_crc, const uint8_t size,
                                 double * out_nb_symbols, unsigned int * out_nb_symbols_payload, uint16_t * out_t_symbol_us) {
    uint8_t H, DE, n_bit_crc;
    uint8_t bw_pow;
    uint16_t t_symbol_us;
    double n_symbol;
    unsigned int toa_us, n_symbol_payload;

    /* Check input parameters */
    if (IS_LORA_DR(sf) == false) {
        printf("ERROR: wrong datarate - %s\n", __FUNCTION__);
        return 0;
    }
    if (IS_LORA_BW(bw) == false) {
        printf("ERROR: wrong bandwidth - %s\n", __FUNCTION__);
        return 0;
    }
    if (IS_LORA_CR(cr) == false) {
        printf("ERROR: wrong coding rate - %s\n", __FUNCTION__);
        return 0;
    }

    /* Get bandwidth 125KHz divider*/
    switch (bw) {
        case BW_125KHZ:
            bw_pow = 1;
            break;
        case BW_250KHZ:
            bw_pow = 2;
            break;
        case BW_500KHZ:
            bw_pow = 4;
            break;
        default:
            printf("ERROR: unsupported bandwith 0x%02X (%s)\n", bw, __FUNCTION__);
            return 0;
    }

    /* Duration of 1 symbol */
    t_symbol_us = (1 << sf) * 8 / bw_pow; /* 2^SF / BW , in microseconds */

    /* Packet parameters */
    H = (no_header == false) ? 1 : 0; /* header is always enabled, except for beacons */
    DE = (sf >= 11) ? 1 : 0; /* Low datarate optimization enabled for SF11 and SF12 */
    n_bit_crc = (no_crc == false) ? 16 : 0;

    /* Number of symbols in the payload */
    n_symbol_payload = ceil( MAX( (double)( 8 * size + n_bit_crc - 4*sf + ((sf >= 7) ? 8 : 0) + 20*H ), 0.0) /
                                  (double)( 4 * (sf - 2*DE)) )
                       * ( cr + 4 ); /* Explicitely cast to double to keep precision of the division */

    /* number of symbols in packet */
    n_symbol = (double)n_symbol_preamble + ((sf >= 7) ? 4.25 : 6.25) + 8.0 + (double)n_symbol_payload;

    /* Duration of packet in microseconds */
    toa_us = (unsigned int)( (double)n_symbol * (double)t_symbol_us );

    DEBUG_PRINTF("INFO: LoRa packet ToA: %u us (n_symbol:%f, t_symbol_us:%u)\n", toa_us, n_symbol, t_symbol_us);

    /* Return details if required */
    if (out_nb_symbols != NULL) {
        *out_nb_symbols = n_symbol;
    }
    if (out_nb_symbols_payload != NULL) {
        *out_nb_symbols_payload = n_symbol_payload;
    }
    if (out_t_symbol_us != NULL) {
        *out_t_symbol_us = t_symbol_us;
    }

    return toa_us;
}


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void _meas_time_start(struct timeval *tm)
{
#if (DEBUG_PERF > 0) && (DEBUG_PERF <= 5)
    gettimeofday(tm, NULL);
#endif
}

void _meas_time_stop(int debug_level, struct timeval start_time, const char *str)
{
#if (DEBUG_PERF > 0) && (DEBUG_PERF <= 5)
    struct timeval tm;
    double time_ms;
    char *indent[] = { "", " ..", " ....", " ......", " ........" };

    gettimeofday(&tm, NULL);

    time_ms = (tm.tv_sec - start_time.tv_sec) * 1000.0 + (tm.tv_usec - start_time.tv_usec) / 1000.0;
    if ((debug_level > 0) && (debug_level <= DEBUG_PERF)) {
        printf("PERF:%s %s %f ms\n", indent[debug_level - 1], str, time_ms);
    }
#endif
}
#pragma GCC diagnostic pop


void timeout_start(struct timeval * start) {
    gettimeofday(start, NULL);
}


int timeout_check(struct timeval start, unsigned int timeout_ms) {
    struct timeval tm;
    struct timeval diff;
    unsigned int ms;

    gettimeofday(&tm, NULL);

    TIMER_SUB(&tm, &start, &diff);

    ms = diff.tv_sec * 1000 + diff.tv_usec / 1000;
    if (ms >= timeout_ms) {
        return -1;
    } else {
        return 0;
    }
}
