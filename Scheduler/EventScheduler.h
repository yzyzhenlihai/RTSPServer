#pragma once
#include"Event.h"
#include"SelectPoller.h"
#include"Timer.h"
#include<vector>
class EventScheduler{
public:
    // 事件调度模型的类型
    enum PollerType{
        POLLER_SELECT,
        POLLER_POLL,
        POLLER_EPOLL
    };
    static EventScheduler* createNew(PollerType pollerType);
    EventScheduler(PollerType pollerType);
    ~EventScheduler();

public:
    bool addIOEvent(IOEvent* event);
    bool removeIOEvent(IOEvent* event);
    bool addTriggerEvent(TriggerEvent* event);
    void handleTriggerEvent();
    void loop();
    Poller* poller();
    Timer::TimerId addTimerEventRunEvery(TimerEvent* event, Timer::TimeInterval interval);
private:

    bool mQuit;
    Poller* mPoller;
    std::vector<TriggerEvent*> mTriggerEvents;
    TimerManager* mTimerManager;
    
};