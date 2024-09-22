#include "config_manage.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#include "cJSON.h"
#include "cJSONx.h"
#include "utlist.h"
#include "zlog.h"


// /* 配置文件路径 */
// #define CONFIG_PATH "/mnt/data/config/current"
// /* 默认配置文件路径 */
// #define DEFAULT_CONFIG_PATH "/mnt/data/config/default"

#define CONFIG_PATH "./../config-file-dir/config"
#define DEFAULT_CONFIG_PATH "./../config-file-dir/default"


typedef struct cb_list
{
    on_config_change cb;
    struct cb_list *prev;
    struct cb_list *next;
} cb_list_t;


typedef struct
{
    char *path;   /* 配置文件路径 */
    char *name;   /* 配置表名 */
    cJSON *value; /* 配置 */
    pthread_rwlock_t value_lock;

    cb_list_t *owners;
    pthread_rwlock_t owners_lock;

    UT_hash_handle hh;
} config_manage_t;

static config_manage_t *g_config_manage_t = NULL;
static config_manage_t *g_default_config_manage_t = NULL;

static bool config_hash_init(char *PATH, config_manage_t **table);

static bool read_file_to_memory(const char *target_file, cJSON **cjson_config);
static bool write_memory_to_file(const char *config_name, const cJSON *cjson_config, char *PATH);

static bool add_config_item(config_manage_t **table, const char *config_name, const cJSON *cjson_config);
static config_manage_t *find_config_item(config_manage_t *dorc, const char *config_name);



static bool notify_owner(const char *config, cJSON *old_value, cJSON *new_value);
static cb_list_t *find_owner(config_manage_t *s, on_config_change cb);
static void delete_all_owners(config_manage_t *s);

static void printf_config(config_manage_t * table);
static char *split_str(const char *str);

bool set_config(const char *name, const cJSON *config)
{
    if(NULL == name || NULL == config)
    {
        dzlog_error("set config error, because name or config is NULL");
        return false;
    }

    int32_t config_len = strlen(name) + strlen(".json") + 1;
    char *config_name = (char *)malloc(config_len);
    if(NULL == config_name)
    {
        dzlog_error("config name %s malloc failed in set config", name);
        return false;
    }
    memset(config_name, 0, config_len);
    (void)snprintf(config_name, config_len, "%s.json", name);

    int32_t default_len = strlen(config_name) + strlen("default_") + 1;
    char *default_config_name = (char *)malloc(default_len);
    if(NULL == default_config_name)
    {
        dzlog_error("default config name %s malloc failed in set config", name);
        return false;
    }
    memset(default_config_name, 0, default_len);
    (void)snprintf(default_config_name, default_len, "default_%s", config_name);

    config_manage_t *s = find_config_item(g_config_manage_t, config_name);
    config_manage_t *default_s = find_config_item(g_default_config_manage_t, default_config_name);
    if (NULL == s && NULL == default_s)//config和default中都不存在，无此配置
    {
        dzlog_error("no such config %s in path %s and %s",config_name, CONFIG_PATH, DEFAULT_CONFIG_PATH);
        free(config_name);
        free(default_config_name);
        return false;
    }
    else if(NULL == s && NULL != default_s)//default中存在，config中不存在，需要添加到config中,并且在config路径中创建文件
    {
        dzlog_debug("no such config %s in path %s, but in path %s", config_name, CONFIG_PATH, DEFAULT_CONFIG_PATH);

        s = (config_manage_t *)malloc(sizeof(config_manage_t));
        if(NULL == s)
        {
            dzlog_error("s malloc failed in set config %s", config_name);
            free(config_name);
            free(default_config_name);
            return false;
        }
        memset(s, 0, sizeof(config_manage_t));

        s->name = strdup(config_name);
        s->value = cJSON_Duplicate(default_s->value, 1);
        int32_t s_path_len = strlen(CONFIG_PATH) + strlen(s->name) + 2;
        s->path = (char *)malloc(s_path_len);
        (void)snprintf(s->path, s_path_len+1, "%s/%s", CONFIG_PATH, s->name);
        s->owners = NULL;
        HASH_ADD_STR(g_config_manage_t, name, s);

        if (pthread_rwlock_init(&(s->value_lock), NULL) != 0)
        {
            dzlog_error("Failed to initialize valueLock in set config");
            free(s->name);
            free(s->path);
            cJSON_Delete(s->value);
            free(s);
            free(default_config_name);
            return false;
        }

        if (pthread_rwlock_init(&(s->owners_lock), NULL) != 0)
        {
            dzlog_error("Failed to initialize ownersLock in set config");
            pthread_rwlock_destroy(&(s->value_lock)); 
            free(s->name);
            free(s->path);
            cJSON_Delete(s->value);
            free(s);
            free(default_config_name);
            return false;
        }

        free(config_name);
        free(default_config_name);
        (void)write_memory_to_file(s->name, s->value, CONFIG_PATH);
        return true;
    }
    else if(NULL != s && NULL == default_s)//config中存在，default中不存在, 不允许这种情况发生
    {
        dzlog_fatal("Such a situation is not allowed to exist.");
        free(config_name);
        free(default_config_name);
        return false;
    }
    else if(NULL != s && NULL != default_s)//表已存在，无需重新添加，只要修改
    {
        (void)pthread_rwlock_wrlock(&s->value_lock);

        if(cJSON_Compare(s->value, config, true))
        {
            (void)pthread_rwlock_unlock(&s->value_lock);
            dzlog_info("Same, no need to modify.");
            free(config_name);
            free(default_config_name);
            return false;
        }
        else
        {
            if(notify_owner(config_name, s->value, (cJSON *)config))
            {
                dzlog_debug("memory file %s changed", config_name);
            }
            else
            {
                dzlog_info("notify falied, config %s changed", config_name);
            }
            cJSON_Delete(s->value);
            s->value = cJSON_Duplicate(config, 1);
            (void)write_memory_to_file(s->name, s->value,CONFIG_PATH);
            (void)pthread_rwlock_unlock(&s->value_lock);

            free(config_name);
            free(default_config_name);
            return true;
        }
    }
    else
    {
        free(config_name);
        free(default_config_name);
        dzlog_error("set config failed, need to check set config");
        return false;
    }
    
}

