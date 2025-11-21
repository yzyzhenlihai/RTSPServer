#include"Sink.h"
#include"UsageEnvironment.h"
#include"MediaSource.h"
#include<arpa/inet.h>
Sink::Sink(UsageEnvironment* env, MediaSource* source, int payloadType):
    mEnv(env),
    mMediaSource(source),
    mRtpPacket(2000),
    mVersion(RtpPacket::RTP_VERSION),
    mPadding(0),
    mExtension(0),
    mCsrcLen(0),
    mMarker(0),
    mPayloadType(payloadType),
    mSeq(0),
    mTimestamp(0),
    mSsrc(rand())

{
    mTimerEvent = TimerEvent::createNew();
    mTimerEvent->setTimerCallback(std::bind(&Sink::handleTimeout, this));

}

Sink::~Sink()
{   

}

// 定时器事件绑定的回调后函数
void Sink::handleTimeout(){
    //LOGI("handleTimeout successfully");
    //1. 从MediaSource中获取帧
    MediaFrame* frame = mMediaSource->getFrameFromOutputQueue();
    //2. 由具体子类实现打包和发送逻辑
    if(!frame){
        return;
    }
    this->sendFrame(frame); //由子类实现具体的发送逻辑
    //3. 将使用过的frame插入输入队列
    mMediaSource->putFrameToInputQueue(frame); // 由线程执行任务，往帧中填原始数据
}   

void Sink::runEvery(int interval){
    mTimerId = mEnv->scheduler()->addTimerEventRunEvery(mTimerEvent, interval); //添加定时器事件
}

//填充RtpHeader并打包发送
void Sink::sendRtpPacket(RtpPacket* rtpPacket){

    RtpHeader* rtpHeader = (RtpHeader*)rtpPacket->data();
    rtpHeader->firstByte = (mVersion<<6) | (mPadding<<5) | (mExtension<<4) | mCsrcLen;
    rtpHeader->secondByte = (mMarker<<7) | mPayloadType;                            
    // rtpHeader->seq = mSeq;
    // rtpHeader->timestamp = mTimestamp;
    // rtpHeader->ssrc = mSsrc;
    rtpHeader->seq = htons(mSeq);
    rtpHeader->timestamp = htonl(mTimestamp);
    rtpHeader->ssrc = htonl(mSsrc);
    if(mSessionSendPacketCallBack){
        mSessionSendPacketCallBack(rtpPacket);
    }
}