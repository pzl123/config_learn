#include "get_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>

#include "cJSON/cJSON.h"
#include "cJSON/cJSONx.h"

struct file_struct *ALL_CONFIG_FILE = NULL;



/**
 * @brief Adds a default configuration file if it does not already exist.
 *
 * This function checks if a file with the specified name exists in the
 * configuration files. If it does not exist, it adds the filename and 
 * associated path to the hash table, creates the target file path, 
 * serializes the provided cJSON object into a JSON string, and writes 
 * that string to the file.
 * 
 * @param filename The name of the configuration file to add.
 * @param path The path where the configuration file should be created.
 * @param cjson_config The cJSON object containing the configuration data.
 */
void add_default_config_file(char *filename, const char *path, cJSON *cjson_config)
{
    struct file_struct *s = find_filename(filename);
    if (NULL == s) {
        printf("no such file in config files, add this file %s\n",filename);
        add_filename(filename, path, cjson_config);

        char target_file_path[MAX_PATH_LEN] = {0}; 
        snprintf(target_file_path, sizeof(target_file_path), "%s%s", path, filename);
        
        FILE *target_file = fopen(target_file_path , "w");
        if (NULL == target_file) 
        {
            perror("Failed to open the target file");
        }

        char *json_string = cJSON_Print(cjson_config); 
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
        fclose(target_file); 
    }
    else
    {
        printf("this file already exists. Modification is not allowed\n");
    }
}

void set_config_file(char *filename, const char *path, cJSON *cjson_config)
{
    struct file_struct *s = find_filename(filename);
    if (NULL == s) {
        printf("no such file in config files, add this file %s\n",filename);
        add_filename(filename, path, cjson_config);

        char target_file_path[MAX_PATH_LEN] = {0}; 
        snprintf(target_file_path, sizeof(target_file_path), "%s%s", path, filename);
        
        FILE *target_file = fopen(target_file_path , "w");
        if (NULL == target_file) 
        {
            perror("Failed to open the target file");
        }

        char *json_string = cJSON_Print(cjson_config); 
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
        fclose(target_file); 
    }
    else
    {
        printf("This file already exists. Modifying this configuration file.\n");
        s->vp.value = cjson_config;
    }
}




/*
*@brief Adds a filename to the configuration file list with an associated path and JSON configuration.
*
*@param filename The name of the file to add.
*@param path The path where the file is located.
*@param cjson_config A pointer to the JSON configuration associated with the file.
*
*@return None. This function modifies the global configuration file list.
*/
void add_filename(char *filename, const char *path, cJSON *cjson_config) 
{
    struct file_struct *s = NULL;

    HASH_FIND_STR(ALL_CONFIG_FILE,filename,s);
    if (NULL == s) 
    {
        s = (struct file_struct*)malloc(sizeof(struct file_struct));
        if (NULL == s)
        {
            perror("malloc error");
        }
        strncpy(s->filename,filename,strlen(filename));
        s->filename[strlen(filename)] = '\0';
        HASH_ADD_STR(ALL_CONFIG_FILE, filename, s);
    }
    char dest_path[MAX_PATH_LEN] = {0};
    snprintf(dest_path, sizeof(dest_path), "%s%s", path, filename);
    strncpy(s->vp.path,dest_path,sizeof(s->vp.path)-1);
    s->vp.path[sizeof(s->vp.path)-1] = '\0';
    s->vp.value = cjson_config;
}



