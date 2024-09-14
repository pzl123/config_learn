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






file_struct_t *ALL_CONFIG_FILE = NULL;
file_struct_t *ALL_DEFAULT_FILE = NULL;




void add_config_name(file_struct_t **table, const char *config_name, const cJSON *cjson_config);
static file_struct_t *find_config_name(file_struct_t *dorc, const char *config_name);
static bool file_to_json(const char *target_file, cJSON **cjson_config);
static cJSON *json_to_file(const char *config_name, const cJSON *cjson_config,char *PATH);
void notify(file_struct_t **table, const char *config);
bool config_init(char *PATH, file_struct_t **table);
char *split(const char *str);

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
        printf("no such config %s in path %s\n",name,CONFIG_PATH);
        return false;
    }
    else
    {
        if(cJSON_Compare(s->value, config, true))
        {
            printf("Same, no need to modify.\n");
            return false;
        }
        else
        {
            // 加写锁
            pthread_rwlock_wrlock(&s->valueLock);
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
        printf("no such default config %s in path %s\n",name,DEFAULT_CONFIG_PATH);
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
        printf("no such default config %s in path %s\n",name,DEFAULT_CONFIG_PATH);
        return false;
    }
    else
    {
        if(cJSON_Compare(s->value, config, true))
        {
            printf("Same default, no need to modify.\n");
            return false;
        }
        else
        {
            cJSON_Delete(s->value);
            s->value = cJSON_Duplicate(config, 1);
            json_to_file(s->key, s->value,DEFAULT_CONFIG_PATH);

            char *buffer = split(s->key);
            if(NULL == buffer)
            {
                printf("split error\n");
                return false;
            }
            else
            {
                set_config(buffer, s->value);
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
        printf("no such default config %s in path %s\n",name,DEFAULT_CONFIG_PATH);
        return false;
    }
    else
    {
        cJSON_Delete(*config);
        *config = cJSON_Duplicate(s->value, 1);
        return true;
    }
}

void add_config_name(file_struct_t **table, const char *config_name, const cJSON *cjson_config) 
{
    file_struct_t *s = NULL;
    HASH_FIND_STR(*table,config_name,s);
    if (NULL == s)
    {
        s = (file_struct_t*)malloc(sizeof(file_struct_t));
        if (NULL == s)
        {
            perror("malloc error\n");
            return ;
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
            perror("Failed to duplicate cJSON object\n");
        }

        // 初始化该表的所有者
        for(int i =0;i<2;i++)
        {
            s->owners[i] = NULL;
        }

        HASH_ADD_STR(*table, key, s);
    }
    else
    {
        printf("file already exits in hash table\n ");
    }

    // 初始化读写锁
    if (pthread_rwlock_init(&(s->valueLock), NULL) != 0) {
        perror("Failed to initialize valueLock");
        free(s);
        return;
    }

    if (pthread_rwlock_init(&(s->ownersLock), NULL) != 0) {
        perror("Failed to initialize ownersLock");
        pthread_rwlock_destroy(&(s->valueLock)); // 清理 valueLock
        free(s);
        return;
    }

}

bool all_config_init(void)
{
    int ret = 0;
    ret = config_init(CONFIG_PATH, &ALL_CONFIG_FILE);
    if (ret == 0)
    {
        printf("Failed to initialize config\n");
        return false;
    }
    
    ret = config_init(DEFAULT_CONFIG_PATH, &ALL_DEFAULT_FILE);
    if (ret == 0)
    {
        printf("Failed to initialize default config\n");
        return false;
    }
    return true;
}

bool config_init(char *PATH, file_struct_t **table)
{
    struct dirent *file  = NULL;
    DIR *dp = opendir(PATH);

    if (dp == NULL)
    {
        perror("opendir");
        return false;
    }
    int files_found = 0;

    while ((file = readdir(dp)) != NULL)
    {
        if (strncmp(file->d_name, ".", 1) == 0)
            continue;
        char dest_path[MAX_PATH_LEN] = {0};
        (void)snprintf(dest_path, sizeof(dest_path), "%s%s", PATH, file->d_name);

        struct stat buf;
        memset(&buf, 0, sizeof(buf));
        if (stat(dest_path, &buf) != 0)
        {
            perror("stat error");
            return false;
        }

        if (S_ISREG(buf.st_mode))
        {
            cJSON *cjson_config = NULL;
            if (file_to_json(dest_path, &cjson_config))
            {
                add_config_name(table,file->d_name, cjson_config);
                files_found++;
                cJSON_Delete(cjson_config);
            }
            else
            {
                fprintf(stderr, "Failed to load or parse JSON from %s\n", dest_path);
            }
        }
    }
    closedir(dp);

    if (files_found == 0)
    {
        printf("No files in the target directory\n");
        return false;
    }

    return true;
}

bool file_to_json(const char *target_file, cJSON **cjson_config)
{
    FILE* fp = fopen(target_file,"r");
    if (fp == NULL)
    {
        perror("open file error ");
        return false;
    }

    (void)fseek(fp,0,SEEK_END);
    int file_len = ftell(fp);
    (void)fseek(fp,0,SEEK_SET);


    if (file_len <= 0)
    {
        fclose(fp);
        return false;
    }

    char * readbuf = (char *)malloc(file_len + 1);
    if (NULL == readbuf)
    {
        perror("malloc error");
        fclose(fp);
        return false;
    }

    size_t bytes_read = fread(readbuf, 1, file_len, fp);
    fclose(fp);

    if (bytes_read != file_len)
    {
        perror("Unable to read full file");
        free(readbuf);
        return false;
    }

    readbuf[file_len] = '\0';

    *cjson_config = cJSON_Parse(readbuf);
    if (*cjson_config == NULL)
    {
        fprintf(stderr, "Failed to parse JSON\n");
        free(readbuf);
        return false;
    }

    free(readbuf);
    return true;
}

cJSON *json_to_file(const char *config_name, const cJSON *cjson_config, char *PATH)
{
    char *json_string = cJSON_Print(cjson_config);
    char target_file_path[MAX_PATH_LEN] = {0};
    snprintf(target_file_path, sizeof(target_file_path), "%s%s", PATH, config_name);

    FILE *target_file = fopen(target_file_path , "w");
    if (NULL == target_file)
    {
        perror("Failed to open the target file");
    }

    if (json_string != NULL) {
        fwrite(json_string, sizeof(char), strlen(json_string), target_file);

        fflush(target_file);
        int fd = fileno(target_file);
        if (fd != -1) {
            if (fsync(fd) != 0) {
                perror("fsync failed");
            }
        } else {
            perror("Failed to get file descriptor");
        }

        free(json_string);
    }else {
        printf("Failed to serialize cJSON object to string.\n");
    }
}

void print_hash_table(file_struct_t * table)
{
    file_struct_t *s = NULL;
    char *json_str = NULL;
    printf("\n"
           "----------------------------------------HASH TABLE-------------------------------------\n");
    for (s = table; s != NULL; s = s->hh.next) {
        printf("config_name: %s\n", s->key);
        printf("path: %s\n", s->path);
        json_str = cJSON_Print(s->value);
        printf("value:\n%s\n", json_str);
        free(json_str);
    }
    printf("----------------------------------------HASH TABLE-------------------------------------\n\n");
    
}

file_struct_t *find_config_name(file_struct_t *dorc, const char *config_name)
{
    file_struct_t *s;
    HASH_FIND_STR(dorc, config_name, s);
    return s;
}

void clear_hash_table(file_struct_t *table)
{
    file_struct_t *current_user;
    file_struct_t *tmp;

    file_struct_t *temp = NULL;
    HASH_ITER(hh, table, current_user, tmp)
    {
        // 清除所有用户
        for(int i = 0; i < 2; i++)
        {
            if (current_user->owners[i] != NULL)
            {
                free(current_user->owners[i]);
            }
        }
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
    printf("callback from owner_num %d\n", num);
}

void attach(file_struct_t **table, const char *config, int num, void (*callback)(int num))
{
    file_struct_t *s = NULL;
    HASH_FIND_STR(*table, config, s);
    if(s == NULL)
    {
        printf("file %s not exist, can't attach\n", config);
        return;
    }
    else
    {
        printf("file %s exist, attaching\n", config);
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

void detach(file_struct_t **table, const char *config, int num)
{
    file_struct_t *s = NULL;
    HASH_FIND_STR(*table, config, s);
    if(s == NULL)
    {
        printf("file %s not exist, can't detach\n", config);
        return;
    }
    else
    {
        printf("file %s exist, detaching\n", config);
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

void notify(file_struct_t **table, const char *config)
{
    file_struct_t *s = NULL;
    HASH_FIND_STR(*table, config, s);
    if(s == NULL)
    {
        printf("file %s not exist, can't notify\n", config);
        return;
    }
    else
    {
        printf("file %s exist, notify\n", config);
        for(int i = 0; i < 2; i++)
        {
            if( NULL != (*table)->owners[i])
            {
                (*table)->owners[i]->callback((*table)->owners[i]->owner_num);
            }
        }
    }
}




char *split(const char *str)
{
    const char *start = strchr(str, '_'); 
    if (start == NULL)
    {
        printf("Error: No delimiter found.\n");
        return NULL;
    }

    int len = strlen(start + 1); // 获取下划线后字符串的长度
    char *ret2 = (char *)malloc(len + 1); // 分配足够的内存
    if (ret2 == NULL)
    {
        printf("Error: Could not allocate memory.\n");
        return NULL;
    }

    // 复制下划线后的字符串到新分配的内存中
    strncpy(ret2, start + 1, len);
    ret2[len] = '\0'; 

    return ret2;
}