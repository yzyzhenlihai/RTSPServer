#include"Event.h"

/*
IO事件
*/
IOEvent* IOEvent::createNew(int fd){
    if(fd<0) return nullptr;
    return new IOEvent(fd);
}


IOEvent::IOEvent(int fd):
    mFd(fd),
    mEvent(EVENT_NONE),
    mREvent(EVENT_NONE),
    mReadCallback(nullptr),
    mWriteCallback(nullptr),
    mErrorCallback(nullptr)
{

}

void IOEvent::handleEvent(){
    if(mReadCallback && (mREvent & EVENT_READ)){
        mReadCallback();
    }
    if(mWriteCallback && (mREvent & EVENT_WRITE)){
        mWriteCallback();
    }
    if(mErrorCallback && (mREvent & EVENT_ERROR)){
        mErrorCallback();
    }
}

IOEvent::~IOEvent(){}

/*触发事件*/
TriggerEvent* TriggerEvent::createNew(){
    return new TriggerEvent();
}
TriggerEvent::TriggerEvent():
    mTriggerCallback(nullptr)
{

}
TriggerEvent::~TriggerEvent(){

}

void TriggerEvent::handleEvent(){
    if(mTriggerCallback){
        mTriggerCallback();
    }
}

/*
定时器事件
*/
TimerEvent* TimerEvent::createNew(){
    return new TimerEvent();
}   

TimerEvent::TimerEvent():
    mTimerCallback(nullptr),
    mIsStop(false)
{

}

TimerEvent::~TimerEvent(){

}

bool TimerEvent::handleEvent(){
    if(mIsStop){
        return mIsStop;
    }

    if(mTimerCallback){
        mTimerCallback();
    }
    return mIsStop;
}
