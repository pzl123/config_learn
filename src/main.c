#include <stdio.h>
#include "get_config.h"
#include "uthash/uthash.h"

typedef struct{
    char keyname[100];                   /* key */
    char value[100];
    UT_hash_handle hh;         /* makes this structure hashable */
}HashEntry;


HashEntry *MAP =NULL;
struct uint{
    char keyname[100];
    char value[100];
};


int main() {
    HashEntry *found;

    // 插入数据
    HashEntry *entry1 = (HashEntry *)malloc(sizeof(HashEntry));
    if (!entry1) return -1;
    strcpy(entry1->keyname, "abc");
    strcpy(entry1->value, "./abcname");
    HASH_ADD_STR(MAP, keyname, entry1);

    HashEntry *entry2 = (HashEntry *)malloc(sizeof(HashEntry));
    if (!entry2) return -1;
    strcpy(entry2->keyname, "def");
    strcpy(entry2->value, "./defname");
    HASH_ADD_STR(MAP, keyname, entry2);

    // 查找并使用数据
    HASH_FIND_STR(MAP, "abc", found);
    if (found) {
        printf("Found value for 'abc': %s\n", found->value);
    } else {
        printf("'abc' not found.\n");
    }

    // 清理内存
    HashEntry *current_entry, *tmp;
    HASH_ITER(hh, MAP, current_entry, tmp) {
        HASH_DEL(MAP, current_entry);
        free(current_entry);
    }

    printf("This is main cmake hello!\n");
    char* time = "time";
    char* type = "string";
    get_config(time, type);

    return 0;
}