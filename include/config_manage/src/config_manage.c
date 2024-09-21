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

#define CONFIG_PATH "./../config_file_dir/config/"
#define DEFAULT_CONFIG_PATH "./../config_file_dir/default/"

typedef struct cb_list {
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
        dzlog_info("name or config is NULL");
        return false;
    }
    int len = strlen(name) + strlen(".json")+1;
    char *config_name = malloc(len);
    if(NULL == config_name)
    {
        dzlog_info("malloc failed");
        return false;
    }
    memset(config_name, 0, len);
    (void)snprintf(config_name, len, "%s%s", name, ".json");
    config_manage_t *s =find_config_item(g_config_manage_t, config_name);
    if (NULL == s)
    {
        dzlog_info("no such config %s in path %s",config_name,CONFIG_PATH);
        free(config_name);
        return false;
    }
    else
    {
        // 加写锁
        (void)pthread_rwlock_wrlock(&s->value_lock);
        if(cJSON_Compare(s->value, config, true))
        {
            (void)pthread_rwlock_unlock(&s->value_lock);
            dzlog_info("Same, no need to modify.");
            free(config_name);
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
            // 解写锁
            (void)pthread_rwlock_unlock(&s->value_lock);
            free(config_name);
            return true;
        }
    }
    
}

bool get_config(const char *name, cJSON **config)
{
    if(NULL == name || NULL == config)
    {
        dzlog_info("name or config is NULL");
        return false;
    }
    char config_name [100];
    (void)snprintf(config_name, sizeof(config_name), "%s%s", name, ".json");
    config_manage_t *s =find_config_item(g_config_manage_t, config_name);
    if (NULL == s)
    {
        dzlog_info("no such default config %s in path %s",config_name,DEFAULT_CONFIG_PATH);
        return false;
    }
    else
    {   
        // 加读锁
        (void)pthread_rwlock_rdlock(&s->value_lock);
        cJSON_Delete(*config);
        *config = cJSON_Duplicate(s->value, 1);
        // 解读锁
        (void)pthread_rwlock_unlock(&s->value_lock);
        return true;
    }
}

