#include "get_config.h"

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
"{                              \
    \"name\":\"sadasdasdasdasdasdasdasd\",   \
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


int main() {

    config_file_init_to_hashtable(CONFIG_PATH);
    char *filename = "abc.json";
    cJSON *cjson_test = cJSON_Parse(message);
    char *json_str = cJSON_Print(cjson_test);
    set_config_file(filename, "./../web/", cjson_test);
    print_hash_table(ALL_CONFIG_FILE);

    filename = "abc.json";
    cjson_test = cJSON_Parse(new_message);
    json_str = cJSON_Print(cjson_test);
    set_config_file(filename, "./../web/", cjson_test);
    print_hash_table(ALL_CONFIG_FILE);
    return 0;
}