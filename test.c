#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

char *sss = "default_config.json";

// 返回分割后的子串，假设分割符是下划线'_'
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
int main() {
    char *a = split(sss);
    if (a != NULL)
    {
        printf("%s\n", a);
        free(a); // 释放内存
    } 
    else
    {
        printf("Error: Could not allocate memory or find the delimiter.\n");
    }
    return 0;
}