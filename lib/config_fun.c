#include "get_config.h"
#include <stdio.h>
#include <cJSON/cJSON.h>
#include <stdlib.h>
#include <string.h>





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
    cJSON* config_weneed =  cJSON_GetObjectItem(cjson_config, keyname);
    if(config_weneed==NULL){
        fprintf(stderr, "Missing JSON fields\n");
        cJSON_Delete(cjson_config);
        return;
    }

    if (cJSON_IsString(config_weneed) && (config_weneed->valuestring != NULL)){
        printf("%s: %s\n", keyname, config_weneed->valuestring);
    }else{
        fprintf(stderr, "JSON field is not a string\n");
    }


    // 释放 JSON 对象
    cJSON_Delete(cjson_config);
    free(readbuf);
}


void Determine_the_type(void * b)
{
    int a;
}
