#pragma once
#include"MediaSource.h"

class H264FileMediaSource:public MediaSource
{

public:
    static H264FileMediaSource* createNew(UsageEnvironment* env, std::string filePath);
    H264FileMediaSource(UsageEnvironment* env, std::string filePath);
    ~H264FileMediaSource();

protected:
    virtual void handleTask();

private:
    int getNALUFromH264File(std::vector<uint8_t>& frame, int offset);
    int checkStartCode(uint8_t* frame, int startPos); //查看当前起始码长度
    int getNextStartCode(std::vector<uint8_t> &frame, int startPos);//查看下一个起始码位置

private:
    std::vector<uint8_t> mFileBuffer;// 存文件中所有的数据，每次从中读取一个NALU
    
    FILE* mFile;
    long long mFileSize;
    int mCurOffset;//记录当前偏置，也是nalu的其起始位置
    int mCnt;

};

