#include "config_manage.h"
#include "observer.h"


#include <stdio.h>
#include <dirent.h>
#include <pthread.h>
 #include <unistd.h>


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

extern file_struct_t *ALL_CONFIG_FILE;
extern file_struct_t *ALL_DEFAULT_FILE;

void *thread_func1(void *str)
{
    cJSON *new1 = NULL; 
    new1 = cJSON_Parse((char *)str); 
    while(1)
    {   
        int i = rand() % 10;
        int j = rand() % 10;
        (void)attach(&ALL_CONFIG_FILE, "config.json", i, callback);
        (void)attach(&ALL_CONFIG_FILE, "config.json", j, callback);

        (void)attach(&ALL_DEFAULT_FILE, "default_config.json", i, callback);
        (void)attach(&ALL_DEFAULT_FILE, "default_config.json", j, callback);
        // printf("thread %ld is running\n", pthread_self());
        set_default("default_config.json", new1);
        // printf("thread %ld is end, set default success\n",pthread_self());
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
    while(1)
    {
        int i = rand() % 10;
        int j = rand() % 10;
        (void)detach(&ALL_CONFIG_FILE, "config.json", i);
        (void)detach(&ALL_CONFIG_FILE, "config.json", j);

        (void)detach(&ALL_DEFAULT_FILE, "default_config.json", i);
        (void)detach(&ALL_DEFAULT_FILE, "default_config.json", j);
        // printf("thread %ld is running\n", pthread_self());
        set_default("default_config.json", new1);
        // printf("thread %ld is end, set default success\n",pthread_self());
        sleep(1);
        time_t now = time(NULL);
        printf("-------------------------------time is %ld -----detach id %ld---------------------------------------------------\n",now, pthread_self());
    }
    cJSON_Delete(new1);
}

int main(void)
{
    pthread_t tid1, tid2;

    all_config_init();



    pthread_create(&tid1, NULL, thread_func1, (void *)message);
    pthread_create(&tid2, NULL, thread_func2, (void *)new_message);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    // get_default("default_config.json", &new1);
    // str = cJSON_Print(new1);
    // printf("%s\n", str); 
    // free(str);

    // cJSON_Delete(new1);
    // new1 = NULL; // 避免悬空指针

    // print_hash_table(ALL_CONFIG_FILE);
    // printf("\n");
    // print_hash_table(ALL_DEFAULT_FILE);
    clear_all_hash_table();

    return 0;
}