/*
*@brief Initializes a hash table with configuration files found in a specified directory.
*
*@param path The path to the directory containing the configuration files.
*
*@return Returns the number of configuration files successfully processed 
*        if at least one file was found, 
*        -1 if an error occurred (e.g., directory not found or no files to process).
*/
int config_file_init_to_hashtable(const char *path)
{
    struct dirent *file  = NULL;
    DIR *dp = opendir(path);

    if (dp == NULL) 
    {
        perror("opendir");
        return -1;
    }
    int files_found = 0; 

    while ((file = readdir(dp)) != NULL) 
    {
        if (strncmp(file->d_name, ".", 1) == 0)
            continue;
        char dest_path[MAX_PATH_LEN] = {0};
        snprintf(dest_path, sizeof(dest_path), "%s%s", path, file->d_name);

        struct stat buf;
        memset(&buf, 0, sizeof(buf));    
        int ret = stat(dest_path, &buf);

        if (S_ISREG(buf.st_mode))
        {
            cJSON *cjson_config = NULL;
            cjson_config = file_to_json(dest_path, cjson_config);
            add_filename(file->d_name, path, cjson_config);
            files_found ++;    
        }
    }

    closedir(dp);

    if (files_found == 0) 
    {
        printf("No files in the target directory\n");
        return -1;
    }


    return files_found;
}



/*
*@brief Reads a JSON file and parses its content into a cJSON object.
*
*@param target_file The path to the target JSON file to be read.
*@param cjson_config A pointer to a cJSON object where the parsed data will be stored.
*
*@return A pointer to the populated cJSON object on success, or NULL on failure. 
*        The caller is responsible for freeing the returned cJSON object.
*/
cJSON *file_to_json(const char *target_file,cJSON *cjson_config)
{
    FILE* fp = fopen(target_file,"r");
    if (fp == NULL) 
    {
        perror("open file error ");
        return NULL;
    }

    (void)fseek(fp,0,SEEK_END);
    int file_len = ftell(fp);
    (void)fseek(fp,0,SEEK_SET);

    
    char * readbuf = (char *)malloc(file_len);
    if (readbuf == 0)
    {
        perror("malloc error");
        fclose(fp);
        return NULL;
    }
    readbuf[file_len] = '\0';

    size_t bytes_read = fread(readbuf, 1, file_len+1, fp);
    if (bytes_read == 0) 
    {
        perror("Unable to read file");
        fclose(fp);
        return NULL;
    }
    fclose(fp);

    cjson_config = cJSON_Parse(readbuf);
    if (cjson_config == NULL)
    {
        fprintf(stderr, "Failed to parse JSON\n");
        return NULL;
    }

    free(readbuf);
    return cjson_config;
}


/**
 * @brief Prints the contents of a hash table.
 * 
 * This function iterates through all entries in the hash table, starting from the 
 * head of the list pointed to by `files`. For each entry, it prints the filename 
 * and associated path.
 * 
 * @param files Pointer to the head of the hash table. Each entry is a `file_struct` 
 *              containing a filename and path.
 * 
 * @note This function assumes that `files` is a valid pointer to a hash table and 
 *       that `file_struct` contains members `filename` and `vp.path` which are 
 *       properly initialized.
 */
void print_hash_table(struct file_struct *files) {
    struct file_struct *s = NULL;
    
    printf("ALL HASH TABLE is\n"
           "----------------------------------\n");
    for (s = files; s != NULL; s = s->hh.next) {
        printf("Filename: %s\n", s->filename);
        printf("path: %s\n", s->vp.path);
        char *json_str = cJSON_Print(s->vp.value);
        printf("value is %s\n", json_str);
        printf("\n");
    }
    printf("----------------------------------\n"
           "ALL HASH TABLE end\n");
}


/**
 * @brief Finds a file entry in the hash table by filename.
 * 
 * This function searches for a `file_struct` entry in the hash table using the 
 * specified filename. It utilizes the `HASH_FIND_STR` macro to perform the 
 * lookup. If the entry is found, it returns a pointer to the corresponding 
 * `file_struct`; otherwise, it returns NULL.
 * 
 * @param filename The filename to search for in the hash table.
 * 
 * @return A pointer to the `file_struct` associated with the filename, or NULL 
 *         if no matching entry is found.
 * 
 * @note `ALL_CONFIG_FILE` should be a global hash table where the filenames are 
 *       stored.
 */
struct file_struct *find_filename(const char *filename)
{
    struct file_struct *s;
    HASH_FIND_STR(ALL_CONFIG_FILE, filename, s);
    return s;
}

