#pragma once
#include<functional>
//IO事件
class IOEvent{
public:
    enum IOEventType{
        EVENT_NONE = 0,
        EVENT_READ = 1,
        EVENT_WRITE = 2,
        EVENT_ERROR = 4
    };

    static IOEvent* createNew(int fd);
    IOEvent(int fd);
    ~IOEvent();
    void setReadCallback(std::function<void()> cb){mReadCallback = cb;}
    void setWriteCallback(std::function<void()> cb){mWriteCallback = cb;}
    void setErrorCallback(std::function<void()> cb){mErrorCallback = cb;}

    void enableReadHandling(){mEvent |= EVENT_READ;}
    void enableWriteHandling(){mEvent |= EVENT_WRITE;}
    void enableErrorHandling(){mEvent |= EVENT_ERROR;}

    void disableReadHandling(){mEvent &= ~EVENT_READ;}
    void disableWriteHandling() { mEvent &= ~EVENT_WRITE; }
    void disableErrorHandling() { mEvent &= ~EVENT_ERROR; }

    bool isReadHandling(){return mEvent & EVENT_READ;}
    bool isWriteHandling(){return mEvent & EVENT_WRITE;}
    bool isErrorHandling(){return mEvent & EVENT_ERROR;}

    void setREvent(int rEvent){mREvent = rEvent;}
    void handleEvent();

    int getFd(){return mFd;}
private:
    
    int mFd;    //文件描述符
    int mEvent; // 目标监听事件read/write/error
    int mREvent;// 当前触发的监听事件read/write/error
    std::function<void()> mReadCallback; 
    std::function<void()> mWriteCallback;
    std::function<void()> mErrorCallback;
};

//触发事件，比如断开连接
class TriggerEvent{
public:
    static TriggerEvent* createNew();
    TriggerEvent();
    ~TriggerEvent();
    void setTriggerCallback(std::function<void()> cb){mTriggerCallback = cb;}
    void handleEvent();

private:
    std::function<void()> mTriggerCallback;
};

//定时器事件
class TimerEvent{
public:
    static TimerEvent* createNew();
    TimerEvent();
    ~TimerEvent();

public:
    bool handleEvent();
    void setTimerCallback(std::function<void()> cb){mTimerCallback = cb;}

private:
    std::function<void()> mTimerCallback;
    bool mIsStop;
};