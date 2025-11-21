#pragma once
#include<stdint.h>
#include<string>
#include<functional>
#include"Event.h"
#include"Rtp.h"
#include"Timer.h"
class UsageEnvironment;
class MediaSource;
class MediaFrame;
class Sink
{

public:
    Sink(UsageEnvironment* env, MediaSource* source,int payloadType);
    static const int RTP_MAX_PKT_SIZE = 1400;
    virtual ~Sink();
    void setSessionPacketCallBack(std::function<void(RtpPacket*)> cb){mSessionSendPacketCallBack = cb;}
    virtual std::string getMediaDescription(uint16_t port) = 0; //纯虚函数
    virtual std::string getAttribute() = 0;
protected:    

    virtual void sendFrame(MediaFrame* frame) = 0; // 由具体子类实现发送逻辑
    void handleTimeout();
    void sendRtpPacket(RtpPacket* rtpPacket);
    void runEvery(int interval);
    
protected:
    
    UsageEnvironment* mEnv;
    MediaSource* mMediaSource;
    std::function<void(RtpPacket*)> mSessionSendPacketCallBack; //连接Sink和MediaSession的桥梁

    //定义RTP包头
    uint8_t mVersion;
    uint8_t mPadding;
    uint8_t mExtension;
    uint8_t mCsrcLen;

    uint8_t mMarker;
    uint8_t mPayloadType;

    uint16_t mSeq;
    uint32_t mTimestamp;
    uint32_t mSsrc;
    
    RtpPacket mRtpPacket;
private:
    TimerEvent* mTimerEvent;
    Timer::TimerId mTimerId;
    

};


