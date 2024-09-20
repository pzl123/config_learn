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





file_struct_t *ALL_CONFIG_FILE = NULL;
file_struct_t *ALL_DEFAULT_FILE = NULL;
extern zlog_category_t *c;



static bool add_config_name(file_struct_t **table, const char *config_name, const cJSON *cjson_config);
static file_struct_t *find_config_name(file_struct_t *dorc, const char *config_name);
static bool file_to_json(const char *target_file, cJSON **cjson_config);
static bool json_to_file(const char *config_name, const cJSON *cjson_config, char *PATH);
// void notify(file_struct_t **table, const char *config);
static bool config_init(char *PATH, file_struct_t **table);
static char *split(const char *str);
static observer_t *find_owner(file_struct_t *s, int owner_num);

/**
 * ----------------------------------------------------------------------------------------------------------------
 * 下面是配置文件模块的函数实现
 * ----------------------------------------------------------------------------------------------------------------
 */

bool set_config(const char *name, const cJSON *config)
{
    file_struct_t *s =find_config_name(ALL_CONFIG_FILE, name);
    if (NULL == s)
    {
        zlog_warn(c,"no such config %s in path %s",name,CONFIG_PATH);
        return false;
    }
    else
    {
        // 加写锁
        pthread_rwlock_wrlock(&s->valueLock);
        if(cJSON_Compare(s->value, config, true))
        {
            pthread_rwlock_unlock(&s->valueLock);
            zlog_info(c, "Same, no need to modify.");
            return false;
        }
        else
        {
            cJSON_Delete(s->value);
            s->value = cJSON_Duplicate(config, 1);
            json_to_file(s->key, s->value,CONFIG_PATH);
            // 解写锁
            pthread_rwlock_unlock(&s->valueLock);
            notify(&ALL_CONFIG_FILE, name); //通知所有者
            return true;
        }
    }
    
}

bool get_config(const char *name, cJSON **config)
{
    file_struct_t *s = find_config_name(ALL_CONFIG_FILE, name);
    if (NULL == s)
    {
        zlog_warn(c, "no such default config %s in path %s",name,DEFAULT_CONFIG_PATH);
        return false;
    }
    else
    {   
        // 加读锁
        pthread_rwlock_rdlock(&s->valueLock);
        cJSON_Delete(*config);
        *config = cJSON_Duplicate(s->value, 1);
        // 解读锁
        pthread_rwlock_unlock(&s->valueLock);
        return true;
    }
}

bool set_default(const char *name, const cJSON *config)
{
    file_struct_t *s = find_config_name(ALL_DEFAULT_FILE, name);
    if (NULL == s)
    {
        zlog_warn(c,"no such default config %s in path %s",name,DEFAULT_CONFIG_PATH);
        return false;
    }
    else
    {   
         // 加写锁
        pthread_rwlock_wrlock(&s->valueLock);
        if(cJSON_Compare(s->value, config, true))
        {
            pthread_rwlock_unlock(&s->valueLock);
            zlog_info(c, "Same default, no need to modify.");
            return false;
        }
        else
        {
            cJSON_Delete(s->value);
            s->value = cJSON_Duplicate(config, 1);
            if(NULL == s->value)
            {
                pthread_rwlock_unlock(&s->valueLock);
                return false;
            }
            json_to_file(s->key, s->value,DEFAULT_CONFIG_PATH);
            // 解写锁
            pthread_rwlock_unlock(&s->valueLock);

            char *buffer = split(s->key);
            if(NULL == buffer)
            {
                zlog_error(c,"split error");
                return false;
            }
            else
            {
                set_config(buffer, config);
            }
            notify(&ALL_DEFAULT_FILE, name); //通知所有者

            free(buffer);
            return true;
        }
    }
}

bool get_default(const char *name, cJSON **config)
{
    file_struct_t *s = find_config_name(ALL_DEFAULT_FILE, name);
    if (NULL == s)
    {
        zlog_warn(c,"no such default config %s in path %s",name,DEFAULT_CONFIG_PATH);
        return false;
    }
    else
    {
        // 加读锁
        pthread_rwlock_rdlock(&s->valueLock);
        cJSON_Delete(*config);
        *config = cJSON_Duplicate(s->value, 1);
        // 解读锁
        pthread_rwlock_unlock(&s->valueLock);
        return true;
    }
}

