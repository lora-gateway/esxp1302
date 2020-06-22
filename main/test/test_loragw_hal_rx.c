/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2019 Semtech

Description:
    Minimum test program for HAL RX capability

License: Revised BSD License, see LICENSE.TXT file include in the project
*/


/* fix an issue between POSIX and C99 */
#if __STDC_VERSION__ >= 199901L
    #define _XOPEN_SOURCE 600
#else
    #define _XOPEN_SOURCE 500
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>

#include "esp_console.h"
#include "argtable3/argtable3.h"

#include "loragw_hal.h"
#include "loragw_reg.h"
#include "loragw_gpio.h"
#include "loragw_aux.h"


#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define RAND_RANGE(min, max) (rand() % (max + 1 - min) + min)

#define DEFAULT_FREQ_HZ     868500000U


// Global variables to simplify arguments parsing
uint32_t fa = DEFAULT_FREQ_HZ;
uint32_t fb = DEFAULT_FREQ_HZ;
uint8_t clocksource = 0;
lgw_radio_type_t radio_type = LGW_RADIO_TYPE_SX1250;
bool single_input_mode = false;
uint8_t max_rx_pkt = 16;
float rssi_offset = 0.0;
uint8_t channel_mode = 0; /* LoRaWAN-like */
unsigned long nb_loop = 0;

void usage(void) {
    //printf("Library version information: %s\n", lgw_version_info());
    printf("\n\n ---- test_loragw_hal_rx ----\n");
    printf("\nAvailable options:\n");
    printf(" -h            print this help\n");
    printf(" -k <uint>     Concentrator clock source (Radio A or Radio B) [0..1]\n");
    printf(" -r <uint>     Radio type (1255, 1257, 1250)\n");
    printf(" -a <float>    Radio A RX frequency in MHz\n");
    printf(" -b <float>    Radio B RX frequency in MHz\n");
    printf(" -o <float>    RSSI Offset to be applied in dB\n");
    printf(" -n <uint>     Number of packet received with CRC OK for each HAL start/stop loop\n");
    printf(" -z <uint>     Size of the RX packet array to be passed to lgw_receive()\n");
    printf(" -m <uint>     Channel frequency plan mode [0:LoRaWAN-like, 1:Same frequency for all channels (-400000Hz on RF0)]\n");
    printf(" -j            Set radio in single input mode (SX1250 only)\n");
}

