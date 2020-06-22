/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2019 Semtech

Description:
    Minimum test program for HAL TX capability

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
#include <math.h>

#include "esp_console.h"
#include "argtable3/argtable3.h"

#include "loragw_hal.h"
#include "loragw_reg.h"
#include "loragw_gpio.h"
#include "loragw_aux.h"


#define RAND_RANGE(min, max) (rand() % (max + 1 - min) + min)

#define DEFAULT_CLK_SRC     0
#define DEFAULT_FREQ_HZ     868500000U


/* describe command line options */
void usage(void) {
    //printf("Library version information: %s\n", lgw_version_info());
    printf("Available options:\n");
    printf(" -h print this help\n");
    printf(" -k <uint>  Concentrator clock source (Radio A or Radio B) [0..1]\n");
    printf(" -c <uint>  RF chain to be used for TX (Radio A or Radio B) [0..1]\n");
    printf(" -r <uint>  Radio type (1255, 1257, 1250)\n");
    printf(" -f <float> Radio TX frequency in MHz\n");
    printf(" -m <str>   modulation type ['CW', 'LORA', 'FSK']\n");
    printf(" -o <int>   CW frequency offset from Radio TX frequency in kHz [-65..65]\n");
    printf(" -s <uint>  LoRa datarate 0:random, [5..12]\n");
    printf(" -b <uint>  LoRa bandwidth in khz 0:random, [125, 250, 500]\n");
    printf(" -l <uint>  FSK/LoRa preamble length, [6..65535]\n");
    printf(" -d <uint>  FSK frequency deviation in kHz [1:250]\n");
    printf(" -q <float> FSK bitrate in kbps [0.5:250]\n");
    printf(" -n <uint>  Number of packets to be sent\n");
    printf(" -z <uint>  size of packets to be sent 0:random, [9..255]\n");
    printf(" -t <uint>  TX mode timestamped with delay in ms. If delay is 0, TX mode GPS trigger\n");
    printf(" -p <int>   RF power in dBm\n");
    printf(" -i         Send LoRa packet using inverted modulation polarity\n");
    printf(" -j         Set radio in single input mode (SX1250 only)\n");
    printf( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n" );
    printf(" --pa   <uint> PA gain SX125x:[0..3], SX1250:[0,1]\n");
    printf(" --dig  <uint> sx1302 digital gain for sx125x [0..3]\n");
    printf(" --dac  <uint> sx125x DAC gain [0..3]\n");
    printf(" --mix  <uint> sx125x MIX gain [5..15]\n");
    printf(" --pwid <uint> sx1250 power index [0..22]\n");
    printf( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n" );
    printf(" --nhdr        Send LoRa packet with implicit header\n");
    printf( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n" );
    printf(" --loop        Number of loops for HAL start/stop (HAL unitary test)\n");
}

uint8_t size = 0;
int8_t freq_offset = 0;
uint32_t ft = DEFAULT_FREQ_HZ;
int8_t rf_power = 0;
uint8_t sf = 0;
uint16_t bw_khz = 0;
uint32_t nb_pkt = 1;
char mod[64] = "LORA";
float br_kbps = 50;
uint8_t clocksource = 0;
uint8_t rf_chain = 0;
lgw_radio_type_t radio_type = LGW_RADIO_TYPE_NONE;
uint16_t preamble = 8;
bool invert_pol = false;
bool no_header = false;
bool single_input_mode = false;
uint8_t fdev_khz = 25;
uint32_t trig_delay_us = 1000000;
bool trig_delay = false;
struct lgw_tx_gain_lut_s txlut; /* TX gain table */
unsigned int nb_loop = 1, cnt_loop;


int test_hal_tx_main(void)
{
    int i, x;
    struct lgw_conf_board_s boardconf;
    struct lgw_conf_rxrf_s rfconf;
    struct lgw_pkt_tx_s pkt;
    uint8_t tx_status;
    uint32_t count_us;


    /* Initialize TX gain LUT */
    txlut.size = 0;
    memset(txlut.lut, 0, sizeof txlut.lut);

    /* Summary of packet parameters */
    if (strcmp(mod, "CW") == 0) {
        printf("Sending %i CW on %u Hz (Freq. offset %d kHz) at %i dBm\n", nb_pkt, ft, freq_offset, rf_power);
    }
    else if (strcmp(mod, "FSK") == 0) {
        printf("Sending %i FSK packets on %u Hz (FDev %u kHz, Bitrate %.2f, %i bytes payload, %i symbols preamble) at %i dBm\n", nb_pkt, ft, fdev_khz, br_kbps, size, preamble, rf_power);
    } else {
        printf("Sending %i LoRa packets on %u Hz (BW %i kHz, SF %i, CR %i, %i bytes payload, %i symbols preamble, %s header, %s polarity) at %i dBm\n", nb_pkt, ft, bw_khz, sf, 1, size, preamble, (no_header == false) ? "explicit" : "implicit", (invert_pol == false) ? "non-inverted" : "inverted", rf_power);
    }

    /* Configure the gateway */
    memset( &boardconf, 0, sizeof boardconf);
    boardconf.lorawan_public = true;
    boardconf.clksrc = clocksource;
    boardconf.full_duplex = false;
    if (lgw_board_setconf(&boardconf) != LGW_HAL_SUCCESS) {
        printf("ERROR: failed to configure board\n");
        return EXIT_FAILURE;
    }

    memset( &rfconf, 0, sizeof rfconf);
    rfconf.enable = true; /* rf chain 0 needs to be enabled for calibration to work on sx1257 */
    rfconf.freq_hz = ft;
    rfconf.type = radio_type;
    rfconf.tx_enable = true;
    rfconf.single_input_mode = single_input_mode;
    if (lgw_rxrf_setconf(0, &rfconf) != LGW_HAL_SUCCESS) {
        printf("ERROR: failed to configure rxrf 0\n");
        return EXIT_FAILURE;
    }

    memset( &rfconf, 0, sizeof rfconf);
    rfconf.enable = (((rf_chain == 1) || (clocksource == 1)) ? true : false);
    rfconf.freq_hz = ft;
    rfconf.type = radio_type;
    rfconf.tx_enable = false;
    rfconf.single_input_mode = single_input_mode;
    if (lgw_rxrf_setconf(1, &rfconf) != LGW_HAL_SUCCESS) {
        printf("ERROR: failed to configure rxrf 1\n");
        return EXIT_FAILURE;
    }

    if (txlut.size > 0) {
        if (lgw_txgain_setconf(rf_chain, &txlut) != LGW_HAL_SUCCESS) {
            printf("ERROR: failed to configure txgain lut\n");
            return EXIT_FAILURE;
        }
    }

    for (cnt_loop = 0; cnt_loop < nb_loop; cnt_loop++) {
        /* Board reset */
        lgw_reset();

        /* connect, configure and start the LoRa concentrator */
        x = lgw_start();
        if (x != 0) {
            printf("ERROR: failed to start the gateway\n");
            return EXIT_FAILURE;
        }

        /* Send packets */
        memset(&pkt, 0, sizeof pkt);
        pkt.rf_chain = rf_chain;
        pkt.freq_hz = ft;
        pkt.rf_power = rf_power;
        if (trig_delay == false) {
            pkt.tx_mode = IMMEDIATE;
        } else {
            if (trig_delay_us == 0) {
                pkt.tx_mode = ON_GPS;
            } else {
                pkt.tx_mode = TIMESTAMPED;
            }
        }
        if ( strcmp( mod, "CW" ) == 0 ) {
            pkt.modulation = MOD_CW;
            pkt.freq_offset = freq_offset;
            pkt.f_dev = fdev_khz;
        }
        else if( strcmp( mod, "FSK" ) == 0 ) {
            pkt.modulation = MOD_FSK;
            pkt.no_crc = false;
            pkt.datarate = br_kbps * 1e3;
            pkt.f_dev = fdev_khz;
        } else {
            pkt.modulation = MOD_LORA;
            pkt.coderate = CR_LORA_4_5;
            pkt.no_crc = true;
        }
        pkt.invert_pol = invert_pol;
        pkt.preamble = preamble;
        pkt.no_header = no_header;
        pkt.payload[0] = 0x40; /* Confirmed Data Up */
        pkt.payload[1] = 0xAB;
        pkt.payload[2] = 0xAB;
        pkt.payload[3] = 0xAB;
        pkt.payload[4] = 0xAB;
        pkt.payload[5] = 0x00; /* FCTrl */
        pkt.payload[6] = 0; /* FCnt */
        pkt.payload[7] = 0; /* FCnt */
        pkt.payload[8] = 0x02; /* FPort */
        for (i = 9; i < 255; i++) {
            pkt.payload[i] = i;
        }

        for (i = 0; i < (int)nb_pkt; i++) {
            if (trig_delay == true) {
                if (trig_delay_us > 0) {
                    lgw_get_instcnt(&count_us);
                    printf("count_us:%u\n", count_us);
                    pkt.count_us = count_us + trig_delay_us;
                    printf("programming TX for %u\n", pkt.count_us);
                } else {
                    printf("programming TX for next PPS (GPS)\n");
                }
            }

            if( strcmp( mod, "LORA" ) == 0 ) {
                pkt.datarate = (sf == 0) ? (uint8_t)RAND_RANGE(5, 12) : sf;
            }

            switch (bw_khz) {
                case 125:
                    pkt.bandwidth = BW_125KHZ;
                    break;
                case 250:
                    pkt.bandwidth = BW_250KHZ;
                    break;
                case 500:
                    pkt.bandwidth = BW_500KHZ;
                    break;
                default:
                    pkt.bandwidth = (uint8_t)RAND_RANGE(BW_125KHZ, BW_500KHZ);
                    break;
            }

            pkt.size = (size == 0) ? (uint8_t)RAND_RANGE(9, 255) : size;

            pkt.payload[6] = (uint8_t)(i >> 0); /* FCnt */
            pkt.payload[7] = (uint8_t)(i >> 8); /* FCnt */
            x = lgw_send(&pkt);
            if (x != 0) {
                printf("ERROR: failed to send packet\n");
                return EXIT_FAILURE;
            }
            /* wait for packet to finish sending */
            do {
                wait_ms(5);
                lgw_status(pkt.rf_chain, TX_STATUS, &tx_status); /* get TX status */
            } while (tx_status != TX_FREE);

            printf("TX done\n");
        }

        printf( "\nNb packets sent: %u (%u)\n", i, cnt_loop + 1 );

        /* Stop the gateway */
        x = lgw_stop();
        if (x != 0) {
            printf("ERROR: failed to stop the gateway\n");
            return EXIT_FAILURE;
        }

        /* Board reset */
        lgw_reset();
    }

    printf("=========== Test End ===========\n");

    return 0;
}

static struct {
    struct arg_lit *help;
    struct arg_int *clock_source;
    struct arg_int *rf_chain;
    struct arg_int *radio_type;
    struct arg_dbl *radio_tx_freq;
    struct arg_str *modu_type;
    struct arg_int *cw_freq_offset;
    struct arg_int *lora_datarate;
    struct arg_int *lora_bandwidth;
    struct arg_int *preamb_length;
    struct arg_int *freq_deviation;
    struct arg_dbl *lora_bitrate;
    struct arg_int *n_packets;
    struct arg_int *packet_size;
    struct arg_int *tx_tmst_delay;
    struct arg_int *lora_rf_power;
    struct arg_lit *invt_polarity;
    struct arg_lit *single_input;
    struct arg_int *pa_gain;
    struct arg_int *dig_gain;
    struct arg_int *dac_gain;
    struct arg_int *mix_gain;
    struct arg_int *pow_index;
    struct arg_lit *implicit_hd;
    struct arg_int *loop_time;
    struct arg_end *end;
} hal_conf_args;

static int do_hal_config_cmd(int argc, char **argv)
{
    uint32_t val;
    int32_t ival;
    double fval;
    const char *sval;
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

    // process '-i' for inverted modulation polarity
    if (hal_conf_args.invt_polarity->count > 0) {
        invert_pol = true;
    }

    // process '-j' for single input mode
    if (hal_conf_args.single_input->count > 0) {
        single_input_mode = true;
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

    // process '-l' for preamble length
    if (hal_conf_args.preamb_length->count > 0) {
        val = (uint32_t)hal_conf_args.preamb_length->ival[0];
        if(val > 65535){
            printf("'-c' with wrong value: %d; Use -h to print help\n", val);
            return -1;
        }
        preamble = (uint16_t)val;
    }

    // process '-m' for modulation type
    if (hal_conf_args.modu_type->count > 0) {
        sval = hal_conf_args.modu_type->sval[0];
        if((strcmp(sval, "CW") != 0) && (strcmp(sval, "LORA") != 0) && (strcmp(sval, "FSK"))){
            printf("'-m' with wrong value: %s; Use -h to print help\n", sval);
            return -1;
        }
        sprintf(mod, "%s", sval);
    }

    // process '-o' for CW frequency offset from Radio TX frequency
    if (hal_conf_args.cw_freq_offset->count > 0) {
        ival = (int32_t)hal_conf_args.cw_freq_offset->ival[0];
        if((ival < -65) || (ival > 65)){
            printf("'-o' with wrong value: %d; Use -h to print help\n", ival);
            return -1;
        }
        freq_offset = ival;
    }

    // process '-d' for FSK frequency deviation
    if (hal_conf_args.freq_deviation->count > 0) {
        ival = (int32_t)hal_conf_args.freq_deviation->ival[0];
        if((ival < 1) || (ival > 250)){
            printf("'-d' with wrong value: %d; Use -h to print help\n", ival);
            return -1;
        }
        fdev_khz = (uint8_t)ival;
    }

    // process '-q' for FSK bitrate
    if (hal_conf_args.lora_bitrate->count > 0) {
        fval = hal_conf_args.lora_bitrate->dval[0];
        if((fval < 0.5) || (fval > 250)){
            printf("'-q' with wrong value: %f; Use -h to print help\n", fval);
            return -1;
        }
        br_kbps = (float)fval;
    }

    // process '-t' for single input mode
    if (hal_conf_args.tx_tmst_delay->count > 0) {
        val = hal_conf_args.tx_tmst_delay->ival[0];
        trig_delay = true;
        trig_delay_us = (uint32_t)(val * 1E3);
    }

    // process '-k' for clock source
    if (hal_conf_args.clock_source->count > 0) {
        val = (uint32_t)hal_conf_args.clock_source->ival[0];
        if(val > 1){
            printf("'-k' with wrong value: %d; should be 0 or 1\n", val);
            return -1;
        }
        clocksource = (uint8_t)val;
    }

    // process '-c' for RF chain
    if (hal_conf_args.rf_chain->count > 0) {
        val = (uint32_t)hal_conf_args.rf_chain->ival[0];
        if(val > 1){
            printf("'-c' with wrong value: %d; should be 0 or 1\n", val);
            return -1;
        }
        rf_chain = (uint8_t)val;
    }

    // process '-f' for Radio TX frequency in MHz
    if (hal_conf_args.radio_tx_freq->count > 0) {
        fval = hal_conf_args.radio_tx_freq->dval[0];
        ft = (uint32_t)((fval*1e6) + 0.5); /* .5 Hz offset to get rounding */
    }

    // process '-s' for LoRa datarate
    if (hal_conf_args.lora_datarate->count > 0) {
        val = (uint32_t)hal_conf_args.lora_datarate->ival[0];
        if((val < 5) || (val > 12)){
            printf("'-s' with wrong value: %d; Use -h to print help\n", val);
            return -1;
        }
        sf = (uint8_t)val;
    }

    // process '-b' for LoRa bandwidth in khz
    if (hal_conf_args.lora_bandwidth->count > 0) {
        val = (uint32_t)hal_conf_args.lora_bandwidth->ival[0];
        if((val != 125) && (val != 250) && (val != 500)){
            printf("'-b' with wrong value: %d; Use -h to print help\n", val);
            return -1;
        }
        bw_khz = (uint16_t)val;
    }

    // process '-n' for number of packets to be sent
    if (hal_conf_args.n_packets->count > 0) {
        val = (uint32_t)hal_conf_args.n_packets->ival[0];
        nb_pkt = val;
    }

    // process '-p' for RF power
    if (hal_conf_args.lora_rf_power->count > 0) {
        val = (uint32_t)hal_conf_args.lora_rf_power->ival[0];
        rf_power = (int8_t)val;
        txlut.size = 1;
        txlut.lut[0].rf_power = rf_power;
    }

    // process '-z' for packet size
    if (hal_conf_args.packet_size->count > 0) {
        val = (uint32_t)hal_conf_args.packet_size->ival[0];
        if((val < 9) || (val > 255)){
            printf("'-z' with wrong value: %d; Use -h to print help\n", val);
            return -1;
        }
        size = (uint8_t)val;
    }

    // process '--pa' for PA gain
    if (hal_conf_args.pa_gain->count > 0) {
        val = (uint32_t)hal_conf_args.pa_gain->ival[0];
        if(val > 3){
            printf("'--pa' with wrong value: %d; Use -h to print help\n", val);
            return -1;
        }
        txlut.size = 1;
        txlut.lut[0].pa_gain = (uint8_t)val;
    }

    // process '--dac' for sx125x DAC gain
    if (hal_conf_args.dac_gain->count > 0) {
        val = (uint32_t)hal_conf_args.dac_gain->ival[0];
        if(val > 3){
            printf("'--dac' with wrong value: %d; Use -h to print help\n", val);
            return -1;
        }
        txlut.size = 1;
        txlut.lut[0].dac_gain = (uint8_t)val;
    }

    // process '--mix' for sx125x MIX gain
    if (hal_conf_args.mix_gain->count > 0) {
        val = (uint32_t)hal_conf_args.mix_gain->ival[0];
        if(val > 15){
            printf("'--mix' with wrong value: %d; Use -h to print help\n", val);
            return -1;
        }
        txlut.size = 1;
        txlut.lut[0].mix_gain = (uint8_t)val;
    }

    // process '--dig' for sx1302 digital gain
    if (hal_conf_args.dig_gain->count > 0) {
        val = (uint32_t)hal_conf_args.dig_gain->ival[0];
        if(val > 3){
            printf("'--mix' with wrong value: %d; Use -h to print help\n", val);
            return -1;
        }
        txlut.size = 1;
        txlut.lut[0].dig_gain = (uint8_t)val;
    }

    // process '--pwid' for sx1250 power index
    if (hal_conf_args.pow_index->count > 0) {
        val = (uint32_t)hal_conf_args.pow_index->ival[0];
        if(val > 22){
            printf("'--pwid' with wrong value: %d; Use -h to print help\n", val);
            return -1;
        }
        txlut.size = 1;
        txlut.lut[0].mix_gain = 5; /* TODO: rework this, should not be needed for sx1250 */
        txlut.lut[0].pwr_idx = (uint8_t)val;
    }

    // process '--loop' for Number of loops for HAL start/stop
    if (hal_conf_args.loop_time->count > 0) {
        val = (uint32_t)hal_conf_args.loop_time->ival[0];
        nb_loop = val;
    }

    // process '--nhdr' for Send LoRa packet with implicit header
    if (hal_conf_args.implicit_hd->count > 0) {
        no_header = true;
    }

    test_hal_tx_main();

    return 0;
}


static void register_config(void)
{
    hal_conf_args.help           = arg_lit0("h", "help",          "print help");
    hal_conf_args.clock_source   = arg_int0("k", NULL, "<0|1>",   "Concentrator clock source (Radio A or B) [0..1]");
    hal_conf_args.rf_chain       = arg_int0("c", NULL, "<0|1>",   "RF chain to be used for TX (Radio A or B) [0..1]");
    hal_conf_args.radio_type     = arg_int0("r", NULL, "<1250|1255|1257>",  "Radio type (1255, 1257, 1250)");
    hal_conf_args.radio_tx_freq  = arg_dbl0("f", NULL, "<float>", "Radio TX frequency in MHz");
    hal_conf_args.modu_type      = arg_str0("m", NULL, "<CW|LORA|FSK>", "modulation type ['CW', 'LORA', 'FSK']");
    hal_conf_args.cw_freq_offset = arg_int0("o", NULL, "<[-65..65]>", "CW frequency offset from Radio TX frequency in kHz [-65..65]");
    hal_conf_args.lora_datarate  = arg_int0("s", NULL, "<0|[5..12]>", "LoRa datarate 0:random, [5..12]");

    hal_conf_args.lora_bandwidth = arg_int0("b", NULL, "<125|250|500>", "LoRa bandwidth in khz 0:random, [125, 250, 500]");
    hal_conf_args.preamb_length  = arg_int0("l", NULL, "<[6.65535]>", "FSK/LoRa preamble length, [6..65535]");
    hal_conf_args.freq_deviation = arg_int0("d", NULL, "<[1..250]>", "FSK frequency deviation in kHz [1:250]");
    hal_conf_args.lora_bitrate   = arg_dbl0("q", NULL, "<[0.5..250]>", "FSK bitrate in kbps [0.5:250]");
    hal_conf_args.n_packets      = arg_int0("n", NULL, "<uint>", "Number of packets to be sent");
    hal_conf_args.packet_size    = arg_int0("z", NULL, "<[9..255]>", "size of packets to be sent 0:random, [9..255]");
    hal_conf_args.tx_tmst_delay  = arg_int0("t", NULL, "<uint>", "TX mode timestamped with delay in ms. If delay is 0, TX mode GPS trigger");
    hal_conf_args.lora_rf_power  = arg_int0("p", NULL, "<int>", "RF power in dBm");
    hal_conf_args.invt_polarity  = arg_lit0("i", NULL, "Send LoRa packet using inverted modulation polarity");
    hal_conf_args.single_input   = arg_lit0("j", NULL, "Set radio in single input mode (SX1250 only)");


    hal_conf_args.pa_gain        = arg_int0(NULL, "pa", "<[0..3]|0|1>", "PA gain SX125x:[0..3], SX1250:[0,1]");
    hal_conf_args.dig_gain       = arg_int0(NULL, "dig", "<[0..3]>", "sx1302 digital gain for sx125x [0..3]");
    hal_conf_args.dac_gain       = arg_int0(NULL, "dac", "<[0..3]>", "sx125x DAC gain [0..3]");
    hal_conf_args.mix_gain       = arg_int0(NULL, "mix", "<[5..15]>", "sx125x MIX gain [5..15]");
    hal_conf_args.pow_index      = arg_int0(NULL, "pwid", "<[0..22]>", "sx1250 power index [0..22]");
    hal_conf_args.implicit_hd    = arg_lit0(NULL, "nhdr", "Send LoRa packet with implicit header");
    hal_conf_args.loop_time      = arg_int0(NULL, "loop", "<uint>", "Number of loops for HAL start/stop");
    hal_conf_args.end = arg_end(2);

    const esp_console_cmd_t hal_conf_cmd = {
        .command = "test_loragw_hal_tx",
        .help = "Config HAL for TX",
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