bool set_default_config(const char *name, const cJSON *config)
{
    if(NULL == name || NULL == config)
    {
        dzlog_info("name or config is NULL");
        return false;
    }
    char config_name [100];
    (void)snprintf(config_name, sizeof(config_name), "%s%s", name, ".json");
    config_manage_t *s =find_config_item(g_config_manage_t, config_name);
    if (NULL == s)
    {
        dzlog_info("no such default config %s in path %s",config_name,DEFAULT_CONFIG_PATH);
        return false;
    }
    else
    {   
         // 加写锁
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
            // 解写锁
            (void)pthread_rwlock_unlock(&s->value_lock);

            char *buffer = split_str(s->name);
            if(NULL == buffer)
            {
                dzlog_info("split_str error");
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
        dzlog_info("name or config is NULL");
        return false;
    }
    char config_name [100];
    (void)snprintf(config_name, sizeof(config_name), "%s%s", name, ".json");
    config_manage_t *s =find_config_item(g_config_manage_t, config_name);
    if (NULL == s)
    {
        dzlog_info("no such default config %s in path %s",config_name,DEFAULT_CONFIG_PATH);
        return false;
    }
    else
    {
        // 加读锁
        (void)pthread_rwlock_rdlock(&s->value_lock);
        cJSON_Delete(*config);
        *config = cJSON_Duplicate(s->value, 1);
        // 解读锁
        (void)pthread_rwlock_unlock(&s->value_lock);
        return true;
    }
}

static bool add_config_item(config_manage_t **table, const char *config_name, const cJSON *cjson_config) 
{
    if(NULL == table || NULL == config_name || NULL == cjson_config)
    {
        dzlog_info("table or config_name or cjson_config is NULL");
        return false;
    }
    config_manage_t *s = NULL;
    int len = 0;
    HASH_FIND_STR(*table,config_name,s);
    if (NULL == s)
    {
        s = (config_manage_t*)malloc(sizeof(config_manage_t));
        if (NULL == s)
        {
            dzlog_info("malloc error");
            return false;
        }

        s->name =strdup(config_name);
        if (NULL == s->name)
        {
            free(s);
            dzlog_info("malloc error");
            return false;
        }
        char *config_path_flag = strstr(config_name, "default");
        if(NULL == config_path_flag)
        {
            len = strlen(CONFIG_PATH) + strlen(config_name) + 1;
        }
        else
        {
            len = strlen(DEFAULT_CONFIG_PATH) + strlen(config_name) + 1;
        }
        s->path = (char*)malloc(len);
        if (NULL == s->path)
        {
            free(s->name);
            free(s);
            dzlog_info("malloc error");
            return false;
        }
        (void)snprintf(s->path, len, "%s%s", config_path_flag? DEFAULT_CONFIG_PATH : CONFIG_PATH, config_name);

        s->value = cJSON_Duplicate(cjson_config, 1);
        if (s->value == NULL)
        {
            dzlog_info("Failed to duplicate cJSON object");
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
            dzlog_info("Failed to initialize valueLock");
            free(s->name);
            free(s->path);
            cJSON_Delete(s->value);
            free(s);
            return false;
        }

        if (pthread_rwlock_init(&(s->owners_lock), NULL) != 0)
        {
            dzlog_info("Failed to initialize ownersLock");
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
        dzlog_info("file already exits in hash table");
        return true;
    }

}

bool config_init(void)
{
    int ret = 0;
    ret = config_hash_init(CONFIG_PATH, &g_config_manage_t);
    if (ret == 0)
    {
        dzlog_info("Failed to initialize config");
        return false;
    }
    
    ret = config_hash_init(DEFAULT_CONFIG_PATH, &g_default_config_manage_t);
    if (ret == 0)
    {
        dzlog_info("Failed to initialize default config");
        return false;
    }
    return true;
}

static bool config_hash_init(char *path, config_manage_t **table)
{
    if(NULL == table || NULL == path)
    {
        dzlog_info("table or path is NULL 1");
        return false;
    }
    struct dirent *file  = NULL;
    int files_found = 0;
    int len = 0;
    char *dest_path = NULL;
    DIR *dp = opendir(path);
    if(NULL == dp)
    {
        dzlog_info("opendir");
        return false;
    }

    while(NULL != (file = readdir(dp)))
    {
        if(strncmp(file->d_name, ".", 1) == 0)
            continue;
        len = strlen(path) + strlen(file->d_name) + 1;
        dest_path = (char *)malloc(len);  
        (void)snprintf(dest_path, len, "%s%s", path, file->d_name);
        dzlog_debug("---------------------------------------------------dest_path is %s", dest_path);
        dzlog_debug("file_name is %s", file->d_name);


        struct stat buf;
        (void)memset(&buf, 0, sizeof(buf));
        if (0 != stat(dest_path, &buf))
        {
            dzlog_error("stat error");
            (void)closedir(dp);
            free(dest_path);
            return false;
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
                    (void)closedir(dp);
                    free(dest_path);
                    return false;
                }
                files_found++;
                cJSON_Delete(cjson_config);
            }
            else
            {
                (void)closedir(dp);
                free(dest_path);
                dzlog_error("Failed to load or parse JSON from %s", dest_path);
                return false;
            }
        }
        free(dest_path);
    }
    (void)closedir(dp);

    if (files_found == 0)
    {
        dzlog_error("No files in the target directory");
        free(dest_path);
        return false;
    }

    return true;
}

