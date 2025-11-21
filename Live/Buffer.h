#pragma once
#include<vector>
#include<sys/socket.h>
#include<assert.h>
#include<string>
#include<algorithm>
#include<iostream>
#include"Log.h"
#include"SocketsOps.h"
class Buffer
{

public:
    static constexpr int initialSize = 2048;
    static constexpr const char* CRLF = "\r\n";
    explicit Buffer(int bufferSize=initialSize);
    ~Buffer() = default;
    //禁用拷贝函数和赋值函数
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;
    //允许移动
    Buffer(Buffer&&) noexcept = default;
    Buffer& operator=(Buffer&&) noexcept = default;
public:

    int read(int fd);
    int readableBytes() const noexcept{return mWriteIndex - mReadIndex;}
    int write(int fd);
    int writableBytes() const noexcept{return mBuffer.size() - mWriteIndex;}
    char* beginWrite(){return mBuffer.data() + mWriteIndex;}
    void append(const char* data, int len);
    /* 确保有足够的空间 */
	void ensureWritableBytes(int len);
    void makeSpace(int len);
    void showBuffer();
    char* peek(){return begin() + mReadIndex;}

    const char* findCRLF();
    const char* findLastCRLF();//找到最后一个\r\n
    void retrieveAll();
    void retrieveUtil(const char* end);
    void retrieve(int len);
private:
    char* begin(){return mBuffer.data();}
private:   
    
    std::vector<char> mBuffer;
    int mBufferSize;
    int mReadIndex;
    int mWriteIndex;
    
};


