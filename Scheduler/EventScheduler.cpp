#include"EventScheduler.h"

EventScheduler* EventScheduler::createNew(PollerType pollerType){

    return new EventScheduler(pollerType);
}
EventScheduler::EventScheduler(PollerType pollerType):
    mQuit(false),
    mPoller(nullptr)
{
    
    switch (pollerType)
    {
    case PollerType::POLLER_SELECT:
        /* code */
        mPoller = new SelectPoller();
        break;
    
    default:
        mPoller = new SelectPoller();
        break;
    }
    mTimerManager = TimerManager::createNew(this); // 创建定时器管理器
}
EventScheduler::~EventScheduler(){
    delete mPoller;
    delete mTimerManager;
    mPoller = nullptr;
}

bool EventScheduler::addIOEvent(IOEvent* event){

    return mPoller->addIOEvent(event);

    
}

bool EventScheduler::removeIOEvent(IOEvent* event){
    return mPoller->removeIOEvent(event);
}

bool EventScheduler::addTriggerEvent(TriggerEvent* event){
    mTriggerEvents.push_back(event);
    return true;
}

void EventScheduler::handleTriggerEvent(){
    if(!mTriggerEvents.empty()){
        for(auto& event : mTriggerEvents){
            event->handleEvent();
        }
        mTriggerEvents.clear();
    }
}

void EventScheduler::loop(){
#ifdef WIN32
    //TODO window环境下定时器事件需要额外开线程来处理
#endif

    while(!mQuit){
        handleTriggerEvent();
        mPoller->handleEvent();
    }
}

Poller* EventScheduler::poller(){
    return mPoller;
}


Timer::TimerId EventScheduler::addTimerEventRunEvery(TimerEvent* event, Timer::TimeInterval interval){
    /*
    添加周期性定时器事件
    */
   Timer::TimeStamp timestamp = Timer::getCurTime();  // 返回的是毫秒级别
   timestamp +=interval;

   return mTimerManager->addTimer(event, timestamp, interval);
   
}