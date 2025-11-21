#include"H264FileMediaSource.h"
#include<cstring>
#include<unistd.h>
H264FileMediaSource* H264FileMediaSource::createNew(UsageEnvironment* env, std::string filePath){
    
    return new H264FileMediaSource(env, filePath);
}

H264FileMediaSource::H264FileMediaSource(UsageEnvironment* env, std::string filePath):
    MediaSource(env),
    mCurOffset(0),
    mCnt(0)
{
    mSourceName = filePath;
    setFps(25);
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    LOGI("Current Working Directory is %s\n", cwd);
    //直接读取整个文件
    mFile = fopen(mSourceName.c_str(), "rb");
    if(mFile == nullptr){
        LOGE("h264 file fopen error");
        return;
    }
    fseek(mFile, 0, SEEK_END); //将文件指针移动到最后
    mFileSize = ftell(mFile); //返回当前文件指针所在的位置
    fseek(mFile, 0, SEEK_SET); //移动文件指针到最前
    mFileBuffer.resize(mFileSize);
    if(fread(mFileBuffer.data(), 1, mFileSize, mFile) != mFileSize){
        LOGE("h264 file fread error");
        return;
    }
    //执行读取帧的任务
    for(int i=0;i<DEFAULT_FRAME_NUM;i++){
        mEnv->threadPool()->addTask(mTask);
    }

}

H264FileMediaSource::~H264FileMediaSource()
{
    fclose(mFile);
}

int H264FileMediaSource::getNALUFromH264File(std::vector<uint8_t>& frame, int offset){
    
    int startCode = checkStartCode(mFileBuffer.data(), offset);
    int frameSize = mFileSize - offset - startCode; //假设这是最后一个NALU
    if(startCode == 3 || startCode == 4){
        //合理起始码
        //获得下一个起始码的位置
        int nextStartCodePos = getNextStartCode(mFileBuffer, offset+startCode);
        //计算NALU的大小
        if(nextStartCodePos != -1){
            frameSize = nextStartCodePos - startCode - offset;
        }
        memcpy(frame.data(), mFileBuffer.data()+offset+startCode, frameSize);

    }else{
        LOGE("start code = %d error", startCode);
    }

    return frameSize;
}

void H264FileMediaSource::handleTask(){
    //处理读取NALU帧任务
    std::unique_lock<std::mutex> locker(mMtx);
    if(mFrameInputQueue.empty()) return;
    MediaFrame* frame = mFrameInputQueue.front();
    int cnt=0;
    while(mCurOffset<mFileSize){
        int naluSize = getNALUFromH264File(frame->mFrameBuf, mCurOffset);
        int startCode = checkStartCode(mFileBuffer.data(), mCurOffset);
        //LOGI("cnt = %d startCode = %d naluSize = %d mCurOffset = %d",mCnt, startCode, naluSize, mCurOffset);
        mCurOffset = mCurOffset + startCode + naluSize; //修改偏置
        mCnt++;
        frame->mFrameSize = naluSize;
        
        uint8_t naluType = frame->mFrameBuf[0] & 0x1F;
        if(naluType == 0x09){
            //访问分割单元符，直接丢弃
            continue;
        }else if(naluType == 0x07 || naluType == 0x08){
            break;
        }else{
            break;
        }
        
    }
    mFrameInputQueue.pop();
    mFrameOutputQueue.push(frame);
}

int H264FileMediaSource::checkStartCode(uint8_t* frame, int startPos){
    int startCode = 0;
    if(frame[startPos]==0 && frame[startPos+1]==0 && frame[startPos+2]==0 && frame[startPos+3]==1){
        startCode = 4;
    }
    if(frame[startPos]==0 && frame[startPos+1]==0 && frame[startPos+2]==1){
        startCode = 3;
    }
    return startCode;
}

int H264FileMediaSource::getNextStartCode(std::vector<uint8_t> &frame, int startPos){
    int pos = startPos;
    int n = frame.size();
    if(startPos>=frame.size()) return -1;
    int nextStartCodePos = -1;
    for(int i=pos;i<n-4;i++){
        int startCode = checkStartCode(frame.data(), i);
        if(startCode>0){
            //找到下一个起始码的起始位置
            nextStartCodePos = i;
            break;
        }

    }
    return nextStartCodePos; //返回-1表示没有找到下一个起始码
}