static bool read_file_to_memory(const char *target_file, cJSON **cjson_config)
{
    if(NULL == target_file || NULL == cjson_config)
    {
        dzlog_info("target_file or cjson_config is NULL");
    }
    FILE* fp = fopen(target_file,"r");
    if(NULL == fp)
    {
        dzlog_info("open file error ");
        return false;
    }

    (void)fseek(fp,0,SEEK_END);
    int file_len = ftell(fp);
    (void)fseek(fp,0,SEEK_SET);


    if(file_len <= 0)
    {
        (void)fclose(fp);
        dzlog_info("file len is 0");
        return false;
    }

    char * readbuf = (char *)malloc(file_len + 1);
    if(NULL == readbuf)
    {
        dzlog_info("malloc error");
        (void)fclose(fp);
        return false;
    }

    size_t bytes_read = fread(readbuf, 1, file_len, fp);
    (void)fclose(fp);

    if(bytes_read != file_len)
    {
        dzlog_info("Unable to read full file");
        free(readbuf);
        return false;
    }

    readbuf[file_len] = '\0';

    *cjson_config = cJSON_Parse(readbuf);
    if(*cjson_config == NULL)
    {
        dzlog_info("Failed to parse JSON");
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
        dzlog_info("config_name or cjson_config or path is NULL");
        return false;
    }
    int len = strlen(config_name) + strlen(path) + 1;
    char *json_string = cJSON_Print(cjson_config);
    char *target_file_path= (char *)malloc(len);
    (void)snprintf(target_file_path, len, "%s%s", path, config_name);

    FILE *target_file = fopen(target_file_path , "w");
    if(NULL == target_file)
    {
        dzlog_info("Failed to open the target file");
        free(json_string);
        return false;
    }
    free(target_file_path);
    
    if(NULL != json_string)
    {
        (void)fwrite(json_string, sizeof(char), strlen(json_string), target_file);

        (void)fflush(target_file);
        int fd = fileno(target_file);
        if(fd != -1)
        {
            if(fsync(fd) != 0)
            {
                dzlog_info("fsync failed");
                (void)fclose(target_file);
                free(json_string);
                return false;
            }
        }
        else
        {
            dzlog_info("Failed to get file descriptor");
            (void)fclose(target_file);
            free(json_string);
            return false;
        }
        (void)fclose(target_file);
        free(json_string);
        return true;
    }else
    {
        dzlog_info("Failed to serialize cJSON object to string.");
        (void)fclose(target_file);
        return false;
    }
}

static void printf_config(config_manage_t * table)
{
    if(NULL == table)
    {
        dzlog_info("table is NULL");
        return;
    }
    config_manage_t *s = NULL;
    char *json_str = NULL;
    dzlog_info("\n"
           "----------------------------------------HASH TABLE-------------------------------------");
    for (s = table; s != NULL; s = s->hh.next) {
        dzlog_info("config_name: %s", s->name);
        dzlog_info("path: %s", s->path);
        json_str = cJSON_Print(s->value);
        dzlog_info("value:\n%s", json_str);
        free(json_str);
    }
    dzlog_info("----------------------------------------HASH TABLE-------------------------------------\n");
    
}

static config_manage_t *find_config_item(config_manage_t *dorc, const char *config_name)
{
    if(NULL == dorc || NULL == config_name)
    {
        dzlog_info("dorc or config_name is NULL");
        return NULL;
    }
    config_manage_t *s;
    HASH_FIND_STR(dorc, config_name, s);
    return s;
}

void clear_hash_table(config_manage_t *table)
{
    if(NULL == table)
    {
        dzlog_info("table is NULL");
        return;
    }
    config_manage_t *current_user;
    config_manage_t *tmp;
    HASH_ITER(hh, table, current_user, tmp)
    {
        free(current_user->name);
        free(current_user->path);
        cJSON_Delete(current_user->value);

        // 清除所有用户
        delete_all_owners(current_user);

        //销毁锁
        (void)pthread_rwlock_destroy(&current_user->value_lock);
        (void)pthread_rwlock_destroy(&current_user->owners_lock);
        //清除表单
        HASH_DEL(table, current_user);  /* delete it (users advances to next) */
        
        free(current_user);             /* free it */
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
        dzlog_error("name or cb is NULL");
        return false;
    }
    dzlog_debug("attach %s", name);
    char config_name [100];
    (void)snprintf(config_name, sizeof(config_name), "%s%s", name, ".json");

    config_manage_t *s = NULL;
    HASH_FIND_STR(g_config_manage_t, config_name, s);
    if(s == NULL)
    {
        dzlog_error("file %s not exist, can't attach", config_name);
        return false;
    }
    else
    {
        dzlog_debug("file %s exist, attaching", config_name);
        cb_list_t *new_owner = (cb_list_t *)malloc(sizeof(cb_list_t));
        if(NULL == new_owner)
        {
            dzlog_error("new_owner malloc failed");
            return false;
        }
        new_owner->cb = cb;
        new_owner->next = NULL;
        new_owner->prev = NULL;

        //加写锁
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
        //解写锁
        (void)pthread_rwlock_unlock(&s->owners_lock);
        return true;
    }
}

