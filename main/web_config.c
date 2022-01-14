#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "web_config.h"
#include "nvs_flash.h"
#include "nvs.h"


bool nvs_ready = false;
static const char *TAG = "esp32 web config";


config_s config[CONFIG_NUM] = {
    { WIFI_SSID, "wifi_ssid", NULL, 0 },
    { WIFI_PASSWORD, "wifi_pswd", NULL, 0 },
    { NS_HOST, "ns_host", NULL, 0 },
    { NS_PORT, "ns_port", NULL, 0 },
    { GW_ID, "gw_id", NULL, 0 },
    { WIFI_MODE, "wifi_mode", NULL, 0 }
};

tag_e name2tag(char *name)
{
    int len;

    for(int i = 0; i < CONFIG_NUM; i++) {
        len = strlen(config[i].name);
        if(strncmp(name, config[i].name, len) == 0){
            return config[i].tag;
        }
    }
    return CONFIG_ERR;
}

esp_err_t init_config_storage(void)
{
    if(nvs_ready)  // already ready; no need to init again
        return ESP_OK;

    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased; retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    if(err == ESP_OK)
        nvs_ready = true;

    return err;
}


int update_config(char *str, int len)
{
    int n, name_len;
    char *p = NULL;

    tag_e tag = name2tag(str);
    if(tag < CONFIG_NUM){
        name_len = strlen(config[tag].name);
        n = len - name_len; // the space for '=' is saved used for '\0'
        if(n == 1) // no value, so return directly without overwrite the old value
            return 0;
        p = malloc(n);
        if(!p)
            return -1;

        strncpy(p, str + name_len + 1, n - 1);
        if(config[tag].val)
            free(config[tag].val);
        p[n-1] = '\0';
        config[tag].val = p;
        config[tag].len = n - 1;
    }
    return 0;
}

void dump_config(void)
{
    for(int i = 0; i < CONFIG_NUM; i++) {
        if(config[i].val != NULL)
            printf("%s: %s @ %p\n", config[i].name, config[i].val, config[i].val);
        else
            printf("%s: (no value)\n", config[i].name);
    }
}

esp_err_t read_config(void)
{
    nvs_handle_t my_handle;
    size_t len;
    char *p = NULL;

    if(nvs_ready != true){
        ESP_LOGW(TAG, "NVS storage not available");
        return ESP_FAIL;
    }

    esp_err_t err = nvs_open("nvs", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "opening NVS handle failed: %s!", esp_err_to_name(err));
        ESP_LOGW(TAG, "Config not updated");
        return ESP_FAIL;
    }

    for(int i = 0; i < CONFIG_NUM; i++){
        // get length and malloc space
        err = nvs_get_str(my_handle, config[i].name, NULL, &len);
        if(err != ESP_OK){
            printf("Error (%s) reading %s!\n", esp_err_to_name(err), config[i].name);
            config[i].val = NULL;
            config[i].len = 0;
            continue;
        }

        p = malloc(len);
        if(!p) {
            printf("Warning: malloc() for %u bytes failed\n", len);
            continue;  // just ignore if failed
        }

        // get value and save to config list
        err = nvs_get_str(my_handle, config[i].name, p, &len);
        if(err != ESP_OK){
            printf("Error (%s) reading %s!\n", esp_err_to_name(err), config[i].name);
            continue;
        }
        p[len-1] = '\0';
        config[i].val = p;
        config[i].len = len - 1;
    }
    nvs_close(my_handle);
    return ESP_OK;
}

int save_config(void)
{
    if(nvs_ready != true){
        ESP_LOGW(TAG, "NVS storage not available");
        return ESP_FAIL;
    }

    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("nvs", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "opening NVS handle failed: %s!", esp_err_to_name(err));
        ESP_LOGW(TAG, "Config not updated");
        return ESP_FAIL;
    }

    // Update config
    for(int i = 0; i < CONFIG_NUM; i++) {
        if(config[i].val != NULL){
            err = nvs_set_str(my_handle, config[i].name, config[i].val);
            printf("set %s: %s\n", config[i].name, (err != ESP_OK) ? "Failed!" : "Done");
        }
    }

    // Commit config
    printf("Save config to NVS... ");
    err = nvs_commit(my_handle);
    printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
    nvs_close(my_handle);
    return ESP_OK;
}

void extract_data_items(char *str)
{
    char *start = str;

    if(str == NULL)
        return;

    while(*str != '\0'){
        if(*str == '&'){
            update_config(start, str-start);
            start = str+1;
        }
        str++;
    }
    update_config(start, str-start);
    printf("\n");
}

static int _config_wifi_mode(wifi_mode_e mode)
{
    int len = 7;
    char *p;

    p = malloc(len+1);
    if(!p) {
        printf("Warning: malloc() for %u bytes failed\n", len);
        return -1;
    }
    if(mode == WIFI_MODE_SOFT_AP)
        strncpy(p, "soft_ap", len+1);
    if(mode == WIFI_MODE_STATION)
        strncpy(p, "station", len+1);
    p[len] = '\0';
    printf("config wifi_mode = %s\n", p);

    if(config[WIFI_MODE].val)
        free(config[WIFI_MODE].val);
    config[WIFI_MODE].val = p;
    config[WIFI_MODE].len = len;

    save_config();
    return 0;
}

int config_wifi_mode(wifi_mode_e mode)
{
    if(mode == WIFI_MODE_SOFT_AP)
        return _config_wifi_mode(WIFI_MODE_SOFT_AP);

    if(mode == WIFI_MODE_STATION)
        return _config_wifi_mode(WIFI_MODE_STATION);

    return -1;
}

/*
int main(int argc, char *argv[])
{
    char buf[128] = "ns_host=192.168.1.232&wifi_ssid=mywifi&wifi_pswd=mypswd&ns_port=1680&apply=";

    extract_data_items(buf);
    dump_config();

    int n = save_config();
    read_config(conf_buf, n);
    dump_config();

    return 0;
}
*/