bool get_config(const char *name, cJSON **config)
{
    if(NULL == name || NULL == config)
    {
        dzlog_error("name or config is NULL, get config failed");
        return false;
    }

    int32_t len = strlen(name) + strlen(".json") + 1;
    char *config_name = (char *)malloc(len);
    if(NULL == config_name)
    {
        dzlog_error("config name %s malloc failed in get config", name);
        return false;
    }
    (void)snprintf(config_name, sizeof(config_name), "%s%s", name, ".json");

    config_manage_t *s =find_config_item(g_config_manage_t, config_name);
    if (NULL == s)
    {
        dzlog_error("no such default config %s in path %s", config_name, CONFIG_PATH);
        free(config_name);
        return false;
    }
    else
    {   
        (void)pthread_rwlock_rdlock(&s->value_lock);
        cJSON_Delete(*config);
        *config = cJSON_Duplicate(s->value, 1);
        (void)pthread_rwlock_unlock(&s->value_lock);

        free(config_name);
        return true;
    }
}

bool set_default_config(const char *name, const cJSON *config)
{
    if(NULL == name || NULL == config)
    {
        dzlog_error("name or config is NULL, set default config failed");
        return false;
    }

    int32_t len = strlen(name) + strlen(".json") + 1;
    char *default_config_name = (char *)malloc(len);
    if(NULL == default_config_name)
    {
        dzlog_error("config name %s malloc failed in set default config", name);
        return false;
    }
    (void)snprintf(default_config_name, len, "%s%s", name, ".json");

    config_manage_t *s =find_config_item(g_default_config_manage_t, default_config_name);
    if (NULL == s)//default 不存在则创建
    {
        dzlog_debug("default config %s not exist, create it", default_config_name);
        s = (config_manage_t *)malloc(sizeof(config_manage_t));
        if(NULL == s)
        {
            dzlog_error("s malloc failed in set default config");
            free(default_config_name);
            return false;
        }

        s->name = strdup(default_config_name);
        s->value = cJSON_Duplicate(config, 1);
        int32_t len = strlen(DEFAULT_CONFIG_PATH) + strlen(default_config_name) + 2;
        s->path = (char *)malloc(len);
        (void)snprintf(s->path, len, "%s/%s", DEFAULT_CONFIG_PATH, default_config_name);
        (void)pthread_rwlock_init(&s->value_lock, NULL);
        (void)pthread_rwlock_init(&s->owners_lock, NULL);
        HASH_ADD_STR(g_default_config_manage_t, name, s);
        (void)write_memory_to_file(s->name, s->value,DEFAULT_CONFIG_PATH);

        free(default_config_name);
        printf_config(g_default_config_manage_t);
        return true;
    }
    else
    {   
        free(default_config_name);
        (void)pthread_rwlock_wrlock(&s->value_lock);
        if(cJSON_Compare(s->value, config, true))
        {
            (void)pthread_rwlock_unlock(&s->value_lock);
            dzlog_info("Same default, no need to modify.");
            return false;
        }
        else
        {
            cJSON_Delete(s->value);
            s->value = cJSON_Duplicate(config, 1);
            if(NULL == s->value)
            {
                (void)pthread_rwlock_unlock(&s->value_lock);
                return false;
            }
            (void)write_memory_to_file(s->name, s->value,DEFAULT_CONFIG_PATH);
            (void)pthread_rwlock_unlock(&s->value_lock);

            char *buffer = split_str(s->name);
            if(NULL == buffer)
            {
                dzlog_error("split_str error");
                return false;
            }
            else
            {
                (void)set_config(buffer, config);
            }

            free(buffer);
            return true;
        }
    }
}

