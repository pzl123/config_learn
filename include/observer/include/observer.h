#ifndef _OBSERVER_H_
#define _OBSERVER_H_

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/*
*Subject（主题/被观察者）：
    主题对象维护一个观察者列表，并提供方法来添加和删除观察者。
    当主题的状态发生变化时，它会通知所有的观察者。
Observer（观察者）：
    观察者对象负责接收来自主题的通知，并更新自己的状态。
    观察者可以是任意数量的对象，它们都注册到主题上，等待状态变化的通知。
ConcreteSubject（具体主题）：
    具体主题继承或实现 Subject 接口，并实现通知观察者的方法。
    它包含具体的业务逻辑和状态。
ConcreteObserver（具体观察者）：
    具体观察者实现 Observer 接口，并定义更新自身状态的方法。
    当接收到主题的通知时，具体观察者会更新自己的状态。

以下是使用 C 语言实现观察者模式的基本步骤：

定义 Observer 接口：
    创建一个观察者接口，定义更新方法。
定义 Subject 接口：
    创建一个主题接口，定义添加、删除观察者和通知观察者的方法。
实现 ConcreteObserver（具体观察者）：
    实现具体的观察者类，实现更新方法。
实现 ConcreteSubject（具体主题）：
    实现具体的主题类，管理观察者列表并通知观察者。
*/

#define MAX_OBSERVERS 10


//定义 Observer 接口
typedef void (*UpdateFunc)(const char *event,int id);

typedef struct {
    UpdateFunc update;
    int id;
} Observer_t;


//定义 Subject 接口
typedef struct {
    Observer_t *observers;
    int numObservers;
} Subject_t;

void subject_init(Subject_t *subject);
void subject_add_observer(Subject_t *subject, Observer_t *observer);
void subject_remove_observer(Subject_t *subject, Observer_t *observer);
void subject_notify_observers(Subject_t *subject, const char *event);



//实现 ConcreteObserver
typedef struct {
    Observer_t base;
} ConcreteObserver_t;

void init_concrete_observer(ConcreteObserver_t *observer,int id);
void concrete_observer_update(const char *event,int id);


/**
 * --------------------------------------------------------------------------------------------------------------------------------
 * 下面代码实现hash table中的观察者添加、删除、修改、通知
 * --------------------------------------------------------------------------------------------------------------------------------
 * /



// void config_add_observer(file_struct_t **subject, Observer_t *observer);
// void config_remove_observer(Subject_t *subject, Observer_t *observer);
// void config_notify_observers(Subject_t *subject, const char *event);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OBSERVER_H__ */