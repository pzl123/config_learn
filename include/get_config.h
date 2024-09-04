#ifndef GET_CONFIG_H
#define GET_CONFIG_H

#define CONFIG_FILE_PATH "./../config/config.json"
#define DEFAULT_CONFIG_FILE_PATH "./../config/default_config.json"

void get_config(const char * keyname,const char * type);
void Determine_the_type(void * b);

#endif