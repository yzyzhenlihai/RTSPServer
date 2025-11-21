#pragma once
#include"EventScheduler.h"
#include"ThreadPool.h"
class UsageEnvironment
{

public:
    static UsageEnvironment* createNew(EventScheduler* eventScheduler, ThreadPool* threadPool);
    UsageEnvironment(EventScheduler* eventScheduler, ThreadPool* threadPool);
    ~UsageEnvironment();

public:
    EventScheduler* scheduler();
    ThreadPool* threadPool();
    
private:
    EventScheduler* mEventScheduler;
    ThreadPool* mThreadPool;
};


