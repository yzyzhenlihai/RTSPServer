#pragma once
#include"Sink.h"

class H264FileSink:public Sink
{

public:
    
    static H264FileSink* createNew(UsageEnvironment* env, MediaSource* source);
    H264FileSink(UsageEnvironment* env, MediaSource* source);
    ~H264FileSink();

public:
    virtual std::string getMediaDescription(uint16_t port);
    virtual std::string getAttribute();
    virtual void sendFrame(MediaFrame* frame);
private:
    
    int mClockRate; // 时钟频率
    int mFps;
};

