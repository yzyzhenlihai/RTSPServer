#pragma once 
#include"UsageEnvironment.h"
#include<string>
#include<queue>
#include<vector>
#include<mutex>
#include"ThreadPool.h"
//定义装数据的帧
class MediaFrame{
public:
    static const int FRAME_MAX_SIZE = 2000 * 100;
    MediaFrame();
    ~MediaFrame();

    std::vector<uint8_t> mFrameBuf;
    int mFrameSize;

private:
};
class MediaSource
{

public:
    static const int DEFAULT_FRAME_NUM = 4;//默认帧的输入和输出的队列的大小
    explicit MediaSource(UsageEnvironment* env);
    ~MediaSource();
public:
    int getFps() const{return mFps;}
    MediaFrame* getFrameFromOutputQueue();
    void putFrameToInputQueue(MediaFrame* frame);
protected:
    void setFps(int fps){mFps=fps;}
    virtual void handleTask() = 0; 
    /* data */
    UsageEnvironment* mEnv;
    int mFps;
    std::string mSourceName;

    MediaFrame mFrames[DEFAULT_FRAME_NUM];
    std::queue<MediaFrame*> mFrameInputQueue;
    std::queue<MediaFrame*> mFrameOutputQueue;

    std::mutex mMtx; // 互斥访问队列
    ThreadPool::Task mTask;
};


