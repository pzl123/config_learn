#include "config_manage.h"
#include "observer.h"


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

extern file_struct_t *ALL_CONFIG_FILE;
extern file_struct_t *ALL_DEFAULT_FILE;

char *tmp = NULL;
int main() {
    cJSON *new = NULL; // 初始化为 NULL
    char *str = NULL;

    all_config_init();


    attach(&ALL_CONFIG_FILE, "config.json", 1, callback);
    attach(&ALL_CONFIG_FILE, "config.json", 2, callback);

    attach(&ALL_DEFAULT_FILE, "default_config.json", 1, callback);
    attach(&ALL_DEFAULT_FILE, "default_config.json", 2, callback);

    
    // new = cJSON_Parse(new_message);
    new = cJSON_Parse(message);

    // set_config("config.json", new);
    set_default("default_config.json", new);

    get_default("default_config.json",&new);
    str  = cJSON_Print(new);
    printf("%s\n",str);
    free(str); // 释放字符串

    cJSON_Delete(new); // 释放 cJSON 对象
    new = NULL; // 避免悬空指



    clear_all_hash_table();

    return 0;
}

