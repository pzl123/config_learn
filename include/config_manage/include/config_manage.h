#ifndef _CONFIG_MANAGE_H_
#define _CONFIG_MANAGE_H_

// #include "observer.h"

#include <stdbool.h>
#include <stdio.h>

#include "uthash.h"
#include "utlist.h"
#include "cJSON.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


#define CONFIG_PATH "./../config_file_dir/config/"
#define DEFAULT_CONFIG_PATH "./../config_file_dir/default/"
#define MAX_PATH_LEN (1024)




typedef struct{
    void (*callback)(int num);
    int owner_num;
} observer_t;

typedef struct
{
    char path[MAX_PATH_LEN];
    char key[100];
    cJSON *value;
    observer_t *owners[2]; // 存储拥有者
    UT_hash_handle hh;
} file_struct_t;

extern file_struct_t *ALL_CONFIG_FILE;
extern file_struct_t *ALL_DEFAULT_FILE;


/**
 * @brief Set the config object
 *
 * @param name 要匹配的config文件名
 * @param config 要设置的json对象
 * @return true 设置文件config成功
 * @return false 设置失败
 */
bool set_config(const char *name, const cJSON *config);

/**
 * @brief Get the config object
 *
 * @param name 要匹配的config文件名
 * @param config 匹配json对象存放区
 * @return true 获取成功
 * @return false 获取失败
 */
bool get_config(const char *name, cJSON **config);

/**
 * @brief Set the default object 同时在default变化时同步config
 *
 * @param name 要改变的指定的default文件名，命名规则为"default_xxx.json"
 * @param config 要更换的json信息
 * @return true 设置成功
 * @return false 设置失败
 */
bool set_default(const char *name, const cJSON *config);

/**
 * @brief Get the default object
 *
 * @param name 要匹配的default文件名
 * @param config 匹配json对象存放区
 * @return true 获取成功
 * @return false 获取失败
 */
bool get_default(const char *name, cJSON **config);

/**
 * @brief 指定配置文件的路径和系统运行中默认的存储表头
 *
 * @param PATH 配置文件的路径  CONFIG_PATH or DEFAULT_CONFIG_PATH  配置 或 默认配置
 * @param table 默认的存储表头 ALL_CONFIG_FILE or ALL_DEFAULT_FILE 配置 或 默认配置
 * @return true
 * @return false
 */
bool config_init(char *PATH, file_struct_t **table);

/**
 * @brief 指定hash table 打印
 *
 * @param table 指定的表头 ALL_CONFIG_FILE or ALL_DEFAULT_FILE 配置 或 默认配置
 */
void print_hash_table(file_struct_t * table);

/**
 * @brief 指定hash table 删除
 *
 * @param table 要删除的hash table 头 ALL_CONFIG_FILE or ALL_DEFAULT_FILE 配置 或 默认配置
 */
void clear_hash_table(file_struct_t *table);






void callback(int num)
{
    printf("callback from owner_num %d\n", num);
}


// 添加一个owner(该owner标识为num)到指定config中的owners数组
void add_owner(file_struct_t **table, const char *config, int num, void (*callback)(int num))
{
    file_struct_t *s = NULL;
    HASH_FIND_STR(*table, config, s);
    if(s == NULL)
    {
        printf("config file %s not exist\n", config);
        return;
    }
    else
    {
        printf("config file %s exist\n", config);
        for(int i = 0; i < 2; i++)
        {
            if( NULL == s->owners[i])
            {
                s->owners[i] = (observer_t *)malloc(sizeof(observer_t));
                if (s->owners[i] == NULL) {
                    fprintf(stderr, "Memory allocation failed\n");
                    exit(EXIT_FAILURE);
                }
                s->owners[i]->callback = callback;
                s->owners[i]->owner_num = num;
                return;
            }
        }
    }
}

void dele_owner(file_struct_t **table, const char *config, int num)
{
    file_struct_t *s = NULL;
    HASH_FIND_STR(*table, config, s);
    if(s == NULL)
    {
        printf("config file %s not exist\n", config);
        return;
    }
    else
    {
        printf("config file %s exist\n", config);
        for(int i = 0; i < 2; i++)
        {
            if( NULL != (*table)->owners[i])
            {
                if((*table)->owners[i]->owner_num == num)
                {
                    free((*table)->owners[i]);
                    (*table)->owners[i] = NULL;
                    return;
                }
            }
        }
    }
}

void notify_owner(file_struct_t **table, const char *config)
{
    file_struct_t *s = NULL;
    HASH_FIND_STR(*table, config, s);
    if(s == NULL)
    {
        printf("config file %s not exist\n", config);
        return;
    }
    else
    {
        printf("config file %s exist\n", config);
        for(int i = 0; i < 2; i++)
        {
            if( NULL != (*table)->owners[i])
            {
                (*table)->owners[i]->callback((*table)->owners[i]->owner_num);
            }
        }
    }
}




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _CONFIG_MANAGE_H_ */