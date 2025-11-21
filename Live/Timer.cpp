#include"Timer.h"
#include"EventScheduler.h"
#include"Event.h"
#include"Poller.h"
#ifndef WIN32
#include<sys/timerfd.h>
#endif


static bool timerFdSetTime(int fd, Timer::TimeStamp expired, Timer::TimeInterval interval){
#ifndef WIN32
    
    struct itimerspec newVal={0}; //全为0会解除定时器
    newVal.it_value.tv_sec = expired/1000;  //ms->s 转化成秒
    newVal.it_value.tv_nsec = expired%1000 * 1000 * 1000;//ms->ns 转化成纳秒
    newVal.it_interval.tv_sec = interval / 1000;
    newVal.it_interval.tv_nsec = interval % 1000 * 1000 * 1000;
    int oldValue = timerfd_settime(fd, TFD_TIMER_ABSTIME, &newVal, nullptr);
    if(oldValue<0){
        return false;
    }

#endif  
    return true;
}
/*
Timer 实现
*/
Timer::TimeStamp Timer::getCurTime(){
#ifndef WIN32
    struct timespec now; // tv_sec (s) tv_nsec (ns-纳秒)
    clock_gettime(CLOCK_MONOTONIC, &now); //表示一个从系统启动开始就只增不减的时钟。它不受用户修改系统时间的影响，是实现定时器最可靠的时间源。
    return now.tv_sec * 1000 + now.tv_nsec/1000000;  // 返回的是毫秒级别
#else
    //TODO windows下获取系统时间
#endif
}

Timer::Timer(TimerEvent* timerEvent, TimeStamp timestamp, TimeInterval timeInterval, TimerId timerId):
    mTimerEvent(timerEvent),
    mTimerStamp(timestamp),
    mTimerInterval(timeInterval),
    mTimerId(timerId)
{
    if(timeInterval>0){
        mIsRepeat = true;
    }else{
        mIsRepeat = false;
    }
}

Timer::~Timer(){

}

bool Timer::handleEvent(){
    if(!mTimerEvent){
        return false;
    }
    return mTimerEvent->handleEvent(); //执行定时器事件
    
}
/*
TimerManager 实现
*/
TimerManager* TimerManager::createNew(EventScheduler* eventSchduler){
    return new TimerManager(eventSchduler);
}

TimerManager::TimerManager(EventScheduler* eventSchduler):
    mLastTimerId(0),
    mPoller(eventSchduler->poller())
{
    //创建定时器IO事件
#ifndef WIN32
    //非Windows系统下使用
    
    mTimerFd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC); //创建定时器文件描述符
    if(mTimerFd<0){
        LOGE("create timerfd error");
        return;
    }else{
        LOGI("timerfd=%d", mTimerFd);
    }
    mTimerIOEvent = IOEvent::createNew(mTimerFd);
    mTimerIOEvent->setReadCallback(std::bind(&TimerManager::handleRead,this));
    mTimerIOEvent->enableReadHandling();
    mPoller->addIOEvent(mTimerIOEvent);
    modifyTimerout();
#else
    //TODO window环境下来执行定时器事件
#endif
}

TimerManager::~TimerManager(){
#ifndef WIN32
    mPoller->removeIOEvent(mTimerIOEvent);
    delete mTimerIOEvent;
#endif
}

//处理定时器事件
void TimerManager::handleRead(){
    Timer::TimeStamp curTimestamp = Timer::getCurTime();
    if(!mTimers.empty() && !mEvents.empty()){
        std::multimap<Timer::TimeStamp, Timer>::iterator it = mEvents.begin();
        Timer timer = it->second;
        int expire = curTimestamp - timer.mTimerStamp;
        if(expire>=0){
            //已经过期
            bool timerEventIsStop = timer.handleEvent(); //执行定时器事件
            if(timer.mIsRepeat){
                if(timerEventIsStop){
                    mTimers.erase(timer.mTimerId);
                    mEvents.erase(it);
                }else{
                    mEvents.erase(it);
                    timer.mTimerStamp = curTimestamp + timer.mTimerInterval;
                    mEvents.insert({timer.mTimerStamp, timer});
                }
            }else{
                mTimers.erase(timer.mTimerId);
                mEvents.erase(it);
            }
            

        }
    }
    modifyTimerout(); // 修改定时器IO的出发时间
}

void TimerManager::modifyTimerout(){
    /*
    修改定时器文件描述符的触发时间
    */
#ifndef WIN32
    std::multimap<Timer::TimeStamp, Timer>::iterator it = mEvents.begin();
    if(it !=mEvents.end()){
        Timer timer = it->second;
        timerFdSetTime(mTimerFd, timer.mTimerStamp, timer.mTimerInterval);
    }else{
        timerFdSetTime(mTimerFd, 0, 0);
    }
#endif
}

Timer::TimeStamp TimerManager::addTimer(TimerEvent* event, Timer::TimeStamp timestamp, Timer::TimeInterval interval){

    mLastTimerId++;
    Timer timer(event, timestamp, interval, mLastTimerId);
    mTimers.insert({mLastTimerId, timer});
    mEvents.insert({timestamp, timer});
    modifyTimerout();
    return mLastTimerId;
}