static cb_list_t *find_owner(config_manage_t *s, on_config_change cb)
{
    if(NULL == s || NULL == cb)
    {
        dzlog_error("s or cb is NULL");
        return NULL;
    }
    (void)pthread_rwlock_rdlock(&s->owners_lock);
    cb_list_t *head = s->owners;
    while(head != NULL)
    {
        if(head->cb == cb)
        {
            (void)pthread_rwlock_unlock(&s->owners_lock);
            return head;
        }
        head = head->next;
    }
    (void)pthread_rwlock_unlock(&s->owners_lock);
    return NULL;
}


bool config_detach(const char *name, on_config_change cb)
{
    if(NULL == name || NULL == cb)
    {
        dzlog_error("name or cb is NULL");
        return false;
    }
    char config_name [100];
    (void)snprintf(config_name, sizeof(config_name), "%s%s", name, ".json");
    config_manage_t *s = NULL;
    HASH_FIND_STR(g_config_manage_t, config_name, s);
    if(s == NULL)
    {
        dzlog_info("file %s not exist, can't detach", config_name);
        return  false;
    }
    else
    {
        cb_list_t *finded_owner = find_owner(s,cb);
        if (finded_owner == NULL)
        {
            dzlog_info("cb %p does not exist in file %s", cb, config_name);
            return false;
        }
        //加写锁
        (void)pthread_rwlock_wrlock(&s->owners_lock);
        if(finded_owner == s->owners)// 处理头节点
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
        else if(finded_owner->prev != NULL)// 处理中间节点
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
            //解写锁
            free(finded_owner);
            (void)pthread_rwlock_unlock(&s->owners_lock);
            dzlog_info("Error: Unknown node type");
            return false;
        }
    }
}

static bool notify_owner(const char *config, cJSON *old_value, cJSON *new_value)
{
    if(NULL == config || NULL == old_value || NULL == new_value)
    {
        dzlog_error("config or old_value or new_value is NULL");
        return false;
    }
    config_manage_t *s = NULL;
    HASH_FIND_STR(g_config_manage_t, config, s);
    if(s == NULL)
    {
        dzlog_info("file %s not exist, can't detach", config);
        return  false;
    }
    else
    {
        //加读锁
        (void)pthread_rwlock_rdlock(&s->owners_lock);
        cb_list_t *head = s->owners;
        while(head != NULL)
        {
            head->cb(old_value, new_value);
            head = head->next;
        }
        //解读锁
        (void)pthread_rwlock_unlock(&s->owners_lock);
        return true;
    }
    
}

//删除所有节点
static void delete_all_owners(config_manage_t *s)
{
    if(s == NULL)
    {
        dzlog_info("Error: s is NULL");
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
        dzlog_info("Error: str is NULL");
        return NULL;
    }
    const char *start = strchr(str, '_'); 
    if (start == NULL)
    {
        dzlog_info("Error: No delimiter found.");
        return NULL;
    }

    int len = strlen(start + 1); // 获取下划线后字符串的长度
    char *ret2 = (char *)malloc(len + 1); // 分配足够的内存
    if (ret2 == NULL)
    {
        dzlog_info("Error: Could not allocate memory.");
        return NULL;
    }

    // 复制下划线后的字符串到新分配的内存中
    strncpy(ret2, start + 1, len);
    ret2[len] = '\0'; 

    return ret2;
}

