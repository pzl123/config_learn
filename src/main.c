#include "config_manage.h"

#include <stdio.h>
#include <dirent.h>

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


int main() {
    cJSON *new = NULL; // 初始化为 NULL
    char *str = NULL;

    config_init(CONFIG_PATH, &ALL_CONFIG_FILE);  
    // cJSON* config = cJSON_Parse(message);
    // set_config("config.json",config);
    // print_hash_table(ALL_CONFIG_FILE);

    // config = cJSON_Parse(new_message);
    // set_config("config.json",config);
    // print_hash_table(ALL_CONFIG_FILE);

    get_config("config.json",&new);
    str  = cJSON_Print(new);
    printf("%s\n",str);

    free(str); // 释放字符串
    // 清理
    cJSON_Delete(new); // 释放 cJSON 对象
    new = NULL; // 避免悬空指针


    // config_init(DEFAULT_CONFIG_PATH, &ALL_DEFAULT_FILE);  
    // print_hash_table(ALL_DEFAULT_FILE);
    // // config = cJSON_Parse(message);
    // // config = cJSON_Parse(new_message);
    // // set_default("default_config.json", config);

    // get_default("default_config.json",&new);
    // str  = cJSON_Print(new);
    // printf("%s\n",str);

    // free(str); // 释放字符串
    // cJSON_Delete(new); // 释放 cJSON 对象
    // new = NULL; // 避免悬空指


    // cJSON_Delete(config);
    //  内存释放区
    clear_hash_table(ALL_CONFIG_FILE);
    // clear_hash_table(ALL_DEFAULT_FILE);

    return 0;
}

