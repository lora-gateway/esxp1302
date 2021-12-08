#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_tls_crypto.h"
#include <esp_http_server.h>

#include "http_server.h"


static const char *TAG = "esp32 web server";

typedef struct {
    char    *username;
    char    *password;
} basic_auth_info_t;
basic_auth_info_t web_auth_info;

char web_username[32] = BASIC_AUTH_USERNAME;
char web_password[32] = BASIC_AUTH_PASSWORD;


static char *http_auth_basic(const char *username, const char *password)
{
    int out;
    char *user_info = NULL;
    char *digest = NULL;
    size_t n = 0;

    asprintf(&user_info, "%s:%s", username, password);
    if (!user_info) {
        ESP_LOGE(TAG, "No enough memory for user information");
        return NULL;
    }
    esp_crypto_base64_encode(NULL, 0, &n, (const unsigned char *)user_info, strlen(user_info));

    /* 6: The length of the "Basic " string
     * n: Number of bytes for a base64 encode format
     * 1: Number of bytes for a reserved which be used to fill zero
    */
    digest = calloc(1, 6 + n + 1);
    if (digest) {
        strcpy(digest, "Basic ");
        esp_crypto_base64_encode((unsigned char *)digest + 6, n, (size_t *)&out, (const unsigned char *)user_info, strlen(user_info));
    }

    free(user_info);
    return digest;
}

static esp_err_t check_basic_auth(httpd_req_t *req)
{
    char *buf = NULL;
    size_t buf_len = 0;
    basic_auth_info_t *basic_auth_info = &web_auth_info;

    buf_len = httpd_req_get_hdr_value_len(req, "Authorization") + 1;
    if (buf_len == 1) {
        ESP_LOGE(TAG, "No auth header received");
        return ESP_FAIL;
    }

    buf = calloc(1, buf_len);
    if (!buf) {
        ESP_LOGE(TAG, "No enough memory for basic authorization");
        return ESP_ERR_NO_MEM;
    }

    if (httpd_req_get_hdr_value_str(req, "Authorization", buf, buf_len) == ESP_OK) {
        ESP_LOGI(TAG, "Found header => Authorization: %s", buf);
    } else {
        ESP_LOGE(TAG, "No auth value received");
        free(buf);
        return ESP_FAIL;
    }

    char *auth_credentials = http_auth_basic(basic_auth_info->username,
            basic_auth_info->password);
    if (!auth_credentials) {
        ESP_LOGE(TAG, "No enough memory for basic authorization credentials");
        free(buf);
        return ESP_ERR_NO_MEM;
    }

    if (strncmp(auth_credentials, buf, buf_len)) {
        ESP_LOGE(TAG, "Authenticated failed");
        free(auth_credentials);
        free(buf);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Authenticated!");
    free(auth_credentials);
    free(buf);
    return ESP_OK;
}

static esp_err_t gw_config_handler(httpd_req_t *req)
{
    char*  buf;
    size_t buf_len;
    esp_err_t err;

    err = check_basic_auth(req);
    if(err == ESP_FAIL){
        ESP_LOGE(TAG, "No auth header received");
        httpd_resp_set_status(req, HTTPD_401);
        httpd_resp_set_type(req, "application/json");
        httpd_resp_set_hdr(req, "Connection", "keep-alive");
        httpd_resp_set_hdr(req, "WWW-Authenticate", "Basic realm=\"\"");
        httpd_resp_send(req, NULL, 0);
        return ESP_OK;
    }

    /* Get header value string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Host: %s", buf);
        }
        free(buf);
    }

    /* Send response with custom headers and body */
    const char* resp_str = (const char *) "Hello ESXP1302 Gateway";
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

static const httpd_uri_t gw_config = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = gw_config_handler,
    .user_ctx  = "Hello ESXP1302 Gateway!"
};

static httpd_handle_t start_web_server(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    ESP_LOGI(TAG, "Starting web server on port: '%d'", config.server_port);

    if (httpd_start(&server, &config) == ESP_OK) {
        ESP_LOGI(TAG, "Registering URI handlers");

        web_auth_info.username = web_username;
        web_auth_info.password = web_password;

        httpd_register_uri_handler(server, &gw_config);
        return server;
    }

    ESP_LOGI(TAG, "Error starting web server!");
    return NULL;
}

static void stop_web_server(httpd_handle_t server)
{
    httpd_stop(server);
}

static void connect_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;

    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting esp32 internal web server");
        *server = start_web_server();
    }
}

static void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;

    if (*server) {
        ESP_LOGI(TAG, "Stopping esp32 internal web server");
        stop_web_server(*server);
        *server = NULL;
    }
}

void http_server_task(void *pvParameters)
{
    static httpd_handle_t server = NULL;

    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
    //ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ETHERNET_EVENT_DISCONNECTED, &disconnect_handler, &server));

    /* Start the server for the first time */
    server = start_web_server();

    vTaskDelete(NULL);

    while(true){
        printf("http server ended\n");
        vTaskDelay(8000 / portTICK_PERIOD_MS);
    }
}
