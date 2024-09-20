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
    void (*callback)(int num);
    int owner_num;
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
    pthread_rwlock_t valueLock;
    observer_t *owners; // 存储拥有者
    pthread_rwlock_t ownersLock;
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
 * @brief 指定hash table 打印
 *
 * @param table 指定的表头 ALL_CONFIG_FILE or ALL_DEFAULT_FILE 配置 或 默认配置
 */
void print_hash_table(file_struct_t * table);

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
 * 观察者函数
 * ----------------------------------------------------------------------------------------------------------
 */

/**
 * @brief 删除指定config文件的指定观察者
 * 
 * @param table 指定hash table 头 ALL_CONFIG_FILE or ALL_DEFAULT_FILE 配置 或 默认配置
 * @param config 要删除观察者的config文件
 * @param num 要删除的观察者的编号
 */
bool detach(file_struct_t **table, const char *config, int num);

/**
 * @brief 添加指定config文件的指定观察者, 从owners链表中尾添加
 * 
 * @param table 指定hash table 头 ALL_CONFIG_FILE or ALL_DEFAULT_FILE 配置 或 默认配置
 * @param config 要添加观察者的config文件
 * @param num 要添加的观察者的编号
 * @param callback 添加的观察者的回调函数
 */
bool attach(file_struct_t **table, const char *config, int num, void (*callback)(int num));


/**
 * @brief 通知指定config文件的所有观察者
 * 
 * @param table 指定hash table 头 ALL_CONFIG_FILE or ALL_DEFAULT_FILE 配置 或 默认配置
 * @param config 要通知的config文件
 */
bool notify(file_struct_t **table, const char *config);

/**
 * @brief 删除指定config文件的所有观察者
 * 
 * @param s 指定config文件
 */
void delete_all_owners(file_struct_t *s);

void callback(int num); // 测试回调函数


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _CONFIG_MANAGE_H_ */