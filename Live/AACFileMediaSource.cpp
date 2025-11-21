#include"AACFileMediaSource.h"
#include"Log.h"
AACFileMediaSource* AACFileMediaSource::createNew(UsageEnvironment* env, std::string filePath){
    return new AACFileMediaSource(env, filePath);
}

AACFileMediaSource::AACFileMediaSource(UsageEnvironment* env, std::string filePath):
    MediaSource(env)
{
    mSourceName = filePath;
    mFile = fopen(filePath.c_str(), "rb");
    setFps(43); //设置帧率
    if(!mFile){
        LOGE("AACFile fopen error");
        return;
    }
    // fseek(mFile, 0, SEEK_END); //将文件指针移动到最后
    // mFileSize = ftell(mFile); //返回当前文件指针所在的位置
    // fseek(mFile, 0, SEEK_SET); //移动文件指针到最前
    // mFileBuffer.resize(mFileSize);
    // if(fread(mFileBuffer.data(), 1, mFileSize, mFile) != mFileSize){
    //     LOGE("aac file fread error");
    //     return;
    // }

    // 执行读取帧的任务
    for(int i=0;i<DEFAULT_FRAME_NUM;i++){
        env->threadPool()->addTask(mTask);
    }
    
}

AACFileMediaSource::~AACFileMediaSource(){

}


// 执行任务（从文件中解析出音频帧）
void AACFileMediaSource::handleTask(){
    std::unique_lock<std::mutex> locker(mMtx);
    if(mFrameInputQueue.empty()) return;
    MediaFrame* frame = mFrameInputQueue.front();
    frame->mFrameSize = getFrameFromAACFile(frame->mFrameBuf.data(), MediaFrame::FRAME_MAX_SIZE);
    // 把帧加入队列
    mFrameInputQueue.pop();
    mFrameOutputQueue.push(frame);
}

int AACFileMediaSource::getFrameFromAACFile(uint8_t* buf, int size){
    uint8_t tmpBuf[7];
    int rSize = fread(tmpBuf, 1, 7, mFile); // 读取ADTS头
    if(rSize == 0){
        if(feof(mFile)){
            LOGI("AACFileMediaSource reach file end");
            return -1;
        }else if(ferror(mFile)){
            LOGE("AACFileMediaSource fread error");
            return -1;
        }
    }else{
        if(!setADTSHeader(tmpBuf, rSize)){
            LOGE("setADTSHeader error");
            return -1;
        }
        int aacOriginLength = mAdtsHeader.aacFrameLength - 7;
        //LOGI("aacSize = %d", aacOriginLength);
        if(aacOriginLength > size) return -1; //超过最大长度为异常
        rSize = fread(buf, 1, aacOriginLength, mFile);
        if(rSize<0){
            LOGE("aacOriginLength fread error");
            return -1;
        }
    }
    return mAdtsHeader.aacFrameLength-7; // 返回读到的帧大小
    
}


bool AACFileMediaSource::setADTSHeader(uint8_t* in, int size){
    if(size < 7){
        LOGE("SetADTSHeader size < 7 error");
        return false;
    }
    if((in[0] == 0xff) && (in[1] & 0xf0) == 0xf0){
        mAdtsHeader.id = ((unsigned int) in[1] & 0x08) >> 3;
        mAdtsHeader.layer = ((unsigned int) in[1] & 0x06) >> 1;
        mAdtsHeader.protectionAbsent = (unsigned int) in[1] & 0x01;
        mAdtsHeader.profile = ((unsigned int) in[2] & 0xc0) >> 6;
        mAdtsHeader.samplingFreqIndex = ((unsigned int) in[2] & 0x3c) >> 2;
        mAdtsHeader.privateBit = ((unsigned int) in[2] & 0x02) >> 1;
        mAdtsHeader.channelCfg = ((((unsigned int) in[2] & 0x01) << 2) | (((unsigned int) in[3] & 0xc0) >> 6));
        mAdtsHeader.originalCopy = ((unsigned int) in[3] & 0x20) >> 5;
        mAdtsHeader.home = ((unsigned int) in[3] & 0x10) >> 4;
        mAdtsHeader.copyrightIdentificationBit = ((unsigned int) in[3] & 0x08) >> 3;
        mAdtsHeader.copyrightIdentificationStart = (unsigned int) in[3] & 0x04 >> 2;
        mAdtsHeader.aacFrameLength = (((((unsigned int) in[3]) & 0x03) << 11) |
                                (((unsigned int)in[4] & 0xFF) << 3) |
                                    ((unsigned int)in[5] & 0xE0) >> 5) ;
        mAdtsHeader.adtsBufferFullness = (((unsigned int) in[5] & 0x1f) << 6 |
                                        ((unsigned int) in[6] & 0xfc) >> 2);
        mAdtsHeader.numberOfRawDataBlockInFrame = ((unsigned int) in[6] & 0x03);

        return true;
    }else{
        LOGE("SetADTSHeader syncword error");
        return false;
    }

   
}

