#pragma once
#include<stdint.h>
#include<unordered_map>
#include<map>

class EventScheduler;
class Poller;
class TimerEvent;
class IOEvent;
/*
定时器包装器
用来描述一个定时器实例
1. 执行什么
2. 什么时候执行
*/
class Timer{
public:
    typedef uint32_t TimerId; 
    typedef int64_t TimeStamp; 
    typedef uint32_t TimeInterval;
    ~Timer();
    
    static TimeStamp getCurTime();
    
private:
    friend class TimerManager;  //定义友元函数，可以访问私有变量
    Timer(TimerEvent* timerEvent, TimeStamp timestamp, TimeInterval timeInterval, TimerId timerId);
    
private:
    
    TimerEvent* mTimerEvent;
    TimerId mTimerId; //定时器唯一标识符
    TimeStamp mTimerStamp; //下一次到期时间戳
    TimeInterval mTimerInterval;//如果是周期定时器，表示重复的时间间隔
    bool mIsRepeat; //true表示是周期定时器
    bool handleEvent();
};


class TimerManager{
public:
    static TimerManager* createNew(EventScheduler* eventSchduler);
    TimerManager(EventScheduler* eventSchduler);
    ~TimerManager();

public:
    void handleRead();
    Timer::TimeStamp addTimer(TimerEvent* event, Timer::TimeStamp timestamp, Timer::TimeInterval interval);

private:    
    void modifyTimerout(); //修改mTimerFd下一次到期的时间
private:
    Poller* mPoller;
    std::unordered_map<Timer::TimerId, Timer> mTimers;
    std::multimap<Timer::TimeStamp, Timer> mEvents; //以到期时间戳为key进行排序
    uint32_t mLastTimerId; //记录上一个登记的定时器ID
#ifndef WIN32
    int mTimerFd;
    IOEvent* mTimerIOEvent;
#endif

};