#ifndef GET_CONFIG_H
#define GET_CONFIG_H
#include "uthash/uthash.h"
#include "cJSON/cJSON.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define CONFIG_FILE "./../config_file_dir/config/config.json"
#define DEFAULT_CONFIG_FILE "./../config_file_dir/default/default_config.json"
#define CONFIG_PATH "./../config_file_dir/config/"
#define MAX_PATH_LEN 1024


extern struct file_struct *ALL_CONFIG_FILE;


struct value_path
{
    char path[100];
    cJSON * value;
};

struct file_struct
{
    char filename[100];       /* key */
    struct value_path vp;     /* value */
    UT_hash_handle hh;        /* makes this structure hashable */
};



void add_filename(char *filename, const char *path, cJSON *cjson_config) ;


int config_file_init_to_hashtable(const char *path);


void print_hash_table(struct file_struct *files);

struct file_struct *find_filename(const char *filename);

cJSON *file_to_json(const char *target_file,cJSON *cjson_config);

bool set_default_config(const char *config_name, const cJSON* value);

void add_default_config_file(char *filename, const char *path, cJSON *cjson_config);

void set_config_file(char *filename, const char *path, cJSON *cjson_config);

#ifdef __cplusplus
}
#endif

#endif