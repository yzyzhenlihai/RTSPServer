#include"RtpInstance.h"
#include"SocketsOps.h"

/*
RtcpInstance defination
*/
RtpInstance* RtpInstance::createNewOverUdp(int localSockfd, uint16_t localPort, std::string destIp, uint16_t destPort){
    return new RtpInstance(localSockfd, localPort, destIp, destPort);
}
RtpInstance* RtpInstance::createNewOverTcp(){

}

RtpInstance::RtpInstance(int localSockfd, uint16_t localPort, std::string destIp, uint16_t destPort):
    mRtpType(RTP_OVER_UDP),
    mLocalSockfd(localSockfd),
    mLocalPort(localPort),
    mDestAddr(destIp, destPort),
    mIsAlive(false)
{
     

}

void RtpInstance::setSessionId(int sessionId){
    mSessionId = sessionId;
}

uint16_t RtpInstance::getLocalPort(){
    return mLocalPort;
}
void RtpInstance::setAlive(bool isAlive){
    mIsAlive = isAlive;
}
//实现RTP发送逻辑
int RtpInstance::send(RtpPacket* rtpPacket){
    if(mRtpType == RTP_OVER_UDP){
        //基于UDP发送
        return sendOverUdp(rtpPacket->data(), rtpPacket->size());
        
    }else if(mRtpType == RTP_OVER_TCP){
        //TODO基于TCP发送

    }
}

int RtpInstance::sendOverUdp(void* buf, int size){
    return sockets::sendto(mLocalSockfd, buf, size, mDestAddr.getAddr());
}

int RtpInstance::sendOverTcp(void* buf, int size){

}
/*
RtcpInstance defination
*/
RtcpInstance* RtcpInstance::createNewOverUdp(int localSockfd, uint16_t localPort, std::string destIp, uint16_t destPort){
    return new RtcpInstance(localSockfd, localPort, destIp, destPort);
}

RtcpInstance::RtcpInstance(int localSockfd, uint16_t localPort, std::string destIp, uint16_t destPort):
    mLocalSockfd(localSockfd),
    mLocalPort(localPort),
    mDestAddr(destIp, destPort)
{


}

void RtcpInstance::setSessionId(int sessionId){
    mSessionId = sessionId;
}
uint16_t RtcpInstance::getLocalPort(){
    return mLocalPort;
}

void RtcpInstance::setAlive(bool isAlive){
    mIsAlive = isAlive;
}
