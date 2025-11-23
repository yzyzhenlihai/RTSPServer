#include "AACFileSink.h"
#include"Log.h"
#include<sstream>
#include<cstring>
#include"MediaSource.h"
AACFileSink* AACFileSink::createNew(UsageEnvironment* env, MediaSource* source){
    return new AACFileSink(env, source, RtpPacket::RTP_PATLOAD_TYPE_AAC);
}

AACFileSink::AACFileSink(UsageEnvironment* env, MediaSource* source, int payloadType):
    Sink(env, source, payloadType),
    mSampleRate(44100),
    mChannels(2),
    mFps(source->getFps())
{
    LOGI("AACFileSink");
    mMarker = 1;
    runEvery(1000/mFps); // 设置定时事件
}

AACFileSink::~AACFileSink(){

    LOGI("~AACFileSink()");
}

// 返回媒体级描述信息
std::string AACFileSink::getMediaDescription(uint16_t port){
    std::stringstream ss;
    ss<<"m=audio "<<port<<" RTP/AVP "<<static_cast<int>(mPayloadType);
    return ss.str();
}

// 获得音频属性
static uint32_t AACSampleRate[16] =
{
    97000, 88200, 64000, 48000,
    44100, 32000, 24000, 22050,
    16000, 12000, 11025, 8000,
    7350, 0, 0, 0 /*reserved */
};// 每个采样频率对应一个索引，用于后续计算config(将profile、采样率和通道数三个信息压缩到2个字节中)
std::string AACFileSink::getAttribute(){
    std::stringstream ss;
    ss<<"a=rtpmap:"<<static_cast<int>(mPayloadType)<<" mpeg4-generic/"<<mSampleRate<<"/"<<mChannels<<"\r\n";
    uint8_t index = 0;
    for(index=0;index<16;index++){
        if(AACSampleRate[index] == mSampleRate) break;
    }
    if(index == 16) return "";
    uint8_t profile = 1; // AAC-LC Low Complexity 兼容性最好的AAC配置
    char configStr[10]={0};
    sprintf(configStr, "%02x%02x", (uint8_t)((profile+1)<<3)|(index>>1),(uint8_t)((index<<7) | (mChannels<<3)));
    ss<<"a=fmtp:"<<static_cast<int>(mPayloadType)<<" profile-level-id=1;";
    ss<<"mode=AAC-hbr;";
    ss<<"sizelength=13;indexlength=3;indexdeltalength=3;";
    ss<<"config="<<configStr;

    return ss.str();
}

// 发送音频帧
void AACFileSink::sendFrame(MediaFrame* frame){
    // 构造RTP负载
    int aacSize = frame->mFrameSize;
    //LOGI("AACFileSink sendFrame aacSize=%d", aacSize);
    //std::vector<uint8_t> fullPayLoad(4+aacSize);
    char fullPayLoad[2048]; //这种频繁使用的函数，使用vector会导致堆内存分配的重复，降低服务器性能
    fullPayLoad[0]=0x00;
    fullPayLoad[1]=0x10;
    
    fullPayLoad[2] = (aacSize>>5) & 0xFF; //计算高8位
    fullPayLoad[3] = ((aacSize & 0x1F)<<3);
    memcpy(fullPayLoad+4, frame->mFrameBuf.data(), aacSize);
    mRtpPacket.SetPayloadData((uint8_t*)fullPayLoad, 4+aacSize);
    sendRtpPacket(&mRtpPacket);
    mSeq++; 
    //mTimestamp += mSampleRate / mFps; // 每帧的时间戳增量 = 采样率/帧率  这里的采样频率对标视频帧的时钟频率
    mTimestamp += 1024;
}