bool get_default_config(const char *name, cJSON **config)
{
    if(NULL == name || NULL == config)
    {
        dzlog_error("name or config is NULL, get default config failed");
        return false;
    }

    int32_t len = strlen(name) + strlen(".json") + 1;
    char *config_name = (char *)malloc(len);
    if(NULL == config_name)
    {
        dzlog_error("default config name %s malloc failed", name);
        return false;
    }
    (void)snprintf(config_name, sizeof(config_name), "%s%s", name, ".json");

    config_manage_t *s =find_config_item(g_config_manage_t, config_name);
    if (NULL == s)
    {
        dzlog_error("no such default config %s in path %s", config_name, DEFAULT_CONFIG_PATH);
        free(config_name);
        return false;
    }
    else
    {
        (void)pthread_rwlock_rdlock(&s->value_lock);
        cJSON_Delete(*config);
        *config = cJSON_Duplicate(s->value, 1);
        (void)pthread_rwlock_unlock(&s->value_lock);

        free(config_name);
        return true;
    }
}

static bool add_config_item(config_manage_t **table, const char *config_name, const cJSON *cjson_config) 
{
    if(NULL == table || NULL == config_name || NULL == cjson_config)
    {
        dzlog_error("table or config_name or cjson_config is NULL");
        return false;
    }
    int32_t len = 0;
    config_manage_t *s = NULL;
    HASH_FIND_STR(*table,config_name,s);
    if (NULL == s)
    {
        config_manage_t *s = (config_manage_t*)malloc(sizeof(config_manage_t));
        if (NULL == s)
        {
            dzlog_error("s malloc error in add_config_item");
            return false;
        }

        s->name =strdup(config_name);
        if(NULL == s->name)
        {
            free(s);
            dzlog_error("s->name %s strdup error", config_name);
            return false;
        }
        char *config_path_flag = strstr(config_name, "default");
        if(NULL == config_path_flag)
        {
            len = strlen(CONFIG_PATH) + strlen(config_name) + 2;
        }
        else
        {
            len = strlen(DEFAULT_CONFIG_PATH) + strlen(config_name) + 2;
        }
        s->path = (char*)malloc(len);
        if (NULL == s->path)
        {
            free(s->name);
            free(s);
            dzlog_error("path s->%s malloc error", config_path_flag? DEFAULT_CONFIG_PATH : CONFIG_PATH);
            return false;
        }
        (void)snprintf(s->path, len, "%s/%s", config_path_flag? DEFAULT_CONFIG_PATH : CONFIG_PATH, config_name);

        s->value = cJSON_Duplicate(cjson_config, 1);
        if (s->value == NULL)
        {
            dzlog_error("Failed to duplicate cJSON object");
            free(s->name);
            free(s->path);
            free(s);
            return false;
        }

        // 初始化配置的观察者结构体
        s->owners = NULL;

        HASH_ADD_STR(*table, name, s);

        // 初始化读写锁
        if (pthread_rwlock_init(&(s->value_lock), NULL) != 0)
        {
            dzlog_error("Failed to initialize valueLock");
            free(s->name);
            free(s->path);
            cJSON_Delete(s->value);
            free(s);
            return false;
        }

        if (pthread_rwlock_init(&(s->owners_lock), NULL) != 0)
        {
            dzlog_error("Failed to initialize ownersLock");
            pthread_rwlock_destroy(&(s->value_lock)); // 清理 valueLock
            free(s->name);
            free(s->path);
            cJSON_Delete(s->value);
            free(s);
            return false;
        }

        return true;
    }
    else
    {
        dzlog_error("file already exits in hash table");
        return true;
    }

}

