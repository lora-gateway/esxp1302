#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <web_config.h>

/*
 * Chunis Deng (chunchengfh@gmail.com)
 */

typedef struct {
    tag_e tag;
    char name[16];
    char *val;
    int len;  // length of the string pointed by val
} config_s;

config_s config[CONFIG_NUM] = {
    { WIFI_SSID, "wifi_ssid", NULL, 0 },
    { WIFI_PASSWORD, "wifi_pswd", NULL, 0 },
    { NS_HOST, "ns_host", NULL, 0 },
    { NS_PORT, "ns_port", NULL, 0 },
    { GW_ID, "gw_id", NULL, 0 }
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
        p[n] = '\0';
        if(config[tag].val)
            free(config[tag].val);
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
    }
}

void read_config(char *buf, int buf_len)
{
    int index = 0;
    int len;
    char *p = NULL;
    tag_e tag;

    while(1) {
        tag = buf[index++];
        if(tag == CONFIG_END)
            break;

        if(tag < CONFIG_NUM){
            len = buf[index++];
            p = malloc(len + 1);
            if(!p) {
                printf("Warning: malloc() for tag = %d failed\n", tag);
                continue;  // just ignore if failed
            }

            strncpy(p, buf + index, len);
            p[len + 1] = '\0';
            if(config[tag].val)  // should not needed
                free(config[tag].val);
            config[tag].val = p;
            config[tag].len = len;
            index += len;

            if(index >= buf_len)
                break;
        }
    }
}

char conf_buf[128];
int save_config(void)
{
    int index = 0;

    for(int i = 0; i < CONFIG_NUM; i++) {
        if(config[i].val != NULL){
            conf_buf[index++] = config[i].tag;
            conf_buf[index++] = config[i].len;
            strncpy(conf_buf+index, config[i].val, config[i].len);
            index += config[i].len;
        }
    }
    conf_buf[index] = CONFIG_END;  // end of the config
    printf("saved config length = %d\n", index);

    return index;
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
