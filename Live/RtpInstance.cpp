#include"RtpInstance.h"
#include"SocketsOps.h"
#include<cstring>
/*
RtcpInstance defination
*/
RtpInstance* RtpInstance::createNewOverUdp(int localSockfd, uint16_t localPort, std::string destIp, uint16_t destPort){
    return new RtpInstance(localSockfd, localPort, destIp, destPort);
}
RtpInstance* RtpInstance::createNewOverTcp(int clientFd, int rtpChannel){
    return new RtpInstance(clientFd, rtpChannel);
}
//for tcp
RtpInstance::RtpInstance(int clientFd, int rtpChannel):
    mRtpType(RTP_OVER_TCP),
    mLocalSockfd(clientFd),
    mLocalPort(0),
    mSessionId(0),
    mIsAlive(false),
    mRtpChannel(rtpChannel)
{

}
//for udp
RtpInstance::RtpInstance(int localSockfd, uint16_t localPort, std::string destIp, uint16_t destPort):
    mRtpType(RTP_OVER_UDP),
    mLocalSockfd(localSockfd),
    mLocalPort(localPort),
    mDestAddr(destIp, destPort),
    mSessionId(0),
    mIsAlive(false),
    mRtpChannel(0)
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
        char buf[2048]={0};
        buf[0] = '$';
        buf[1] = static_cast<uint8_t>(mRtpChannel);
        int rtpSize = rtpPacket->size(); 
        buf[2] = (rtpSize & 0xFF00)>>8; //高八位
        buf[3] = rtpSize & 0xFF; // 低八位
        memcpy(buf+4, rtpPacket->data(), rtpSize);
        return sendOverTcp(buf, 4+rtpSize);
    }
}

int RtpInstance::sendOverUdp(void* buf, int size){
    return sockets::sendto(mLocalSockfd, buf, size, mDestAddr.getAddr());
}

int RtpInstance::sendOverTcp(void* buf, int size){
    return sockets::write(mLocalSockfd, (char*)buf, size);
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