static bool add_config_name(file_struct_t **table, const char *config_name, const cJSON *cjson_config) 
{
    file_struct_t *s = NULL;
    HASH_FIND_STR(*table,config_name,s);
    if (NULL == s)
    {
        s = (file_struct_t*)malloc(sizeof(file_struct_t));
        if (NULL == s)
        {
            zlog_error(c,"malloc error");
            return false;
        }
        strncpy(s->key,config_name,strlen(config_name));
        s->key[strlen(config_name)] = '\0';
        char dest_path[MAX_PATH_LEN] = {0};
        snprintf(dest_path, sizeof(dest_path), "%s%s", CONFIG_PATH, config_name);
        strncpy(s->path,dest_path,strlen(dest_path));
        s->path[strlen(dest_path)] = '\0';
        s->value = cJSON_Duplicate(cjson_config, 1);
        if (s->value == NULL)
        {
            zlog_error(c, "Failed to duplicate cJSON object");
            free(s);
            return false;
        }

        // 初始化配置的观察者结构体
        s->owners = NULL;

        HASH_ADD_STR(*table, key, s);

        // 初始化读写锁
        if (pthread_rwlock_init(&(s->valueLock), NULL) != 0)
        {
            zlog_error(c, "Failed to initialize valueLock");
            free(s);
            return false;
        }

        if (pthread_rwlock_init(&(s->ownersLock), NULL) != 0)
        {
            zlog_error(c, "Failed to initialize ownersLock");
            pthread_rwlock_destroy(&(s->valueLock)); // 清理 valueLock
            free(s);
            return false;
        }
        return true;
    }
    else
    {
        zlog_info(c,"file already exits in hash table");
        return true;
    }

}

bool all_config_init(void)
{
    int ret = 0;
    ret = config_init(CONFIG_PATH, &ALL_CONFIG_FILE);
    if (ret == 0)
    {
        zlog_fatal(c,"Failed to initialize config");
        return false;
    }
    
    ret = config_init(DEFAULT_CONFIG_PATH, &ALL_DEFAULT_FILE);
    if (ret == 0)
    {
        zlog_fatal(c,"Failed to initialize default config");
        return false;
    }
    return true;
}

static bool config_init(char *PATH, file_struct_t **table)
{
    struct dirent *file  = NULL;
    DIR *dp = opendir(PATH);
    if(dp == NULL)
    {
        zlog_error(c,"opendir");
        return false;
    }
    int files_found = 0;

    while((file = readdir(dp)) != NULL)
    {
        if(strncmp(file->d_name, ".", 1) == 0)
            continue;
        char dest_path[MAX_PATH_LEN] = {0};
        (void)snprintf(dest_path, sizeof(dest_path), "%s%s", PATH, file->d_name);

        struct stat buf;
        memset(&buf, 0, sizeof(buf));
        if (stat(dest_path, &buf) != 0)
        {
            zlog_error(c, "stat error");
            return false;
        }

        if (S_ISREG(buf.st_mode))
        {
            cJSON *cjson_config = NULL;
            if (file_to_json(dest_path, &cjson_config))
            {
                if(add_config_name(table,file->d_name, cjson_config))
                {
                    zlog_info(c,"Successfully loaded JSON from %s", dest_path);
                }
                files_found++;
                cJSON_Delete(cjson_config);
            }
            else
            {
                zlog_error(c, "Failed to load or parse JSON from %s", dest_path);
            }
        }
    }
    closedir(dp);

    if (files_found == 0)
    {
        zlog_warn(c, "No files in the target directory");
        return false;
    }

    return true;
}

static bool file_to_json(const char *target_file, cJSON **cjson_config)
{
    FILE* fp = fopen(target_file,"r");
    if(fp == NULL)
    {
        zlog_error(c,"open file error ");
        return false;
    }

    (void)fseek(fp,0,SEEK_END);
    int file_len = ftell(fp);
    (void)fseek(fp,0,SEEK_SET);


    if(file_len <= 0)
    {
        fclose(fp);
        return false;
    }

    char * readbuf = (char *)malloc(file_len + 1);
    if(NULL == readbuf)
    {
        zlog_error(c, "malloc error");
        fclose(fp);
        return false;
    }

    size_t bytes_read = fread(readbuf, 1, file_len, fp);
    fclose(fp);

    if(bytes_read != file_len)
    {
        zlog_error(c,"Unable to read full file");
        free(readbuf);
        return false;
    }

    readbuf[file_len] = '\0';

    *cjson_config = cJSON_Parse(readbuf);
    if(*cjson_config == NULL)
    {
        zlog_error(c, "Failed to parse JSON");
        free(readbuf);
        return false;
    }

    free(readbuf);
    return true;
}

