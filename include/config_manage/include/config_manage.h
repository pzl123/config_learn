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


/**
 * @brief 拥有者结构体
 * 
 * @owner_num 拥有者编号
 * @callback 回调函数
 */
typedef struct observer_list{
    void (*callback)(cJSON *old_value, cJSON *new_value);
    struct observer_list *prev;
    struct observer_list *next;
} observer_t;


/**
 * @brief 文件结构体
 * 
 * @path 文件路径
 * @key 文件名
 * @value 文件内容
 * @valueLock 读写锁
 * @owners 拥有者,是一个双向链表
 * @ownersLock 拥有者读写锁
 * @hh uthash hash handle
 */
typedef struct
{
    char path[100];
    char key[100];
    cJSON *value;
    pthread_rwlock_t value_lock;
    observer_t *owners;
    pthread_rwlock_t owners_lock;
    UT_hash_handle hh;
} file_struct_t;



/**
 * ----------------------------------------------------------------------------------------------------------
 * 管理函数
 * ----------------------------------------------------------------------------------------------------------
 */

/**
 * @brief 初始化配置文件
 */
bool all_config_init(void);

/**
 * @brief 清空所有hash table
 */
void clear_all_hash_table(void);





/**
 * ----------------------------------------------------------------------------------------------------------
 * config操作函数
 * ----------------------------------------------------------------------------------------------------------
 */

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
 * @brief Get the config object 获得的对象需要手动释放
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
 * @brief Get the default object 获得的对象需要手动释放
 *
 * @param name 要匹配的default文件名
 * @param config 匹配json对象存放区
 * @return true 获取成功
 * @return false 获取失败
 */
bool get_default(const char *name, cJSON **config);



/**
 * ----------------------------------------------------------------------------------------------------------
 * 通知函数
 * ----------------------------------------------------------------------------------------------------------
 */

/**
 * @brief 删除指定config文件的指定拥有者
 * 
 * @param table 指定hash table 头 ALL_CONFIG_FILE or ALL_DEFAULT_FILE 配置 或 默认配置
 * @param config 要删除拥有者的config文件
 * @param num 要删除的拥有者的编号
 */
bool detach(const char *name, void (*callback)(cJSON *old_value, cJSON *new_value));

/**
 * @brief 添加指定config文件的指定拥有者, 从owners链表中尾添加
 * 
 * @param config 要添加拥有者的config文件
 * @param callback 添加的拥有者的回调函数
 */
bool attach(const char *name, void (*callback)(cJSON *old_value, cJSON *new_value));




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _CONFIG_MANAGE_H_ */