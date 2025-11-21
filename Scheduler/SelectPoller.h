#pragma once
#include"Poller.h"
#include"Log.h"
#include<sys/select.h>
#include<vector>

class SelectPoller : public Poller{

public:
    static SelectPoller* createNew();
    SelectPoller();
    ~SelectPoller();
    
public:
    bool addIOEvent(IOEvent* event);
    bool updateIOEvent(IOEvent* event);
    bool removeIOEvent(IOEvent* event);
    void handleEvent();

private:

    fd_set mReadSet;
    fd_set mWriteSet;
    fd_set mExceptionSet;
    std::vector<IOEvent*> mIOEvents; //临时存储当前事件触发的IO事件
    int mMaxNumSockets;
};