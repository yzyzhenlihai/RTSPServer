#pragma once
#include"InetAddress.h"
#include"SocketsOps.h"
#include"Log.h"
#include"Event.h"
#include"EventScheduler.h"
#include"UsageEnvironment.h"
#include"RtspConnection.h"
#include"MediaSession.h"
#include"MediaSessionManager.h"
#include<cstring>
#include<mutex>
class RtspServer{

public:
    static RtspServer* createNew(UsageEnvironment* env,Ipv4Address& addr, MediaSessionManager* sessionManager);
    RtspServer(UsageEnvironment* env,Ipv4Address& addr,MediaSessionManager* sessionManager);
    ~RtspServer();
    void start();
    void handleDisconnect(int clientFd);
    void handleCloseConnect();
    UsageEnvironment* env(){return mEnv;}
    MediaSessionManager* getSessionManager(){return mSessionManager;}
private:
    void handleRead();
private:

    Ipv4Address mAddr;
    int mFd; // 监听描述符
    IOEvent* mAcceptIOEvent;
    TriggerEvent* mCloseTriggerEvent;
    bool mListen;
    UsageEnvironment* mEnv;
    std::map<int, RtspConnection*> mConnMap; 
    std::vector<int> mDisconnList;
    std::mutex mMtx;
    MediaSessionManager* mSessionManager;
};