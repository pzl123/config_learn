#ifndef _CONFIG_MANAGE_H_
#define _CONFIG_MANAGE_H_

#include <stdbool.h>

#include "uthash/uthash.h"
#include "cJSON/cJSON.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


#define CONFIG_PATH "./../config_file_dir/config/"
#define DEFAULT_CONFIG_PATH "./../config_file_dir/default/"
#define MAX_PATH_LEN (1024)

typedef struct
{
    char key[100];
    cJSON *value;
} key_and_value_t;


typedef struct
{
    char path[MAX_PATH_LEN];   
    key_and_value_t kv; 
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

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _CONFIG_MANAGE_H_ */