bool config_init(void)
{
    int32_t ret = 0;
    ret = config_hash_init(CONFIG_PATH, &g_config_manage_t);
    if (ret == 0)
    {
        dzlog_error("Failed to initialize config");
        return false;
    }
    
    ret = config_hash_init(DEFAULT_CONFIG_PATH, &g_default_config_manage_t);
    if (ret == 0)
    {
        dzlog_error("Failed to initialize default config");
        return false;
    }
    return true;
}

static bool config_hash_init(char *path, config_manage_t **table)
{
    if(NULL == table || NULL == path)
    {
        dzlog_error("table or path is NULL on config_hash_init");
        return false;
    }
    struct dirent *file  = NULL;
    int32_t files_found = 0;

    DIR *dp = opendir(path);
    if(NULL == dp)
    {
        dzlog_error("opendir %s error", path);
        return false;
    }

    while(NULL != (file = readdir(dp)))
    {
        if(strncmp(file->d_name, ".", 1) == 0)
        {
            continue;
        }
        int32_t len = strlen(path) + strlen(file->d_name) + 2;
        char *dest_path = (char *)malloc(len);
        if (NULL == dest_path)
        {
            dzlog_warn("dest_path %s malloc error", file->d_name);
            continue;
        }
        (void)snprintf(dest_path, len, "%s/%s", path, file->d_name);

        struct stat buf;
        (void)memset(&buf, 0, sizeof(buf));
        if (0 != stat(dest_path, &buf))
        {
            dzlog_error("stat %s error", dest_path);
            free(dest_path);
            continue;
        }

        if (S_ISREG(buf.st_mode))
        {
            cJSON *cjson_config = NULL;
            if (read_file_to_memory(dest_path, &cjson_config))
            {
                if(add_config_item(table,file->d_name, cjson_config))
                {
                    dzlog_debug("Successfully loaded JSON from %s", dest_path);
                }
                else
                {
                    dzlog_error("Failed to load JSON from %s", dest_path);
                    free(dest_path);
                    continue;
                }
                files_found++;
                cJSON_Delete(cjson_config);
            }
            else
            {
                dzlog_error("Failed to load or parse JSON from %s", dest_path);
                free(dest_path);
                continue;
            }
        }
        free(dest_path);
    }
    (void)closedir(dp);

    if (files_found == 0)
    {
        dzlog_warn("No files in the target directory");
    }

    return true;
}

static bool read_file_to_memory(const char *target_file, cJSON **cjson_config)
{
    if(NULL == target_file || NULL == cjson_config)
    {
        dzlog_error("target_file or cjson_config is NULL in read_file_to_memory");
        return false;
    }
    FILE* fp = fopen(target_file,"r");
    if(NULL == fp)
    {
        dzlog_error("open file %s error", target_file);
        return false;
    }

    (void)fseek(fp,0,SEEK_END);
    int32_t file_len = ftell(fp);
    (void)fseek(fp,0,SEEK_SET);


    if(file_len <= 0)
    {
        (void)fclose(fp);
        dzlog_warn("%s file is NULL", target_file);
        return false;
    }

    char * readbuf = (char *)malloc(file_len + 1);
    if(NULL == readbuf)
    {
        dzlog_error("%s readbuf malloc error in read_file_to_memory", target_file);
        (void)fclose(fp);
        return false;
    }

    size_t bytes_read = fread(readbuf, 1, file_len, fp);
    (void)fclose(fp);

    if(bytes_read != file_len)
    {
        dzlog_error("Unable to read full file %s", target_file);
        free(readbuf);
        return false;
    }

    readbuf[file_len] = '\0';

    *cjson_config = cJSON_Parse(readbuf);
    if(*cjson_config == NULL)
    {
        dzlog_error("Failed to parse %s JSON", target_file);
        free(readbuf);
        return false;
    }

    free(readbuf);
    return true;
}

