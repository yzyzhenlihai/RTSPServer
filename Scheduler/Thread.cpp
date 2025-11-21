#include"Thread.h"


Thread::~Thread(){

}

Thread::Thread():
    mIsStart(false),
    mIsDetach(false),
    mThreadFunc(nullptr)
{

}

bool Thread::start(std::function<void()> threadFunc){
    mThreadFunc = threadFunc;
    mIsStart = true;
    mThread = std::thread([this](){
        if(mThreadFunc){
            mThreadFunc();
        }
    });
    return true;
}

// join模式
bool Thread::join(){
    if(!mIsStart || !mIsDetach){
        return false;
    }
    mThread.join();
    
    return true;
}

//detach模式
bool Thread::detach(){
    if(!mIsStart) return false;
    if(mIsDetach) return true;

    mThread.detach();
    mIsDetach = true;
    return true;
}

