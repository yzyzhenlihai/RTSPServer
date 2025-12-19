#include"SelectPoller.h"

SelectPoller* SelectPoller::createNew(){
    return new SelectPoller();
}
SelectPoller::SelectPoller():
    mMaxNumSockets(0)
{

    FD_ZERO(&mReadSet);
    FD_ZERO(&mWriteSet);
    FD_ZERO(&mExceptionSet);
}

SelectPoller::~SelectPoller(){}

bool SelectPoller::addIOEvent(IOEvent* event){
    return updateIOEvent(event);
}

bool SelectPoller::updateIOEvent(IOEvent* event){

    int fd = event->getFd();
    if(fd<0){
        LOGE("SelectPoller updateIOEvent getFd() error fd=%d", fd);
        return false;
    }
    //清除当前fd的位
    FD_CLR(fd, &mReadSet);
    FD_CLR(fd, &mWriteSet);
    FD_CLR(fd, &mExceptionSet);
    
    std::map<int, IOEvent*>::iterator it = mEventMap.find(fd);
    if(it != mEventMap.end()){
        //以及添加过当前事件，只需要修改
        if(event->isReadHandling()) FD_SET(fd, &mReadSet);
        if(event->isWriteHandling()) FD_SET(fd, &mWriteSet);
        if(event->isErrorHandling()) FD_SET(fd, &mExceptionSet);
        
    }else{
        //添加当前事件
        if(event->isReadHandling()) FD_SET(fd, &mReadSet);
        if(event->isWriteHandling()) FD_SET(fd, &mWriteSet);
        if(event->isErrorHandling()) FD_SET(fd, &mExceptionSet);

        mEventMap.insert({fd, event});
    }
    mMaxNumSockets = mEventMap.rbegin()->first + 1; // 最大描述符+1
    return true;
}

bool SelectPoller::removeIOEvent(IOEvent* event){
    int fd = event->getFd();
    if(fd<0){
        LOGE("SelectPoller removeIOEvent getFd() error fd=%d", fd);
        return false;
    }
    FD_CLR(fd, &mReadSet);
    FD_CLR(fd, &mWriteSet);
    FD_CLR(fd, &mExceptionSet);

    std::map<int, IOEvent*>::iterator it = mEventMap.find(fd);
    if(it != mEventMap.end()){
        mEventMap.erase(it);
    }

    if(mEventMap.empty()) mMaxNumSockets = 0;
    else mMaxNumSockets = mEventMap.rbegin()->first + 1;

    return true;
}
void SelectPoller::handleEvent(){

    fd_set readSet = mReadSet;
    fd_set writeSet = mWriteSet;
    fd_set exceptionSet = mExceptionSet;
    struct timeval timeout = {0};
    timeout.tv_sec = 1;
    int ret = select(mMaxNumSockets, &readSet, &writeSet, &exceptionSet, &timeout);
    int rEvent = 0;
    
    if(ret<0){
        LOGI("SelectPoller handleEvent select error ret=%d", ret);
        return;
    }
    
    //遍历所有注册的事件
    for(std::map<int, IOEvent*>::iterator it = mEventMap.begin(); it!=mEventMap.end(); it++){
        
        rEvent = 0;
        if(FD_ISSET(it->first, &readSet)){
            rEvent |= IOEvent::EVENT_READ;
        }
        if(FD_ISSET(it->first, &writeSet)){
            rEvent |= IOEvent::EVENT_WRITE;
        }
        if(FD_ISSET(it->first, &exceptionSet)){
            rEvent |= IOEvent::EVENT_ERROR;
        }
        if(rEvent!=0){
            it->second->setREvent(rEvent);
            mIOEvents.push_back(it->second);
        }
        
    }

    //执行IO事件
    for(auto& ioEvent : mIOEvents){
        ioEvent->handleEvent();
    }

    mIOEvents.clear();

    
}