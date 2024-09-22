#ifndef _CONFIG_MANAGE_H_
#define _CONFIG_MANAGE_H_

#include <stdbool.h>
#include <stdio.h>

#include "uthash.h"
#include "utlist.h"
#include "cJSON.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* 配置变更回调 */
typedef void (*on_config_change)(cJSON *old_value, cJSON *new_value);

/**
 * @brief 初始化配置模块
 *
 * @return bool
 */
bool config_init(void);

/**
 * @brief 清理配置模块
 */
void config_clear(void);


/**
 * @brief 设置默认配置
 *
 * @param name 配置表名
 * @param config 默认配置
 * @return bool
 */
bool set_default_config(const char *name, const cJSON *config);

/**
 * @brief 获取默认配置
 *
 * @param name 配置表名
 * @param config 默认配置，外部管理动态内存
 * @return bool
 */
bool get_default_config(const char *name, cJSON **config);

/**
 * @brief 设置配置
 *
 * @param name 配置表名
 * @param config 配置
 * @return bool
 */
bool set_config(const char *name, const cJSON *config);

/**
 * @brief 获取配置
 *
 * @param name 配置表名
 * @param config 配置，外部管理动态内存
 * @return bool
 */
bool get_config(const char *name, cJSON **config);

/**
 * @brief 注册配置变更回调
 *
 * @param name 配置表名
 * @param cb 回调函数
 */
bool config_attach(const char *name, on_config_change cb);

/**
 * @brief 取消注册配置变更回调
 *
 * @param name 配置表名
 * @param cb 回调函数
 */
bool config_detach(const char *name, on_config_change cb);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _CONFIG_MANAGE_H_ */
