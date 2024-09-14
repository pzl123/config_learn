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


int main() {
    cJSON *new = NULL; // 初始化为 NULL
    char *str = NULL;

    config_init(CONFIG_PATH, &ALL_CONFIG_FILE);

    add_owner(&ALL_CONFIG_FILE, "config.json", 1, callback);
    add_owner(&ALL_CONFIG_FILE, "config.json", 2, callback);

    // file_struct_t *s = NULL;
    // HASH_FIND_STR(ALL_CONFIG_FILE, "config.json", s);
    // printf("s->owners[0]: %p\n", s->owners[0]);
    // printf("s->owners[1]: %p\n", s->owners[1]);

    // printf("ALL_CONFIG_FILE->owners[0]: %p\n", ALL_CONFIG_FILE->owners[0]);
    // printf("ALL_CONFIG_FILE->owners[1]: %p\n", ALL_CONFIG_FILE->owners[1]);
    
    new = cJSON_Parse(new_message);
    // new = cJSON_Parse(message);
// 
    set_config("config.json", new);

    // get_default("default_config.json",&new);
    // str  = cJSON_Print(new);
    // printf("%s\n",str);

    // free(str); // 释放字符串
    cJSON_Delete(new); // 释放 cJSON 对象
    new = NULL; // 避免悬空指




    // cJSON_Delete(config);
    //  内存释放区

    // print_hash_table(ALL_CONFIG_FILE);

    clear_hash_table(ALL_CONFIG_FILE);
    // clear_hash_table(ALL_DEFAULT_FILE);





    // printf("------------------------------测试观察者模式-----------------------------\n");

    // Subject_t subject;
    // ConcreteObserver_t observer1, observer2;

    // subject_init(&subject);
    // int i = 1;
    // init_concrete_observer(&observer1,i);
    // init_concrete_observer(&observer2,i+1);

    // subject_add_observer(&subject, &observer1.base);
    // subject_add_observer(&subject, &observer2.base);

    // subject_notify_observers(&subject, "Hello, World!");

    // subject_remove_observer(&subject, &observer2.base);

    // subject_notify_observers(&subject, "Goodbye, World!");

    // free(subject.observers);



    return 0;
}

