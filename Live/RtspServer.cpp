#include "RtspServer.h"



RtspServer* RtspServer::createNew(UsageEnvironment* env,Ipv4Address& addr,MediaSessionManager* sessionManager)
{
    return new RtspServer(env, addr, sessionManager);
}

RtspServer::RtspServer(UsageEnvironment* env, Ipv4Address& addr,MediaSessionManager* sessionManager):
    mEnv(env),
    mAddr(addr),
    mListen(false),
    mAcceptIOEvent(nullptr),
    mCloseTriggerEvent(nullptr),
    mSessionManager(sessionManager)
{
    //1. 创建监听描述符
    mFd = sockets::createTcpSock();
    sockets::setReuseAddr(mFd, 1); // 设置端口复用
    //2. 绑定地址
    sockets::bind(mFd, addr.getIp(), addr.getPort());
    //3. 向事件调度器添加IO事件
    LOGI("rtsp://%s:%d", addr.getIp().c_str(), addr.getPort());
    
    mAcceptIOEvent = IOEvent::createNew(mFd);
    mAcceptIOEvent->enableReadHandling();
    mAcceptIOEvent->setReadCallback(std::bind(&RtspServer::handleRead, this));

    //4. 构造触发事件,关闭连接
    mCloseTriggerEvent = TriggerEvent::createNew();
    mCloseTriggerEvent->setTriggerCallback(std::bind(&RtspServer::handleCloseConnect, this));

}

RtspServer::~RtspServer(){
    
    mEnv->scheduler()->removeIOEvent(mAcceptIOEvent);
    
    delete mAcceptIOEvent;
    delete mCloseTriggerEvent;

    ::close(mFd);
    
}

void RtspServer::handleRead(){

    while(true){
        int clientFd= sockets::accept(mFd);
        if(clientFd<0){
            if(errno == EAGAIN || errno == EWOULDBLOCK){
                break;
            }else{
                LOGE("RtspServer handleRead error,listenFd=%d,clientFd=%d,errno = %d: %s", mFd, clientFd, errno, strerror(errno));
                break;
            }
            
        }
        LOGI("clientFd=%d connect to RtspServer successfully", clientFd);
        //创建客户端连接
        RtspConnection* clientConn = RtspConnection::createNew(this, clientFd);
        clientConn->setDisconnectCallback(std::bind(&RtspServer::handleDisconnect, this, clientFd));
        mConnMap.insert({clientFd, clientConn});
    }
}

//断开某个客户端的连接
void RtspServer::handleDisconnect(int clientFd){
    std::unique_lock<std::mutex> locker(mMtx);
    mDisconnList.push_back(clientFd);
    //添加触发事件
    mEnv->scheduler()->addTriggerEvent(mCloseTriggerEvent);
}

void RtspServer::handleCloseConnect(){
    std::unique_lock<std::mutex> locker(mMtx);
    for(auto clientFd : mDisconnList){
        std::map<int, RtspConnection*>::iterator it = mConnMap.find(clientFd);
        if(it!=mConnMap.end()){
            LOGI("clientFd=%d disconnected", clientFd);
            delete it->second;
            mConnMap.erase(clientFd);
            sockets::close(clientFd);
        }
    }
    mDisconnList.clear();
}

void RtspServer::start(){
    mListen = true;
    bool ret = sockets::listen(mFd, 60);
    // 向事件调度器添加IO事件
    mEnv->scheduler()->addIOEvent(mAcceptIOEvent);
}
