#include"H264FileSink.h"
#include"Rtp.h"
#include"MediaSource.h"
#include<cstring>

H264FileSink* H264FileSink::createNew(UsageEnvironment* env, MediaSource* source){
    if(source == nullptr) return nullptr;
    return new H264FileSink(env, source);
}

H264FileSink::H264FileSink(UsageEnvironment* env, MediaSource* source):
    Sink(env, source, RtpPacket::RTP_PAYLOAD_TYPE_H264),
    mClockRate(90000),
    mFps(source->getFps())
{
    LOGI("H264FileSink");
    runEvery(1000/mFps); //假设帧率是25fps，1秒发送25帧，发送1帧的时间间隔是0.04s，也就是40ms
}

H264FileSink::~H264FileSink()
{
    
}

std::string H264FileSink::getMediaDescription(uint16_t port){
    char buf[64]={0};
    sprintf(buf, "m=video %hu RTP/AVP %d", port, mPayloadType);
    return std::string(buf);
}

std::string H264FileSink::getAttribute(){
    char buf[64]={0};
    sprintf(buf, "a=rtpmap:%d H264/%d\r\n", mPayloadType, mClockRate);
    sprintf(buf + strlen(buf), "a=framerate:%d", mFps);
    return std::string(buf);
}

//子类实现sendFrame的打包并发送逻辑
void H264FileSink::sendFrame(MediaFrame* frame){
    
    uint8_t naluHeader = frame->mFrameBuf[0]; // 获得NALU头
    int naluSize = frame->mFrameSize;
    if(naluSize <= RTP_MAX_PKT_SIZE){
        if(!mRtpPacket.SetPayloadData(frame->mFrameBuf.data(), naluSize)){
            LOGE("SetPayloadData error");
            return;
        }
        //TODO 发送RTP包
        sendRtpPacket(&mRtpPacket);
        mSeq++;
        //SPS和PPS帧，时间戳不需要变化
        if((!((naluHeader & 0x1F) == 7 || (naluHeader & 0x1F) == 8))){
            mTimestamp += mClockRate / mFps;
        }
    }else{
        //*  0                   1                   2
        //*  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
        //* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        //* | FU indicator  |   FU header   |   FU payload   ...  |
        //* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        //*     FU Indicator
        //*    0 1 2 3 4 5 6 7
        //*   +-+-+-+-+-+-+-+-+
        //*   |F|NRI|  Type   |
        //*   +---------------+

        //*      FU Header
        //*    0 1 2 3 4 5 6 7
        //*   +-+-+-+-+-+-+-+-+
        //*   |S|E|R|  Type   |
        //*   +---------------+
        //分片NALU模式
        //1. 计算完整包的个数
        //2. 构造分片NALU的RTP包头
        //3. 发送分片NALU的RTP包
        int fullPacketCnt = naluSize / RTP_MAX_PKT_SIZE;
        int lastPacketSize = naluSize % RTP_MAX_PKT_SIZE;
        for(int i=0;i<fullPacketCnt;i++){
            uint8_t fuIndicator = (naluHeader & 0xE0) | 28; //FU-A类型
            uint8_t fuHeader = 0;
            if(i==0){
                //第一个分片
                fuHeader = 0x80 | (naluHeader & 0x1F); //S=1 E=0 R=0
            }else if(i==fullPacketCnt-1 && lastPacketSize==0){
                //最后一个分片
                fuHeader = 0x40 | (naluHeader & 0x1F); //S=0 E=1 R=0
            }else{
                //中间分片
                fuHeader = (naluHeader & 0x1F); //S=0
            }
            //构造FU-A RTP包的负载
            std::vector<uint8_t> fullPayload(RTP_MAX_PKT_SIZE + 2);
            fullPayload[0] = fuIndicator;
            fullPayload[1] = fuHeader;
            memcpy(fullPayload.data()+2, frame->mFrameBuf.data()+1+i*RTP_MAX_PKT_SIZE, RTP_MAX_PKT_SIZE); //+1表示跳过naluHeader
            mRtpPacket.SetPayloadData(fullPayload.data(), RTP_MAX_PKT_SIZE + 2);
            //TODO 发送RTP包
            sendRtpPacket(&mRtpPacket);
            //修改RTP包的序列号
            mSeq++;

        }//for
        //发送最后一个分片
        if(lastPacketSize>0){
            uint8_t fuIndicator = (naluHeader & 0xE0) | 28; //FU-A类型
            uint8_t fuHeader = 0x40 | (naluHeader & 0x1F); //S=0 E=1 R=0
            std::vector<uint8_t> lastPayload(lastPacketSize + 2);
            lastPayload[0] = fuIndicator;
            lastPayload[1] = fuHeader;
            memcpy(lastPayload.data()+2, frame->mFrameBuf.data()+1+fullPacketCnt*RTP_MAX_PKT_SIZE, lastPacketSize);
            mRtpPacket.SetPayloadData(lastPayload.data(), lastPacketSize + 2);
            //TODO 发送RTP包
            sendRtpPacket(&mRtpPacket);
            //修改RTP包的序列号
            mSeq++;
        }
        mTimestamp += mClockRate / mFps;
    }
    
}   