int test_hal_rx_main(void)
{
    int i, j, x;
    struct lgw_conf_board_s boardconf;
    struct lgw_conf_rxrf_s rfconf;
    struct lgw_conf_rxif_s ifconf;

    unsigned long nb_pkt_crc_ok = 0, cnt_loop;
    int nb_pkt;


    const int32_t channel_if_mode0[9] = {
        -400000,
        -200000,
        0,
        -400000,
        -200000,
        0,
        -400000,
        -200000,
        -400000 /* lora service */
    };

    const int32_t channel_if_mode1[9] = {
        -400000,
        -400000,
        -400000,
        -400000,
        -400000,
        -400000,
        -400000,
        -400000,
        -400000 /* lora service */
    };

    const uint8_t channel_rfchain_mode0[9] = { 1, 1, 1, 0, 0, 0, 0, 0, 1 };
    const uint8_t channel_rfchain_mode1[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    /* parse command line options are done in do_hal_config_cmd() */

    printf("===== sx1302 HAL RX test =====\n");

    /* Configure the gateway */
    memset( &boardconf, 0, sizeof boardconf);
    boardconf.lorawan_public = true;
    boardconf.clksrc = clocksource;
    boardconf.full_duplex = false;
    if (lgw_board_setconf(&boardconf) != LGW_HAL_SUCCESS) {
        printf("ERROR: failed to configure board\n");
        return EXIT_FAILURE;
    }

    /* set configuration for RF chains */
    memset( &rfconf, 0, sizeof rfconf);
    rfconf.enable = true;
    rfconf.freq_hz = fa;
    rfconf.type = radio_type;
    rfconf.rssi_offset = rssi_offset;
    rfconf.tx_enable = false;
    rfconf.single_input_mode = single_input_mode;
    if (lgw_rxrf_setconf(0, &rfconf) != LGW_HAL_SUCCESS) {
        printf("ERROR: failed to configure rxrf 0\n");
        return EXIT_FAILURE;
    }

    memset( &rfconf, 0, sizeof rfconf);
    rfconf.enable = true;
    rfconf.freq_hz = fb;
    rfconf.type = radio_type;
    rfconf.rssi_offset = rssi_offset;
    rfconf.tx_enable = false;
    rfconf.single_input_mode = single_input_mode;
    if (lgw_rxrf_setconf(1, &rfconf) != LGW_HAL_SUCCESS) {
        printf("ERROR: failed to configure rxrf 1\n");
        return EXIT_FAILURE;
    }

    /* set configuration for LoRa multi-SF channels (bandwidth cannot be set) */
    memset(&ifconf, 0, sizeof(ifconf));
    for (i = 0; i < 8; i++) {
        ifconf.enable = true;
        if (channel_mode == 0) {
            ifconf.rf_chain = channel_rfchain_mode0[i];
            ifconf.freq_hz = channel_if_mode0[i];
        } else if (channel_mode == 1) {
            ifconf.rf_chain = channel_rfchain_mode1[i];
            ifconf.freq_hz = channel_if_mode1[i];
        } else {
            printf("ERROR: channel mode not supported\n");
            return EXIT_FAILURE;
        }
        ifconf.datarate = DR_LORA_SF7;
        if (lgw_rxif_setconf(i, &ifconf) != LGW_HAL_SUCCESS) {
            printf("ERROR: failed to configure rxif %d\n", i);
            return EXIT_FAILURE;
        }
    }

    /* set configuration for LoRa Service channel */
    memset(&ifconf, 0, sizeof(ifconf));
    ifconf.rf_chain = channel_rfchain_mode0[i];
    ifconf.freq_hz = channel_if_mode0[i];
    ifconf.datarate = DR_LORA_SF7;
    ifconf.bandwidth = BW_250KHZ;
    if (lgw_rxif_setconf(8, &ifconf) != LGW_HAL_SUCCESS) {
        printf("ERROR: failed to configure rxif for LoRa service channel\n");
        return EXIT_FAILURE;
    }

    /* set the buffer size to hold received packets */
    struct lgw_pkt_rx_s rxpkt[max_rx_pkt];
    printf("INFO: rxpkt buffer size is set to %u\n", max_rx_pkt);
    printf("INFO: Select channel mode %u\n", channel_mode);

    /* Loop until user quits */
    cnt_loop = 0;
    while(cnt_loop < 10)
    {
        cnt_loop += 1;

        /* Board reset */
        lgw_reset();

        /* connect, configure and start the LoRa concentrator */
        x = lgw_start();
        if (x != 0) {
            printf("ERROR: failed to start the gateway\n");
            return EXIT_FAILURE;
        }

        /* Loop until we have enough packets with CRC OK */
        printf("Waiting for packets...\n");
        nb_pkt_crc_ok = 0;
        while (((nb_pkt_crc_ok < nb_loop) || nb_loop == 0)) {
            /* fetch N packets */
            nb_pkt = lgw_receive(ARRAY_SIZE(rxpkt), rxpkt);

            if (nb_pkt == 0) {
                wait_ms(10);
            } else {
                for (i = 0; i < nb_pkt; i++) {
                    if (rxpkt[i].status == STAT_CRC_OK) {
                        nb_pkt_crc_ok += 1;
                    }
                    printf("\n----- %s packet -----\n", (rxpkt[i].modulation == MOD_LORA) ? "LoRa" : "FSK");
                    printf("  count_us: %u\n", rxpkt[i].count_us);
                    printf("  size:     %u\n", rxpkt[i].size);
                    printf("  chan:     %u\n", rxpkt[i].if_chain);
                    printf("  status:   0x%02X\n", rxpkt[i].status);
                    printf("  datr:     %u\n", rxpkt[i].datarate);
                    printf("  codr:     %u\n", rxpkt[i].coderate);
                    printf("  rf_chain  %u\n", rxpkt[i].rf_chain);
                    printf("  freq_hz   %u\n", rxpkt[i].freq_hz);
                    printf("  snr_avg:  %.1f\n", rxpkt[i].snr);
                    printf("  rssi_chan:%.1f\n", rxpkt[i].rssic);
                    printf("  rssi_sig :%.1f\n", rxpkt[i].rssis);
                    printf("  crc:      0x%04X\n", rxpkt[i].crc);
                    for (j = 0; j < rxpkt[i].size; j++) {
                        printf("%02X ", rxpkt[i].payload[j]);
                    }
                    printf("\n");
                }
                printf("Received %d packets (total:%lu)\n", nb_pkt, nb_pkt_crc_ok);
            }
        }

        printf( "\nNb valid packets received: %lu CRC OK (%lu)\n", nb_pkt_crc_ok, cnt_loop );

        /* Stop the gateway */
        x = lgw_stop();
        if (x != 0) {
            printf("ERROR: failed to stop the gateway\n");
            return EXIT_FAILURE;
        }
    }

    /* Board reset */
    lgw_reset();

    printf("=========== Test End ===========\n");

    return 0;
}

static struct {
    struct arg_lit *help;
    struct arg_lit *single_input;
    struct arg_int *clock_source;
    struct arg_int *radio_type;
    struct arg_dbl *radio_a_freq;
    struct arg_dbl *radio_b_freq;
    struct arg_dbl *rssi_offset;
    struct arg_int *num_packet;
    struct arg_int *rx_array_size;
    struct arg_int *freq_plan_mode;
    struct arg_end *end;
} hal_conf_args;

static int do_hal_config_cmd(int argc, char **argv)
{
    uint32_t val;
    double fval;
    int nerrors;

    nerrors = arg_parse(argc, argv, (void **)&hal_conf_args);

    // process '-h' or '--help' first, before the error reporting
    if (hal_conf_args.help->count) {
        usage();
        return 0;
    }

    if (nerrors != 0) {
        arg_print_errors(stderr, hal_conf_args.end, argv[0]);
        return 0;
    }

    // process '-r' for radio type
    if (hal_conf_args.radio_type->count > 0) {
        val = (uint32_t)hal_conf_args.radio_type->ival[0];
        if(val == 1255)
            radio_type = LGW_RADIO_TYPE_SX1255;
        else if(val == 1257)
            radio_type = LGW_RADIO_TYPE_SX1257;
        else if(val == 1250)
            radio_type = LGW_RADIO_TYPE_SX1250;
        else {
            printf("'-r' with wrong value: %d; should be 1255/1257/1250\n", val);
            return -1;
        }
    }

    // process '-k' for clock source
    if (hal_conf_args.clock_source->count > 0) {
        val = (uint32_t)hal_conf_args.clock_source->ival[0];
        if(val > 1){
            printf("'-k' with wrong value: %d; should be 0 or 1\n", val);
            return -1;
        }
        clocksource = val;
    }

    // process '-j' for single input mode
    if (hal_conf_args.single_input->count > 0) {
        single_input_mode = true;
    }

    // process '-a' for single input mode
    if (hal_conf_args.radio_a_freq->count > 0) {
        fval = hal_conf_args.radio_a_freq->dval[0];
        fa = (uint32_t)((fval * 1e6) + 0.5); /* 0.5 Hz offset to get rounding */
    }
    // process '-b' for single input mode
    if (hal_conf_args.radio_b_freq->count > 0) {
        fval = hal_conf_args.radio_b_freq->dval[0];
        fb = (uint32_t)((fval * 1e6) + 0.5); /* 0.5 Hz offset to get rounding */
    }

    // process '-n' for number of packets to be received before exiting
    if (hal_conf_args.num_packet->count > 0) {
        val = (uint32_t)hal_conf_args.num_packet->ival[0];
        nb_loop = val;
    }

    // process '-z' for size of the RX packet array to be passed to lgw_receive()
    if (hal_conf_args.rx_array_size->count > 0) {
        val = (uint32_t)hal_conf_args.rx_array_size->ival[0];
        max_rx_pkt = val;
    }

    // process '-m' for Channel frequency plan mode
    if (hal_conf_args.freq_plan_mode->count > 0) {
        val = (uint32_t)hal_conf_args.freq_plan_mode->ival[0];
        if(val > 1){
            printf("'-m' with wrong value: %d; should be 0 or 1\n", val);
            return -1;
        }
        channel_mode = val;
    }

    // process '-o' for RSSI offset in dB
    if (hal_conf_args.rssi_offset->count > 0) {
        fval = hal_conf_args.rssi_offset->dval[0];
        rssi_offset = (float)fval;
    }

    test_hal_rx_main();

    return 0;
}


static void register_config(void)
{
    hal_conf_args.help           = arg_lit0("h", "help",          "print help");
    hal_conf_args.clock_source   = arg_int0("k", NULL, "<0|1>",   "Concentrator clock source (Radio A or Radio B) [0..1]");
    hal_conf_args.radio_type     = arg_int0("r", NULL, "<1250|1255|1257>",  "Radio type (1255, 1257, 1250)");
    hal_conf_args.radio_a_freq   = arg_dbl0("a", NULL, "<float>", "Radio A RX frequency in MHz");
    hal_conf_args.radio_b_freq   = arg_dbl0("b", NULL, "<float>", "Radio B RX frequency in MHz");
    hal_conf_args.rssi_offset    = arg_dbl0("o", NULL, "<float>", "RSSI Offset to be applied in dB");
    hal_conf_args.num_packet     = arg_int0("n", NULL, "<uint>",  "Number of packet received with CRC OK for each HAL start/stop loop");
    hal_conf_args.rx_array_size  = arg_int0("z", NULL, "<uint>",  "Size of the RX packet array to be passed to lgw_receive()");
    hal_conf_args.freq_plan_mode = arg_int0("m", NULL, "<0|1>",   "Channel frequency plan mode [0:LoRaWAN-like, 1:Same frequency for all channels (-400000Hz on RF0)]");
    hal_conf_args.single_input   = arg_lit0("j", NULL,            "Set radio in single input mode (SX1250 only)");
    hal_conf_args.end = arg_end(2);

    const esp_console_cmd_t hal_conf_cmd = {
        .command = "test_loragw_hal_rx",
        .help = "Config HAL for RX",
        .hint = NULL,
        .func = &do_hal_config_cmd,
        .argtable = &hal_conf_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&hal_conf_cmd));
}

void app_main(void)
{

    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    repl_config.task_stack_size = 4096 * 16;
    repl_config.prompt = "sx1302_hal>";

    usage();
    register_config();

    // initialize console REPL environment
    ESP_ERROR_CHECK(esp_console_repl_init(&repl_config));
    // start console REPL
    ESP_ERROR_CHECK(esp_console_repl_start());
}
