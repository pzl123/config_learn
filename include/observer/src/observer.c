#include "observer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//实现 ConcreteObserver
void concrete_observer_update(const char *event,int id) {
    printf("ConcreteObserver(id=%d) received event: %s\n", id, event);
}

void init_concrete_observer(ConcreteObserver_t *observer, int id) {
    observer->base.update = concrete_observer_update;
    observer->base.id = id;
}

//实现 ConcreteSubject
void subject_init(Subject_t *subject)
{
    subject->observers = malloc(MAX_OBSERVERS * sizeof(Observer_t));
    subject->numObservers = 0;
}

void subject_add_observer(Subject_t *subject, Observer_t *observer)
{
    if (subject->numObservers < MAX_OBSERVERS) {
        subject->observers[subject->numObservers++] = *observer;
    }
}

void subject_remove_observer(Subject_t *subject, Observer_t *observer) {
    for (int i = 0; i < subject->numObservers; ++i) {
        if (subject->observers[i].update == observer->update) {
            memmove(&subject->observers[i], &subject->observers[i + 1],
                    (subject->numObservers - i - 1) * sizeof(Observer_t));
            --subject->numObservers;
            break;
        }
    }
}

void subject_notify_observers(Subject_t *subject, const char *event) {
    char *p = malloc(strlen(event) + 100); 
    char buffer[20];
    if (!p) {
        fprintf(stderr, "Memory allocation failed.\n");
        return;
    }
    for (int i = 0; i < subject->numObservers; ++i) {
        strcpy(p, event);
        sprintf(buffer, " to observer%d", subject->observers[i].id);
        strcat(p, buffer);
        subject->observers[i].update(p, subject->observers[i].id);
        memset(p, 0, strlen(event) + 100);  // 清空缓冲区
    }

    free(p);  // 释放内存
}

/**
 * --------------------------------------------------------------------------------------------------------------------------------
 * 下面代码实现hash table中的观察者添加、删除、修改、通知
 * --------------------------------------------------------------------------------------------------------------------------------
 */

//实现hash table中的
// void config_add_observer(file_struct_t **config_table, Observer_t *observer)
// {
//     if ((*config_table)->numObservers < MAX_OBSERVERS) {
//         (*config_table)->list[(*config_table)->numObservers++] = *observer;
//     }
// }