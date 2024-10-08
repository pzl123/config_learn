#include "config_manage.h"
#include "observer.h"
// #include "fcgi/fcgi_main.h"


#include <stdio.h>
#include <dirent.h>
#include <pthread.h>
#include <unistd.h>

#include "zlog.h"
#include "fcgi_stdio.h"


#define ZLOG_INI_CONF "/home/zlgmcu/project/config-learn/bin/zlog.conf"


char *message =
"{                              \
    \"name\":\"mculover666\",   \
    \"age\": 22,                \
    \"weight\": 55.5,           \
    \"address\":                \
        {                       \
            \"country\": \"China\",\
            \"zip-code\": 111111\
        },                      \
    \"skill\": [\"c\", \"Java\", \"Python\"],\
    \"student\": false          \
}";

char *new_message =
"{ \
    \"time\":\"202409040821\", \
    \"Software_version\": 12345678 \
}";

char *new_message1 =
"{ \
    \"time\":\"202409040821\", \
    \"Software_version\": 2233445566 \
}";



void callback1(cJSON *old_cjson, cJSON *new_cjson)
{
    char *str = cJSON_Print(new_cjson);
    dzlog_info("old_cjson: %s",str);
    free(str);
    str = cJSON_Print(new_cjson);
    dzlog_info("new_cjson: %s",str);
    free(str);
}

void callback2(cJSON *old_cjson, cJSON *new_cjson)
{
    char *str = cJSON_Print(new_cjson);
    dzlog_info("old_cjson: %s",str);
    free(str);
    str = cJSON_Print(new_cjson);
    dzlog_info("new_cjson: %s",str);
    free(str);
}

void *thread_func1(void *str)
{
    cJSON *new1 = NULL; 
    new1 = cJSON_Parse((char *)str); 
    while(1)
    {   
        int32_t i = rand() % 10;
        int32_t j = rand() % 10;
        (void)config_attach("config.json", callback1);
        (void)config_attach("config.json", callback2);
        set_config("config.json", new1);
        sleep(1);
        time_t now = time(NULL);
        printf("-------------------------------time is %ld -----attach id %ld---------------------------------------------------\n",now, pthread_self());
    }
    cJSON_Delete(new1);
}


void *thread_func2(void *str)
{
    cJSON *new1 = NULL; 
    new1 = cJSON_Parse((char *)str); 
    // new1 = cJSON_Parse(new_message); 
    while(1)
    {
        int32_t i = rand() % 10;
        int32_t j = rand() % 10;
        (void)config_detach("config.json", callback1);
        (void)config_detach("config.json", callback2);
        set_config("config.json", new1);
        sleep(1);
        time_t now = time(NULL);
        printf("-------------------------------time is %ld -----detach id %ld---------------------------------------------------\n",now, pthread_self());
    }
    cJSON_Delete(new1);
}

int main(void)
{
    int32_t ret = zlog_init(ZLOG_INI_CONF);
    if (ret != 0)
    {
        printf("zlog_init failed!\n");
        (void)fprintf(stderr, "zlog_init failed!\n");
        return ret;
    }
    ret = dzlog_set_category("pcu");
    if (ret != 0)
    {
        printf("dzlog_set_category failed!\n");
        zlog_fini();
        return ret;
    }
    dzlog_info("******************** PCU START ********************");

    pthread_t tid1, tid2;
    cJSON *new1 = cJSON_Parse(new_message1); 
    
    config_init();

    (void)config_attach("config", callback1);
    (void)config_attach("config", callback2);

    set_config("config", new1);
    (void)config_detach("config", callback1);

    cJSON_Delete(new1);
    new1 = cJSON_Parse(new_message);
    set_config("config", new1);

    set_default_config("default_cde", new1);
    set_config("cde", new1);
    // // // pthread_create(&tid1, NULL, thread_func1, (void *)new_message);
    // // // pthread_create(&tid2, NULL, thread_func2, (void *)new_message1);

    // // // pthread_join(tid1, NULL);
    // // // pthread_join(tid2, NULL);

    // // // get_default("default_config.json", &new1);
    // // // str = cJSON_Print(new1);
    // // // printf("%s\n", str); 
    // // // free(str);

    cJSON_Delete(new1);
    new1 = NULL; // 避免悬空指针

    // // print_hash_table(ALL_CONFIG_FILE);
    // // printf("\n");
    // // print_hash_table(ALL_DEFAULT_FILE);
    config_clear();

    zlog_fini();
    return 0;
}