#pragma once 
#include<stdint.h>
#include<string>
#include"InetAddress.h"
#include"Rtp.h"
class RtpInstance{


public:
    //定义RTP传输类型
    enum RtpType{
        RTP_OVER_UDP,
        RTP_OVER_TCP
    };
    static RtpInstance* createNewOverUdp(int localSockfd, uint16_t localPort, std::string destIp, uint16_t destPort);
    static RtpInstance* createNewOverTcp(int clientFd, int rtpChannel);

    RtpInstance(int localSockfd, uint16_t localPort, std::string destIp, uint16_t destPort);
    RtpInstance(int clientFd, int rtpChannel);
public:

    void setSessionId(int sessionId);
    uint16_t getLocalPort();
    void setAlive(bool isAlive);
    bool alive(){ return mIsAlive;}
    int send(RtpPacket* rtpPacket);
    uint16_t getPeerPort();
private:
    int sendOverUdp(void* buf, int size);
    int sendOverTcp(void* buf, int size);
private:
    RtpType mRtpType;
    int mLocalSockfd;
    uint16_t mLocalPort; // for udp
    Ipv4Address mDestAddr; //for udp
    int mSessionId;
    bool mIsAlive;
    int mRtpChannel;    // for tcp
};

class RtcpInstance{

public:
    static RtcpInstance* createNewOverUdp(int localSockfd, uint16_t localPort, std::string destIp, uint16_t destPort);
    RtcpInstance(int localSockfd, uint16_t localPort, std::string destIp, uint16_t destPort);

public:
    void setSessionId(int sessionId);
    uint16_t getLocalPort();
    void setAlive(bool isAlive);
private:
    int mLocalSockfd;
    uint16_t mLocalPort;
    Ipv4Address mDestAddr;
    uint16_t mSessionId;
    bool mIsAlive;
    
};