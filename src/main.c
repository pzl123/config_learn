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

    config_init();  
    cJSON * new = cJSON_CreateObject();
    cJSON* config = cJSON_Parse(message);
    set_config("config.json",config);
    print_hash_table();

    config = cJSON_Parse(new_message);
    set_config("config.json",config);
    print_hash_table();

    get_config("config.json",&new);
    char * str  = cJSON_Print(new);
    printf("%s\n",str);


    cJSON_Delete(config);
    cJSON_Delete(new);
    clear_hash_table();
    return 0;
}