static bool write_memory_to_file(const char *config_name, const cJSON *cjson_config, char *path)
{
    if(NULL == config_name || NULL == cjson_config || NULL == path)
    {
        dzlog_error("config_name or cjson_config or path is NULL in write_memory_to_file");
        return false;
    }
    int32_t len = strlen(config_name) + strlen(path) + 2;
    char *json_string = cJSON_Print(cjson_config);
    char *target_file_path= (char *)malloc(len);
    if(NULL == target_file_path)
    {
        dzlog_error("target_file_path %s malloc error in write_memory_to_file", config_name);
        free(json_string);
        return false;
    }
    (void)snprintf(target_file_path, len, "%s/%s", path, config_name);

    FILE *target_file = fopen(target_file_path , "w");
    if(NULL == target_file)
    {
        dzlog_error("Failed to open the target file %s", target_file_path);
        free(target_file_path);
        free(json_string);
        return false;
    }
    free(target_file_path);
    
    if(NULL != json_string)
    {
        (void)fwrite(json_string, sizeof(char), strlen(json_string), target_file);
        (void)fflush(target_file);
        int32_t fd = fileno(target_file);
        if(fd != -1)
        {
            if(fsync(fd) != 0)
            {
                dzlog_error("fsync failed for %s", config_name);
                (void)fclose(target_file);
                free(json_string);
                return false;
            }
        }
        else
        {
            dzlog_error("Failed to get file %s descriptor", config_name);
            (void)fclose(target_file);
            free(json_string);
            return false;
        }
        (void)fclose(target_file);
        free(json_string);
        return true;
    }else
    {
        dzlog_error("Failed to serialize %s cJSON object to string.", config_name);
        (void)fclose(target_file);
        return false;
    }
}

static void printf_config(config_manage_t * table)
{
    if(NULL == table)
    {
        dzlog_error("table is NULL in printf_config");
        return;
    }
    config_manage_t *s = NULL;
    char *json_str = NULL;
    dzlog_debug("----------------------------------------HASH TABLE-------------------------------------");
    for (s = table; s != NULL; s = s->hh.next)
    {
        dzlog_debug("path: %s", s->path);
        dzlog_debug("config_name: %s", s->name);
        json_str = cJSON_Print(s->value);
        dzlog_debug("value:\n%s", json_str);
        free(json_str);
    }
    dzlog_debug("----------------------------------------HASH TABLE-------------------------------------\n");
    
}

static config_manage_t *find_config_item(config_manage_t * table, const char *config_name)
{
    if(NULL == table || NULL == config_name)
    {
        dzlog_error("table or config_name is NULL in find_config_item");
        return NULL;
    }
    config_manage_t *s;
    HASH_FIND_STR(table, config_name, s);
    return s;
}

void clear_hash_table(config_manage_t *table)
{
    if(NULL == table)
    {
        dzlog_error("table is NULL in clear_hash_table");
        return;
    }
    config_manage_t *current_user;
    config_manage_t *tmp;
    HASH_ITER(hh, table, current_user, tmp)
    {
        free(current_user->name);
        free(current_user->path);
        cJSON_Delete(current_user->value);

        delete_all_owners(current_user);

        (void)pthread_rwlock_destroy(&current_user->value_lock);
        (void)pthread_rwlock_destroy(&current_user->owners_lock);

        HASH_DEL(table, current_user);
        
        free(current_user);      
    }
}



void config_clear(void)
{
    clear_hash_table(g_config_manage_t);
    clear_hash_table(g_default_config_manage_t);
}



bool config_attach(const char *name, on_config_change cb)
{
    if(NULL == name || NULL == cb)
    {
        dzlog_error("name or cb is NULL in config_attach");
        return false;
    }

    int32_t len = strlen(name) + strlen(".json") +1;
    char *config_name = (char *)malloc(len);
    if(NULL == config_name)
    {
        dzlog_error("config_name %s malloc failed in config_attach", name);
        return false;
    }
    (void)snprintf(config_name, len, "%s%s", name, ".json");

    config_manage_t *s = NULL;
    HASH_FIND_STR(g_config_manage_t, config_name, s);
    free(config_name);
    if(s == NULL)
    {
        return false;
    }
    else
    {
        cb_list_t *new_owner = (cb_list_t *)malloc(sizeof(cb_list_t));
        if(NULL == new_owner)
        {
            dzlog_error("new_owner malloc failed");
            return false;
        }
        new_owner->cb = cb;
        new_owner->next = NULL;
        new_owner->prev = NULL;

        (void)pthread_rwlock_wrlock(&s->owners_lock);
        if( NULL == s->owners)
        {   
            s->owners = new_owner;
        }
        else
        {
            cb_list_t *head = s->owners;
            while(head->next != NULL)
            {
                head = head->next;
            }
            head->next = new_owner;
            new_owner->prev = head;
        }
        (void)pthread_rwlock_unlock(&s->owners_lock);
        return true;
    }
}

