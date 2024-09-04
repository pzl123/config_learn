#include <stdio.h>
#include "mylib.h"
#include "get_config.h"

int main()
{
    printf("this is main cmake hello!\n");
    char* time = "time";
    char* type = "string";
    get_config(time,type);
    return 0;
}
