#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

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
volatile int exit_flag = 0; // 全局标志用于控制线程退出

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
        if (s->owners != NULL) {
            s->owners->prev = NULL;
        }
        free(owner);
    } else if (owner->prev != NULL) { // 处理中间节点
        owner->prev->next = owner->next;
        if (owner->next != NULL) {
            owner->next->prev = owner->prev;
        }
        free(owner);
    } else if (owner->next == NULL && owner->prev != NULL) { // 处理尾节点
        owner->prev->next = NULL;
        free(owner);
    } else {
        free(owner);
        printf("Error: Unknown node type\n");
    }

    pthread_rwlock_unlock(&s->ownersLock);
}

// 遍历所有节点
void traverse_owners(file_struct_t **table) {
    file_struct_t *s = *table;
    pthread_rwlock_rdlock(&s->ownersLock);
    observer_t *head = s->owners;
    while (head != NULL) {
        printf("owner_num: %d\n", head->owner_num);
        head = head->next;
    }
    pthread_rwlock_unlock(&s->ownersLock);
}


//删除所有节点
void delete_all_owners(file_struct_t **table) {
    file_struct_t *s = *table;
    pthread_rwlock_wrlock(&s->ownersLock);
    observer_t *head = s->owners;
    while (head != NULL) {
        observer_t *temp = head;
        head = head->next;
        free(temp);
    }
    s->owners = NULL;
}


void *thread_func1(void *arg)
{
    while (!exit_flag) {
        int i = rand() % 10;
        add_owner(&A, i);
        printf("owners add owner_num %d\n", i);
        time_t now = time(NULL);
        printf("-------------------------------time is %ld -----attach id %ld---------------------------------------------------\n", now, pthread_self());
        // sleep(1);
    }
    return NULL;
}

void *thread_func2(void *arg)
{
    while (!exit_flag) {
        int i = rand() % 10;
        observer_t *owner = find_owner(&A, i);
        if (owner) {
            printf("owners delete owner_num %d\n", owner->owner_num);
            delete_owner(&A, owner);
        }
        time_t now = time(NULL);
        printf("-------------------------------time is %ld -----detach id %ld---------------------------------------------------\n", now, pthread_self());
        // sleep(1);
    }
    return NULL;
}

void *thread_func3(void *arg)
{
    file_struct_t *s = arg;
    while (!exit_flag) {
        traverse_owners(&s);
        time_t now = time(NULL);
        printf("-------------------------------time is %ld -----traverse id %ld---------------------------------------------------\n", now, pthread_self());
        // sleep(1);
    }
    return NULL;
}

int test1(void)
{
    pthread_t thread1, thread2, thread3;
    A = (file_struct_t *)malloc(sizeof(file_struct_t));
    if (!A) {
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

    // 创建线程
    pthread_create(&thread1, NULL, thread_func1, NULL);
    pthread_create(&thread2, NULL, thread_func2, NULL);
    pthread_create(&thread3, NULL, thread_func3, A);

    // 等待一段时间后设置退出标志
    sleep(100); // 假设等待10秒后停止线程
    exit_flag = 1;

    // 等待线程结束
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);

    // 示例代码，展示如何销毁锁和释放内存
    pthread_rwlock_destroy(&A->ownersLock);
    observer_t *current = A->owners;
    while (current != NULL) {
        observer_t *next = current->next;
        free(current);
        current = next;
    }
    free(A);
    return 0;
}

int test2(void)
{
    int i = 10;
    char *j = (char *)malloc(10);
    strcpy(j, "123456789");
    j[10] = '\0';
    printf("%s\n", j);
}

int removeElement(int* nums, int numsSize, int val)
{
    int k = 0, i = 0;
    for(i = 0; i < numsSize; i++)
    {
        if(nums[i] != val)
        {
            nums[k++] = nums[i];
        }
    }
    return k;
}

int removeDuplicates(int* nums, int numsSize)
{
    int k = 1, i = 1;
    for(i = 0; i < numsSize; i++)
    {
        if(nums[i-1] != nums[i])
        {
            nums[k++] = nums[i];
        }
    }
    return k;
}



/*****************************队列***********************************/// Task definition
typedef struct task
{
    void (*func)(void *arg);
    void *arg;
} TASK_t;

typedef struct queue
{
    TASK_t *task;
    int32_t head;
    int32_t tail;
    int32_t size;
    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_cond;
} QUEUE_t;


typedef struct taskqueue
{
    QUEUE_t *queue;
    pthread_t thread_id[4]; 
} TASKQUEUE_t;

