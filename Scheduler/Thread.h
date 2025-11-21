#pragma once
#include<functional>
#include<thread>

class Thread{
public:
    Thread();
    ~Thread();
    bool start(std::function<void()> threadFunc);
    bool join();
    bool detach();
protected:
    
private:
    std::function<void()> mThreadFunc;
    bool mIsStart;
    bool mIsDetach;
    std::thread mThread;
};