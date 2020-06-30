/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2019 Semtech

Description:
    Test program for network connection: as Wifi station and send UDP packets

License: Revised BSD License, see LICENSE.TXT file include in the project
*/

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include <lwip/netdb.h>
//#include <sys/param.h>


#define EXAMPLE_ESP_MAXIMUM_RETRY  5

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1


uint8_t wifi_ssid[32];
uint8_t wifi_pswd[64];
uint8_t udp_host[32];
char udp_msg[64] = "Message from SX1302 ESP32 PKT-FWD";
uint32_t udp_port;

static const char *TAG = "wifi station";
static int s_retry_num = 0;

static void event_handler(void *arg, esp_event_base_t event_base,
                                int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {},
    };


    strncpy((char *)wifi_config.sta.ssid, (char *)wifi_ssid, 32);
    strncpy((char *)wifi_config.sta.password, (char *)wifi_pswd, 64);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 wifi_ssid, wifi_pswd);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 wifi_ssid, wifi_pswd);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_got_ip));
    vEventGroupDelete(s_wifi_event_group);
}


static void udp_client_task(void *pvParameters)
{
    char rx_buffer[128];
    int addr_family = 0;
    int ip_protocol = 0;
    struct sockaddr_in dest_addr;
    struct sockaddr_in source_addr;
    int sock;
    int len, err;

    while (1) {
        dest_addr.sin_addr.s_addr = inet_addr((char *)udp_host);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(udp_port);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;

        sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created, sending to %s:%d", udp_host, udp_port);

        while (1) {
            err = sendto(sock, udp_msg, strlen(udp_msg), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                break;
            }
            ESP_LOGI(TAG, "Message sent");

            socklen_t socklen = sizeof(source_addr);
            len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            } else {
                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, udp_host);
                ESP_LOGI(TAG, "%s", rx_buffer);
                if (strncmp(rx_buffer, "OK: ", 4) == 0) {
                    ESP_LOGI(TAG, "Received expected message, reconnecting");
                    break;
                }
            }

            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }

        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void test_network_connection(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();

    // TODO: deal with Wifi broken
    xTaskCreate(udp_client_task, "udp_client", 4096, NULL, 5, NULL);
}


static struct {
    struct arg_lit *help;
    struct arg_str *wifi_ssid;
    struct arg_str *wifi_pswd;
    struct arg_str *udp_host;
    struct arg_int *udp_port;
    struct arg_str *udp_msg;
    struct arg_end *end;
} net_conf_args;

void usage(void) {
    printf("\n\n ---- test_network_connection ----\n");
    printf("\nAvailable options:\n");
    printf(" -h                   print this help\n");
    printf(" -u <wifi ssid>       Wifi SSID\n");
    printf(" -p <wifi password>   Wifi Password\n");
    printf(" --host <UDP Host>    UDP Host\n");
    printf(" --port <UDP Port>    UDP Port\n");
    printf(" -m <string>          UDP message for test\n");
}

// Note: No Error Checking!
// TODO: Should release the resources with arg_freetable().
// Please provide right arguments, or take whatever consequences.
static int do_net_config_cmd(int argc, char **argv)
{
    uint32_t val;
    const char *sval;
    int nerrors;

    nerrors = arg_parse(argc, argv, (void **)&net_conf_args);

    // process '-h' or '--help' first, before the error reporting
    if (net_conf_args.help->count) {
        usage();
        return 0;
    }

    if (nerrors != 0) {
        arg_print_errors(stderr, net_conf_args.end, argv[0]);
        return 0;
    }

    // process '-u' for modulation type
    if (net_conf_args.wifi_ssid->count > 0) {
        sval = net_conf_args.wifi_ssid->sval[0];
        sprintf((char *)wifi_ssid, "%s", (char *)sval);
    }

    // process '-p' for modulation type
    if (net_conf_args.wifi_pswd->count > 0) {
        sval = net_conf_args.wifi_pswd->sval[0];
        sprintf((char *)wifi_pswd, "%s", (char *)sval);
    }

    // process '--host' for modulation type
    if (net_conf_args.udp_host->count > 0) {
        sval = net_conf_args.udp_host->sval[0];
        sprintf((char *)udp_host, "%s", (char *)sval);
    }

    // process '--port' for number of packets to be received before exiting
    if (net_conf_args.udp_port->count > 0) {
        val = (uint32_t)net_conf_args.udp_port->ival[0];
        udp_port = val;
    }

    // process '-m' for modulation type
    if (net_conf_args.udp_msg->count > 0) {
        sval = net_conf_args.udp_msg->sval[0];
        sprintf(udp_msg, "%s", (char *)sval);
    }

    test_network_connection();

    return 0;
}


static void register_config(void)
{
    net_conf_args.help       = arg_lit0("h", "help",          "print help");
    net_conf_args.wifi_ssid  = arg_str0("u", NULL, "<wifi ssid>", "Wifi SSID");
    net_conf_args.wifi_pswd  = arg_str0("p", NULL, "<wifi password>", "Wifi Password");
    net_conf_args.udp_host   = arg_str0(NULL, "host", "<UDP Host>", "UDP Host");
    net_conf_args.udp_port   = arg_int0(NULL, "port", "<UDP Port>",  "UDP Port");
    net_conf_args.udp_msg    = arg_str0("m", NULL, "<message>", "UDP message for test");
    net_conf_args.end = arg_end(2);

    const esp_console_cmd_t hal_conf_cmd = {
        .command = "test_network_connection",
        .help = "Test Wifi/UDP network connection",
        .hint = NULL,
        .func = &do_net_config_cmd,
        .argtable = &net_conf_args
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
