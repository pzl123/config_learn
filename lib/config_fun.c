#include "get_config.h"
#include <stdio.h>
#include <cJSON/cJSON.h>
#include <cJSON/cJSONx.h>
#include <stdlib.h>
#include <string.h>


CONFIG sys_config;

const cjsonx_reflect_t device_reflection[] = {
    __cjsonx_str(CONFIG, time),
    __cjsonx_int(CONFIG, Software_version),
    __cjsonx_end()
};





void get_config(const char * keyname,const char * type){
    FILE* fp = fopen(CONFIG_FILE_PATH,"r");
    if (fp == NULL) {
        perror("Unable to open file");
        return;
    }

    fseek(fp,0,SEEK_END);
    int file_len = ftell(fp);
    fseek(fp,0,SEEK_SET);

    
    char * readbuf = (char *)malloc(file_len);
    if(readbuf == 0){
        perror("内存分配失败");
        fclose(fp);
        return;
    }
    readbuf[file_len] = '\0';

    size_t bytes_read = fread(readbuf, 1, file_len+1, fp);
    if (bytes_read == 0) {
        perror("Unable to read file");
        fclose(fp);
        return;
    }
    fclose(fp);

    cJSON* cjson_config = cJSON_Parse(readbuf);
    if (cjson_config == NULL){
        fprintf(stderr, "Failed to parse JSON\n");
        return;
    }

    cjsonx_obj2struct(cjson_config, &sys_config, device_reflection);

    printf("sys_config.time: %s\n",sys_config.time);
    printf("sys_config.Software_version: %d\n",sys_config.Software_version);


    // 释放 JSON 对象
    cJSON_Delete(cjson_config);
    free(readbuf);
}


void Determine_the_type(void * b)
{
    int a;
}
