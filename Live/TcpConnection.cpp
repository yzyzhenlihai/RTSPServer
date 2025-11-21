#include"TcpConnection.h"


TcpConnection::TcpConnection(UsageEnvironment* env, int clientFd):
    mEnv(env),
    mClientFd(clientFd),
    mDisconnectCallback(nullptr)
{
    //为客户端连接创建IO事件
    mClientIOEvent = IOEvent::createNew(clientFd);
    mClientIOEvent->setReadCallback(std::bind(&TcpConnection::handleRead, this));
    mClientIOEvent->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    mClientIOEvent->setErrorCallback(std::bind(&TcpConnection::handleError, this));
    mClientIOEvent->enableReadHandling();
    //添加到事件循环
    env->scheduler()->addIOEvent(mClientIOEvent);
}

TcpConnection::~TcpConnection(){
    mEnv->scheduler()->removeIOEvent(mClientIOEvent);
    delete mClientIOEvent;
    sockets::close(mClientFd);
}

void TcpConnection::handleRead(){
    // char buf[128];
    // int ret = read(mClientFd, buf, sizeof(buf));
    // if(ret<0){
    //     handleDisconnect();
    //     return;
    // }else{
    //     LOGI("read %s from client = %d", buf, mClientFd);
    // }
    int ret = mReadBuffer.read(mClientFd);
    if(ret<0){
        LOGI("read error, fd=%d, ret=%d", mClientFd, ret);
        handleDisconnect();
        return;
    }
    mReadBuffer.showBuffer();
    handleReadBytes();
}

void TcpConnection::handleWrite(){

}

void TcpConnection::handleError(){

}

void TcpConnection::handleDisconnect(){
    if(mDisconnectCallback){
        mDisconnectCallback();
    }
}

void TcpConnection::handleReadBytes(){
    
}
