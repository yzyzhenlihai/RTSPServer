# pragma once
#include"Sink.h"
class AACFileSink : public Sink{

public:
    static AACFileSink* createNew(UsageEnvironment* env, MediaSource* source);
    AACFileSink(UsageEnvironment* env, MediaSource* source, int payloadType);
    ~AACFileSink();

protected:

    virtual std::string getMediaDescription(uint16_t port); 
    virtual std::string getAttribute();
    virtual void sendFrame(MediaFrame* frame);


private:
    int mSampleRate; //采样率
    int mChannels;  //通道数
    int mFps;
};