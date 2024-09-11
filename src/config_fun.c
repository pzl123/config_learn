#include "config_manage.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>

#include "cJSON/cJSON.h"
#include "cJSON/cJSONx.h"





file_struct_t *ALL_CONFIG_FILE = NULL;
file_struct_t *ALL_DEFAULT_FILE = NULL;


void add_config_name(file_struct_t **table, const char *config_name, const cJSON *cjson_config);
static file_struct_t *find_config_name(file_struct_t *dorc, const char *config_name);
static bool file_to_json(const char *target_file, cJSON **cjson_config);
static cJSON *json_to_file(const char *config_name, const cJSON *cjson_config,char *PATH);


bool set_config(const char *name, const cJSON *config)
{   
    file_struct_t *s =find_config_name(ALL_CONFIG_FILE, name);  
    if (NULL == s) 
    {
        printf("no such config %s in path %s",name,CONFIG_PATH);
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
            s->value = cJSON_Duplicate(config, 1);
            json_to_file(s->key, s->value,CONFIG_PATH);
            return true;
        }
    }
}

bool get_config(const char *name, cJSON **config)
{   
    file_struct_t *s = find_config_name(ALL_CONFIG_FILE, name);  
    if (NULL == s) 
    {
        printf("no such config %s in path %s",name,DEFAULT_CONFIG_PATH);
        return false;
    } 
    else
    {
        *config = cJSON_Duplicate(s->value, 1);
        return true;
    }
}

bool set_default(const char *name, const cJSON *config)
{   
    file_struct_t *s = find_config_name(ALL_DEFAULT_FILE, name);  
    if (NULL == s) 
    {
        printf("no such config %s in path %s",name,DEFAULT_CONFIG_PATH);
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
            s->value = cJSON_Duplicate(config, 1);
            json_to_file(s->key, s->value,DEFAULT_CONFIG_PATH);
            strtok(s->key, "_");
            char *prefix = strtok(NULL, "_");
            set_config(prefix, s->value);
            return true;
        }
    }
}

bool get_default(const char *name, cJSON **config)
{   
    file_struct_t *s = find_config_name(ALL_DEFAULT_FILE, name);  
    if (NULL == s) 
    {
        printf("no such config %s in path %s",name,DEFAULT_CONFIG_PATH);
        return false;
    } 
    else
    {
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
            perror("malloc error");
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
            perror("Failed to duplicate cJSON object");
        }

        HASH_ADD_STR(*table, key, s);
    }
    else
    {
        printf("file already exits in hash table\n ");
    }

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
    }
    printf("----------------------------------------HASH TABLE-------------------------------------\n\n");
    free(json_str);
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
    HASH_ITER(hh, table, current_user, tmp) {
        HASH_DEL(table, current_user);  /* delete it (users advances to next) */
        cJSON_Delete(current_user->value);
        free(current_user);             /* free it */
    }

}