static cb_list_t *find_owner(config_manage_t *table, on_config_change cb)
{
    if(NULL == table || NULL == cb)
    {
        dzlog_error("table or cb is NULL in find_owner");
        return NULL;
    }
    (void)pthread_rwlock_rdlock(&table->owners_lock);
    cb_list_t *head = table->owners;
    while(head != NULL)
    {
        if(head->cb == cb)
        {
            (void)pthread_rwlock_unlock(&table->owners_lock);
            return head;
        }
        head = head->next;
    }
    (void)pthread_rwlock_unlock(&table->owners_lock);
    return NULL;
}


bool config_detach(const char *name, on_config_change cb)
{
    if(NULL == name || NULL == cb)
    {
        dzlog_error("name or cb is NULL in config_detach");
        return false;
    }
    int32_t len = strlen(name) + strlen(".json") +1;
    char *config_name = (char *)malloc(len);
    if(NULL == config_name)
    {
        dzlog_error("config_name %s malloc failed in config_detach", name);
        return false;
    }
    (void)snprintf(config_name, len, "%s%s", name, ".json");

    config_manage_t *s = NULL;
    HASH_FIND_STR(g_config_manage_t, config_name, s);
    free(config_name);
    if(s == NULL)
    {
        dzlog_error("%s find s is NULL in config_detach", name);
        return  false;
    }
    else
    {
        cb_list_t *finded_owner = find_owner(s,cb);
        if (finded_owner == NULL)
        {
            dzlog_error("%s finded_owner is NULL in config_detach", name);
            return false;
        }
        (void)pthread_rwlock_wrlock(&s->owners_lock);
        if(finded_owner == s->owners)
        { 
            s->owners = finded_owner->next;
            if(s->owners != NULL)
            {
                s->owners->prev = NULL;
            }
            free(finded_owner);
            (void)pthread_rwlock_unlock(&s->owners_lock);
            return true;
        }
        else if(finded_owner->prev != NULL)
        {
            finded_owner->prev->next = finded_owner->next;
            if(finded_owner->next != NULL)
            {
                finded_owner->next->prev = finded_owner->prev;
            }
            free(finded_owner);
            (void)pthread_rwlock_unlock(&s->owners_lock);
            return true;
        }
        else if (finded_owner->next == NULL && finded_owner->prev != NULL)
        {
            finded_owner->prev->next = NULL;
            free(finded_owner);
            (void)pthread_rwlock_unlock(&s->owners_lock);
            return true;
            
        }
        else
        {
            free(finded_owner);
            (void)pthread_rwlock_unlock(&s->owners_lock);
            dzlog_error("Error: Unknown node type");
            return false;
        }
    }
}

static bool notify_owner(const char *config, cJSON *old_value, cJSON *new_value)
{
    if(NULL == config || NULL == old_value || NULL == new_value)
    {
        dzlog_error("config or old_value or new_value is NULL in notify_owner");
        return false;
    }
    config_manage_t *s = NULL;
    HASH_FIND_STR(g_config_manage_t, config, s);
    if(s == NULL)
    {
        dzlog_error("file %s not exist, can't notify", config);
        return  false;
    }
    else
    {
        (void)pthread_rwlock_rdlock(&s->owners_lock);
        cb_list_t *head = s->owners;
        while(head != NULL)
        {
            head->cb(old_value, new_value);
            head = head->next;
        }
        (void)pthread_rwlock_unlock(&s->owners_lock);
        return true;
    }
    
}

//删除所有节点
static void delete_all_owners(config_manage_t *s)
{
    if(s == NULL)
    {
        dzlog_error("Error: s is NULL");
        return;
    }
    (void)pthread_rwlock_wrlock(&s->owners_lock);
    cb_list_t *head = s->owners;
    while (head != NULL)
    {
        cb_list_t *temp = head;
        head = head->next;
        free(temp);
    }
    s->owners = NULL;
    (void)pthread_rwlock_unlock(&s->owners_lock);
}


static char *split_str(const char *str)
{
    if(NULL == str)
    {
        dzlog_error("Error: str is NULL in split_str");
        return NULL;
    }
    const char *start = strchr(str, '_'); 
    if (start == NULL)
    {
        dzlog_error("Error: No delimiter found.");
        return NULL;
    }

    int32_t len = strlen(start + 1); // 获取下划线后字符串的长度
    char *ret2 = (char *)malloc(len + 1); // 分配足够的内存
    if (ret2 == NULL)
    {
        dzlog_error("Error: Could not allocate memory.");
        return NULL;
    }

    // 复制下划线后的字符串到新分配的内存中
    strncpy(ret2, start + 1, len);
    ret2[len] = '\0'; 

    return ret2;
}
