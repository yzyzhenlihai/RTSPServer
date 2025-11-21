#pragma once
#include"UsageEnvironment.h"
#include"SocketsOps.h"
#include"Event.h"
#include"Buffer.h"
#include<functional>
class TcpConnection{
public:

    TcpConnection(UsageEnvironment* env, int clientfd);
    ~TcpConnection();

public:
    void handleRead();
    
    void setDisconnectCallback(std::function<void()> cb){mDisconnectCallback = cb;}
    void handleDisconnect();
    
protected:
    virtual void handleReadBytes();
    virtual void handleWrite();
    virtual void handleError();

protected:

    UsageEnvironment* mEnv;
    int mClientFd;
    IOEvent* mClientIOEvent;
    Buffer mReadBuffer; //读Buffer
    Buffer mWriteBuffer; // 写Buffer
    std::function<void()> mDisconnectCallback;
};