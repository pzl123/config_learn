#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "uthash.h" // 包含 UT_hash_handle 的头文件
#include "cJSON.h"

#define MAX_PATH_LEN 256
#define MAX_KEY_LEN 100

typedef struct Observer_t {
    void (*callback)(void *, void *);
    void *arg;
} Observer_t;

typedef struct file_struct_t {
    char path[MAX_PATH_LEN];
    char key[MAX_KEY_LEN];
    cJSON *value;
    Observer_t *owners[2]; // 存储观察者的数组
    UT_hash_handle hh;
} file_struct_t;

// 添加观察者
void add_owner(file_struct_t *config, void (*callback)(void *, void *), void *arg) {
    assert(config != NULL);

    for (int i = 0; i < 2; i++) {
        if (config->owners[i] == NULL) {
            config->owners[i] = (Observer_t *)malloc(sizeof(Observer_t));
            if (config->owners[i] == NULL) {
                perror("malloc");
                return;
            }
            config->owners[i]->callback = callback;
            config->owners[i]->arg = arg;
            return;
        }
    }
    // 如果所有位置都被占用了，则无法添加新的观察者
    fprintf(stderr, "Cannot add more owners to this config item.\n");
}

// 通知观察者
void notify_owners(file_struct_t *config) {
    assert(config != NULL);

    for (int i = 0; i < 2; i++) {
        if (config->owners[i] != NULL) {
            config->owners[i]->callback(config->owners[i]->arg, config);
        }
    }
}

// 更新配置项并通知观察者
void update_config(file_struct_t *config, cJSON *newValue) {
    assert(config != NULL);

    // 更新配置项的值
    cJSON_free(config->value);
    config->value = newValue;

    // 通知所有注册的观察者
    notify_owners(config);
}

// 回调函数示例
void callback(void *arg, void *config_ptr) {
    file_struct_t *config = (file_struct_t *)config_ptr;
    printf("callback: %s/%s = %s\n", config->path, config->key, cJSON_PrintUnformatted(config->value));
}

int main() {
    // 创建 JSON 对象
    cJSON *json_value = cJSON_CreateString("Hello, World!");
    cJSON *new_value = cJSON_CreateString("New Value");

    // 创建配置项
    file_struct_t *config = (file_struct_t *)malloc(sizeof(file_struct_t));
    if (config == NULL) {
        perror("malloc");
        return 1;
    }

    strncpy(config->path, "/path/to/file", MAX_PATH_LEN - 1);
    strncpy(config->key, "key1", MAX_KEY_LEN - 1);
    config->value = json_value;

    // 添加观察者
    add_owner(config, callback, NULL);
    add_owner(config, callback, NULL);

    // 更新配置项并通知观察者
    update_config(config, new_value);

    // 清理资源
    cJSON_Delete(json_value);
    cJSON_Delete(new_value);
    free(config);

    return 0;
}