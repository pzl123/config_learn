#include "observer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void concrete_observer_update(const char *event) {
    printf("ConcreteObserver received event: %s\n", event);
}

void init_concrete_observer(ConcreteObserver *observer) {
    observer->base.update = concrete_observer_update;
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
    for (int i = 0; i < subject->numObservers; ++i) {
        subject->observers[i].update(event);
    }
}