static bool json_to_file(const char *config_name, const cJSON *cjson_config, char *PATH)
{
    char *json_string = cJSON_Print(cjson_config);
    char target_file_path[MAX_PATH_LEN] = {0};
    snprintf(target_file_path, sizeof(target_file_path), "%s%s", PATH, config_name);

    FILE *target_file = fopen(target_file_path , "w");
    if(NULL == target_file)
    {
        zlog_error(c, "Failed to open the target file");
        free(json_string);
        return false;
    }

    if(json_string != NULL)
    {
        fwrite(json_string, sizeof(char), strlen(json_string), target_file);

        fflush(target_file);
        int fd = fileno(target_file);
        if(fd != -1)
        {
            if(fsync(fd) != 0)
            {
                zlog_error(c,"fsync failed");
                fclose(target_file);
                free(json_string);
                return false;
            }
        }
        else
        {
            zlog_error(c,"Failed to get file descriptor");
            fclose(target_file);
            free(json_string);
            return false;
        }
        fclose(target_file);
        free(json_string);
        return true;
    }else
    {
        zlog_error(c,"Failed to serialize cJSON object to string.");
        fclose(target_file);
        return false;
    }
}

void print_hash_table(file_struct_t * table)
{
    file_struct_t *s = NULL;
    char *json_str = NULL;
    zlog_info(c,"\n"
           "----------------------------------------HASH TABLE-------------------------------------");
    for (s = table; s != NULL; s = s->hh.next) {
        printf("config_name: %s", s->key);
        printf("path: %s", s->path);
        json_str = cJSON_Print(s->value);
        printf("value:\n%s", json_str);
        free(json_str);
    }
    zlog_info(c, "----------------------------------------HASH TABLE-------------------------------------\n");
    
}

static file_struct_t *find_config_name(file_struct_t *dorc, const char *config_name)
{
    file_struct_t *s;
    HASH_FIND_STR(dorc, config_name, s);
    return s;
}

void clear_hash_table(file_struct_t *table)
{
    file_struct_t *current_user;
    file_struct_t *tmp;
    HASH_ITER(hh, table, current_user, tmp)
    {
        // 清除所有用户
        delete_all_owners(current_user);

        //销毁锁
        (void)pthread_rwlock_destroy(&current_user->valueLock);
        (void)pthread_rwlock_destroy(&current_user->ownersLock);
        //清除表单
        HASH_DEL(table, current_user);  /* delete it (users advances to next) */
        cJSON_Delete(current_user->value);
        free(current_user);             /* free it */
    }
}



void clear_all_hash_table(void)
{
    clear_hash_table(ALL_CONFIG_FILE);
    clear_hash_table(ALL_DEFAULT_FILE);
}

/**
 * ----------------------------------------------------------------------------------------------------------------
 * 下面是通知机制
 * ----------------------------------------------------------------------------------------------------------------
 */


void callback(int num)
{
    zlog_info(c,"callback from owner_num %d", num);
}


bool attach(file_struct_t **table, const char *config, int num, void (*callback)(int num))
{
    zlog_info(c, "attach %s", config);
    file_struct_t *s = NULL;
    HASH_FIND_STR(*table, config, s);
    if(s == NULL)
    {
        zlog_warn(c,"file %s not exist, can't attach", config);
        return false;
    }
    else
    {
        zlog_info(c,"file %s exist, attaching", config);
        observer_t *new_owner = (observer_t *)malloc(sizeof(observer_t));
        if(NULL == new_owner)
        {
            //解写锁
            pthread_rwlock_unlock(&s->ownersLock);
            zlog_error(c,"new_owner malloc failed");
            return false;
        }
        new_owner->callback = callback;
        new_owner->owner_num = num;
        new_owner->next = NULL;
        new_owner->prev = NULL;

        //加写锁
        pthread_rwlock_wrlock(&s->ownersLock);
        if( NULL == s->owners)
        {   
            s->owners = new_owner;
        }
        else
        {
            observer_t *head = s->owners;
            while(head->next != NULL)
            {
                head = head->next;
            }
            head->next = new_owner;
            new_owner->prev = head;
        }
        //解写锁
        pthread_rwlock_unlock(&s->ownersLock);
        zlog_info(c,"file %s exist, and owner_num %d exist", config, num);
        return true;
    }
}

