#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct observer_list {
    int owner_num;
    struct observer_list *prev;
    struct observer_list *next;
} observer_t;

typedef struct {
    observer_t *owners; // 存储拥有者
    pthread_rwlock_t ownersLock;
} file_struct_t;

file_struct_t *A = NULL;

// 尾部添加观察者
void add_owner(file_struct_t **table, int owner_num) {
    file_struct_t *s = *table;
    observer_t *new_owner = (observer_t *)malloc(sizeof(observer_t));
    if (!new_owner) {
        printf("Memory allocation failed.\n");
        return;
    }
    new_owner->owner_num = owner_num;
    new_owner->prev = NULL;
    new_owner->next = NULL;

    pthread_rwlock_wrlock(&s->ownersLock);
    if (s->owners == NULL) {
        s->owners = new_owner;
    } else {
        observer_t *head = s->owners;
        while (head->next != NULL) {
            head = head->next;
        }
        head->next = new_owner;
        new_owner->prev = head;
    }
    pthread_rwlock_unlock(&s->ownersLock);
}

// 返回 owners 中标志位为 owner_num 的节点
observer_t *find_owner(file_struct_t **table, int owner_num) {
    file_struct_t *s = *table;
    pthread_rwlock_rdlock(&s->ownersLock);
    observer_t *head = s->owners;
    while (head != NULL) {
        if (head->owner_num == owner_num) {
            pthread_rwlock_unlock(&s->ownersLock);
            return head;
        }
        head = head->next;
    }
    pthread_rwlock_unlock(&s->ownersLock);
    return NULL;
}

// 删除指定节点，并释放内存
void delete_owner(file_struct_t **table, observer_t *owner) {
    file_struct_t *s = *table;

    pthread_rwlock_wrlock(&s->ownersLock);

    if (owner == s->owners) { // 处理头节点
        s->owners = owner->next;
    }

    if (owner->prev != NULL) {
        owner->prev->next = owner->next;
    }

    if (owner->next != NULL) {
        owner->next->prev = owner->prev;
    }

    free(owner);

    pthread_rwlock_unlock(&s->ownersLock);
}

//遍历所有节点
void traverse_owners(file_struct_t **table) {
    file_struct_t *s = *table;
    pthread_rwlock_rdlock(&s->ownersLock);
    observer_t *head = s->owners;
    while (head != NULL) {
        printf("owner_num: %d\n", head->owner_num);
        head = head->next;
    }
}




int main()
{
    A = (file_struct_t *)malloc(sizeof(file_struct_t));
    if(!A)
    {
        printf("Memory allocation for file_struct_t failed.\n");
        return 1;
    }

    A->owners = NULL;
    pthread_rwlock_init(&A->ownersLock, NULL);

    add_owner(&A, 1);
    add_owner(&A, 2);
    add_owner(&A, 3);

    // 查找节点
    observer_t *found = find_owner(&A, 2);
    if (found) {
        printf("Found node with owner_num %d\n", found->owner_num);
    } else {
        printf("Node not found\n");
    }

    // 删除节点
    delete_owner(&A, found);
    traverse_owners(&A);
    // 示例代码，展示如何销毁锁和释放内存
    pthread_rwlock_destroy(&A->ownersLock);
    observer_t *current = A->owners;
    while(current != NULL)
    {
        observer_t *next = current->next;
        free(current);
        current = next;
    }
    free(A);

    return 0;
}