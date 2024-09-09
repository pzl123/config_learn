#ifndef _CONFIG_MANAGE_H_
#define _CONFIG_MANAGE_H_

#include <stdbool.h>

#include "uthash/uthash.h"
#include "cJSON/cJSON.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


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


// bool set_default(const char *name, const cJSON *config);

// bool get_default(const char *config_name, const cJSON *config);

/**
 * @brief 从config 文件夹读出所有配置，添加到内存中hash table中
 * 
 * @return true 成功
 * @return false 失败
 */
bool config_init(void);

/**
 * @brief 查看当前config hash table
 * 
 */
void print_hash_table(void);

/**
 * @brief 删除hash table 回收内存
 * 
 */
void clear_hash_table(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _CONFIG_MANAGE_H_ */