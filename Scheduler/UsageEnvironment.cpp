#include"UsageEnvironment.h"

UsageEnvironment* UsageEnvironment::createNew(EventScheduler* eventScheduler, ThreadPool* threadPool){

    return new UsageEnvironment(eventScheduler, threadPool);
}

UsageEnvironment::UsageEnvironment(EventScheduler* eventScheduler, ThreadPool* threadPool):
    mEventScheduler(eventScheduler),
    mThreadPool(threadPool)
{

}

EventScheduler* UsageEnvironment::scheduler(){
    return mEventScheduler;
}

UsageEnvironment::~UsageEnvironment()
{
        
}

ThreadPool* UsageEnvironment::threadPool(){
    return mThreadPool;
}