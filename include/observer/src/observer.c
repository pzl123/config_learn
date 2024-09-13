#include "observer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//实现 ConcreteObserver
void concrete_observer_update(const char *event,int id) {
    printf("ConcreteObserver(id=%d) received event: %s\n", id, event);
}

void init_concrete_observer(ConcreteObserver *observer, int id) {
    observer->base.update = concrete_observer_update;
    observer->base.id = id;
}

//实现 ConcreteSubject
void subject_init(Subject *subject) {
    subject->observers = malloc(MAX_OBSERVERS * sizeof(Observer));
    subject->numObservers = 0;
}

void subject_add_observer(Subject *subject, Observer *observer) {
    if (subject->numObservers < MAX_OBSERVERS) {
        subject->observers[subject->numObservers++] = *observer;
    }
}

void subject_remove_observer(Subject *subject, Observer *observer) {
    for (int i = 0; i < subject->numObservers; ++i) {
        if (subject->observers[i].update == observer->update) {
            memmove(&subject->observers[i], &subject->observers[i + 1],
                    (subject->numObservers - i - 1) * sizeof(Observer));
            --subject->numObservers;
            break;
        }
    }
}

void subject_notify_observers(Subject *subject, const char *event) {
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

