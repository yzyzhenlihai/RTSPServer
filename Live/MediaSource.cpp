#include"MediaSource.h"

MediaFrame::MediaFrame(){

    mFrameBuf.resize(FRAME_MAX_SIZE);

}
MediaFrame::~MediaFrame(){

}

/*
MediaSource defination
*/
MediaSource::MediaSource(UsageEnvironment* env):
    mEnv(env),
    mFps(0)
{
    for(int i=0;i<DEFAULT_FRAME_NUM;i++){
        mFrameInputQueue.push(&mFrames[i]);
    }
    mTask.setTaskCallback(std::bind(&MediaSource::handleTask,this));
}

MediaSource::~MediaSource()
{
    LOGI("~MedieSource");
}

//从输出队列中取出帧
MediaFrame* MediaSource::getFrameFromOutputQueue(){
    std::unique_lock<std::mutex> locker(mMtx);
    if(mFrameOutputQueue.empty()){
        return nullptr;
    }
    MediaFrame* frame = mFrameOutputQueue.front();
    mFrameOutputQueue.pop();

    return frame;
}

void MediaSource::putFrameToInputQueue(MediaFrame* frame){
    std::unique_lock<std::mutex> locker(mMtx);
    mFrameInputQueue.push(frame);
    //TODO 向线程池添加任务
    mEnv->threadPool()->addTask(mTask);
}