void task_queue_init(QUEUE_t *q, int32_t size);
void queue_destroy(QUEUE_t *q);
bool queue_is_empty(const QUEUE_t *q);
bool queue_is_full(const QUEUE_t *q);
bool queue_push(QUEUE_t *q, const TASK_t *task);
bool queue_pop(QUEUE_t *q, TASK_t *task);
void *worker(void *arg);
void taskqueue_init(TASKQUEUE_t *q, int queue_size);
void taskqueue_destroy(TASKQUEUE_t *q);
void submit_task(TASKQUEUE_t *q, void (*func)(void *), void *arg);
void example_task(void *arg);

void task_queue_init(QUEUE_t *q, int32_t size)
{
    q->task = (TASK_t *)malloc(sizeof(TASK_t) * size);
    if (!q->task) {
        perror("Failed to allocate memory for task array");
        exit(EXIT_FAILURE);
    }
    q->head = 0;
    q->tail = 0;
    q->size = size;
    pthread_mutex_init(&q->queue_mutex, NULL);
    pthread_cond_init(&q->queue_cond, NULL);
}

void queue_destroy(QUEUE_t *q)
{
    // Free the allocated memory
    free(q->task);
    pthread_mutex_destroy(&q->queue_mutex);
    pthread_cond_destroy(&q->queue_cond);
}

bool queue_is_empty(const QUEUE_t *q)
{
    return q->head == q->tail;
}

bool queue_is_full(const QUEUE_t *q)
{
    return (q->tail + 1) % q->size == q->head;
}

bool queue_push(QUEUE_t *q, const TASK_t *task)
{
    pthread_mutex_lock(&q->queue_mutex);
    if (queue_is_full(q))
    {
        printf("Error: queue is full\n");
        pthread_mutex_unlock(&q->queue_mutex);
        return false;
    }

    q->task[q->tail] = *task;

    q->tail = (q->tail + 1) % q->size;

    pthread_cond_signal(&q->queue_cond);
    pthread_mutex_unlock(&q->queue_mutex);
    return true;
}

bool queue_pop(QUEUE_t *q, TASK_t *task)
{
    pthread_mutex_lock(&q->queue_mutex);
    while (queue_is_empty(q))
    {
        printf("Error: queue is empty\n");
        pthread_cond_wait(&q->queue_cond, &q->queue_mutex);
    }

    *task = q->task[q->head];
    q->head = (q->head + 1) % q->size;

    pthread_mutex_unlock(&q->queue_mutex);
    return true;
}

void *worker(void *arg)
{
    TASKQUEUE_t *q = (TASKQUEUE_t *)arg;
    TASK_t task;
    while (1)
    {
        queue_pop(q->queue, &task);
        if (task.func != NULL)
        {
            task.func(task.arg);
        }
        else
        {
            break; 
        }
    }
    return NULL;
}

void taskqueue_init(TASKQUEUE_t *q, int queue_size)
{
    q->queue = malloc(sizeof(QUEUE_t));
    if (!q->queue) {
        perror("Failed to allocate memory for queue");
        exit(EXIT_FAILURE);
    }
    task_queue_init(q->queue, queue_size);

    for (int i = 0; i < 4; i++) {
        pthread_create(&q->thread_id[i], NULL, worker, q);
    }
}

void taskqueue_destroy(TASKQUEUE_t *q)
{
    TASK_t stop_task = {NULL, NULL};
    for (int i = 0; i <= 4; i++) {
        queue_push(q->queue, &stop_task);
    }

    for (int i = 0; i < 4; i++) {
        if (pthread_join(q->thread_id[i], NULL) != 0)
        {
            perror("Failed to join thread");
        }
    }
    
    queue_destroy(q->queue);

    free(q->queue);
}

void submit_task(TASKQUEUE_t *q, void (*func)(void *), void *arg)
{
    TASK_t task = {func, arg};
    queue_push(q->queue, &task);
}

void example_task(void *arg)
{
    if (arg == NULL) {
        printf("Warning: arg is NULL!\n");
        return;
    }
    
    int32_t value  = *(int32_t *)arg;
    
    printf("pthread_id %ld, pop task value is %d\n", (long)pthread_self(), value);
    sleep(1);
}

void test_queue()
{
    TASKQUEUE_t q;
    int queue_size = 10; 

    taskqueue_init(&q, queue_size); 

    while (1)
    {
        int32_t a;
        printf("Enter a number (or 0 to exit): ");
        scanf("%d", &a);

        if (a == 0)
        {
            break;
        }

        submit_task(&q, example_task, (void *)&a);
    }

    taskqueue_destroy(&q); 
}

int main()
{
    // test2();
    test_queue();
    return 0;
}