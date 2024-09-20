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






file_struct_t *ALL_CONFIG_FILE = NULL;
file_struct_t *ALL_DEFAULT_FILE = NULL;




void add_config_name(file_struct_t **table, const char *config_name, const cJSON *cjson_config);
static file_struct_t *find_config_name(file_struct_t *dorc, const char *config_name);
static bool file_to_json(const char *target_file, cJSON **cjson_config);
static cJSON *json_to_file(const char *config_name, const cJSON *cjson_config,char *PATH);
// void notify(file_struct_t **table, const char *config);
bool config_init(char *PATH, file_struct_t **table);
char *split(const char *str);
void delete_all_owners(file_struct_t *s);

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
        // 加写锁
        pthread_rwlock_wrlock(&s->valueLock);
        if(cJSON_Compare(s->value, config, true))
        {
            pthread_rwlock_unlock(&s->valueLock);
            printf("Same, no need to modify.\n");
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
        // printf("------------------------------------%ld----------------------------------------\n",pthread_self());
        // printf("s->value : %p\n",s->value);
        // printf("config : %p\n",config);
        // printf("------------------------------------%ld----------------------------------------\n",pthread_self());
        // 加写锁
        pthread_rwlock_wrlock(&s->valueLock);
        if(cJSON_Compare(s->value, config, true))
        {
            pthread_rwlock_unlock(&s->valueLock);
            printf("Same default, no need to modify.\n");
            return false;
        }
        else
        {

            cJSON_Delete(s->value);
            s->value = cJSON_Duplicate(config, 1);
            json_to_file(s->key, s->value,DEFAULT_CONFIG_PATH);
            // 解写锁
            pthread_rwlock_unlock(&s->valueLock);

            char *buffer = split(s->key);
            if(NULL == buffer)
            {
                printf("split error\n");
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
        // for(int i =0;i<2;i++)
        // {
        //     s->owners[i] = NULL;
        // }
        s->owners = NULL;

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
    if(dp == NULL)
    {
        perror("opendir");
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
    observer_t *elt, *tmp2;

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
    printf("callback from owner_num %d\n", num);
}


bool attach(file_struct_t **table, const char *config, int num, void (*callback)(int num))
{
    file_struct_t *s = NULL;
    HASH_FIND_STR(*table, config, s);
    if(s == NULL)
    {
        printf("file %s not exist, can't attach\n", config);
        return false;
    }
    else
    {
        printf("file %s exist, attaching\n", config);
        observer_t *new_owner = (observer_t *)malloc(sizeof(observer_t));
        if(NULL == new_owner)
        {
            //解写锁
            pthread_rwlock_unlock(&s->ownersLock);
            printf("new_owner malloc failed\n");
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
        printf("file %s exist, and owner_num %d exist\n", config, num);
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
    printf("owner_num %d does not exist in file %s\n", owner_num, s->key);
    return NULL;
}


bool detach(file_struct_t **table, const char *config, int num)
{
    file_struct_t *s = NULL;
    HASH_FIND_STR(*table, config, s);
    if(s == NULL)
    {
        printf("file %s not exist, can't detach\n", config);
        return  false;
    }
    else
    {
        observer_t *finded_owner = find_owner(s,num);
        if (finded_owner == NULL)
        {
            printf("owner_num %d does not exist in file %s\n", num, config);
            return false;
        }
        printf("file %s exist, detaching\n", config);
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
            printf("owner_num %d is the first owner of file %s\n", num, config);
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
            printf("owner_num %d is the middle owner of file %s\n", num, config);
            return true;
        }
        else if (finded_owner->next == NULL && finded_owner->prev != NULL)
        {
            finded_owner->prev->next = NULL;
            free(finded_owner);
            pthread_rwlock_unlock(&s->ownersLock);
            printf("owner_num %d is the last owner of file %s\n", num, config);
            return true;
            
        }
        else
        {
            //解写锁
            free(finded_owner);
            pthread_rwlock_unlock(&s->ownersLock);
            printf("Error: Unknown node type\n");
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
        printf("file %s not exist, can't detach\n", config);
        return  false;
    }
    else
    {
        printf("file %s exist, notifying\n", config);
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
void delete_all_owners(file_struct_t *s) {
    pthread_rwlock_wrlock(&s->ownersLock);
    observer_t *head = s->owners;
    while (head != NULL) {
        observer_t *temp = head;
        head = head->next;
        free(temp);
    }
    s->owners = NULL;
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



/**
 * ----------------------------------------------------------------------------------------------------------------
 * 下面是自定义的链表基础操作
 * ----------------------------------------------------------------------------------------------------------------
 */


/**
 * @brief: 遍历链表 找到想要的即返回
 * 
 * @param {observer_t} *head
 * @param {int} num 想要的标识符
 */
static bool list_traverse(observer_t *head, int num)
{
    observer_t *tmp = NULL;
    if(NULL != head)
    {
        head = head->next;
        if(num == head->owner_num)
        {
            tmp = head;
            return true;
        }
    }
}