static observer_t *find_owner(file_struct_t *s, int owner_num)
{
    pthread_rwlock_rdlock(&s->ownersLock);
    observer_t *head = s->owners;
    while(head != NULL)
    {
        if(head->owner_num == owner_num)
        {
            pthread_rwlock_unlock(&s->ownersLock);
            return head;
        }
        head = head->next;
    }
    pthread_rwlock_unlock(&s->ownersLock);
    zlog_info(c,"owner_num %d does not exist in file %s", owner_num, s->key);
    return NULL;
}


bool detach(file_struct_t **table, const char *config, int num)
{
    file_struct_t *s = NULL;
    HASH_FIND_STR(*table, config, s);
    if(s == NULL)
    {
        zlog_warn(c,"file %s not exist, can't detach", config);
        return  false;
    }
    else
    {
        observer_t *finded_owner = find_owner(s,num);
        if (finded_owner == NULL)
        {
            zlog_warn(c,"owner_num %d does not exist in file %s", num, config);
            return false;
        }
        zlog_info(c,"file %s exist, detaching", config);
        //加写锁
        pthread_rwlock_wrlock(&s->ownersLock);
        if(finded_owner == s->owners)// 处理头节点
        { 
            s->owners = finded_owner->next;
            if(s->owners != NULL)
            {
                s->owners->prev = NULL;
            }
            free(finded_owner);
            pthread_rwlock_unlock(&s->ownersLock);
            zlog_info(c, "owner_num %d is the first owner of file %s", num, config);
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
            pthread_rwlock_unlock(&s->ownersLock);
            zlog_info(c,"owner_num %d is the middle owner of file %s", num, config);
            return true;
        }
        else if (finded_owner->next == NULL && finded_owner->prev != NULL)
        {
            finded_owner->prev->next = NULL;
            free(finded_owner);
            pthread_rwlock_unlock(&s->ownersLock);
            zlog_info(c,"owner_num %d is the last owner of file %s", num, config);
            return true;
            
        }
        else
        {
            //解写锁
            free(finded_owner);
            pthread_rwlock_unlock(&s->ownersLock);
            zlog_error(c,"Error: Unknown node type");
            return false;
        }
    }
}

bool notify(file_struct_t **table, const char *config)
{
    file_struct_t *s = NULL;
    HASH_FIND_STR(*table, config, s);
    if(s == NULL)
    {
        zlog_warn(c,"file %s not exist, can't detach", config);
        return  false;
    }
    else
    {
        zlog_info(c,"file %s exist, notifying", config);
        //加读锁
        pthread_rwlock_rdlock(&s->ownersLock);
        observer_t *head = s->owners;
        while(head != NULL)
        {
            head->callback(head->owner_num);
            head = head->next;
        }
        //解读锁
        pthread_rwlock_unlock(&s->ownersLock);
        return true;
    }
    
}

//删除所有节点
void delete_all_owners(file_struct_t *s)
{
    pthread_rwlock_wrlock(&s->ownersLock);
    observer_t *head = s->owners;
    while (head != NULL)
    {
        observer_t *temp = head;
        head = head->next;
        free(temp);
    }
    s->owners = NULL;
    pthread_rwlock_unlock(&s->ownersLock);
}


static char *split(const char *str)
{
    const char *start = strchr(str, '_'); 
    if (start == NULL)
    {
        zlog_error(c,"Error: No delimiter found.");
        return NULL;
    }

    int len = strlen(start + 1); // 获取下划线后字符串的长度
    char *ret2 = (char *)malloc(len + 1); // 分配足够的内存
    if (ret2 == NULL)
    {
        zlog_error(c,"Error: Could not allocate memory.");
        return NULL;
    }

    // 复制下划线后的字符串到新分配的内存中
    strncpy(ret2, start + 1, len);
    ret2[len] = '\0'; 

    return ret2;
}

