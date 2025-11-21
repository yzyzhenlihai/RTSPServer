#include"Buffer.h"


Buffer::Buffer(int bufferSize):
    mBufferSize(bufferSize),
    mReadIndex(0),
    mWriteIndex(0)
{

}


int Buffer::read(int fd){

    char extraBuffer[65536];
    const int writable = writableBytes();

    int n = ::recv(fd, extraBuffer, sizeof(extraBuffer), 0);
    if(n<=0){
        return -1;
    }else if(n<=writable){
        //可以正常写
        std::copy(extraBuffer, extraBuffer+n, beginWrite());
        mWriteIndex+=n;
    }else{
        //需要扩容后再写
        std::copy(extraBuffer, extraBuffer+writable, beginWrite());
        mWriteIndex+=writable;
        append(extraBuffer+writable, n-writable);
    }
    
    return n;
}

int Buffer::write(int fd){

    int n = sockets::write(fd, peek(), readableBytes());
    return n;
}

void Buffer::append(const char* data, int len){

    ensureWritableBytes(len);
    assert(len<=writableBytes());
    std::copy(data, data+len, beginWrite());
    mWriteIndex+=len;

}

void Buffer::ensureWritableBytes(int len)
{
    if (writableBytes() < len)
    {
        makeSpace(len);
    }
    assert(writableBytes() >= len);
}

void Buffer::makeSpace(int len){

    if(mReadIndex + writableBytes()<len){
        //需要扩容
        mBuffer.resize(mWriteIndex + len);
    }else{
        //内部移动
        int readable = readableBytes();
        std::move(begin()+mReadIndex, begin()+mWriteIndex, begin());
        mReadIndex=0;
        mWriteIndex=readable;
    }
}

void Buffer::showBuffer(){
    std::string str = std::string(mBuffer.data()+mReadIndex, mBuffer.data()+mWriteIndex);
    
    LOGI("\nClient=========>Server\n%s", str.c_str());
}

const char* Buffer::findCRLF(){

    const char* crlf = std::search(begin()+mReadIndex, beginWrite(), CRLF, CRLF+2);
    return crlf == beginWrite()?nullptr:crlf;
}
const char* Buffer::findLastCRLF(){
    const char* clrf = std::find_end(peek(), beginWrite(), CRLF, CRLF+2);
    return clrf == beginWrite()?nullptr:clrf;
}

void Buffer::retrieveAll(){
    mReadIndex = 0;
    mWriteIndex = 0;
    mBuffer.clear();
}


void Buffer::retrieveUtil(const char* end){
    assert(peek()<=end);
    assert(end<=beginWrite());
    retrieve(end-peek());
}
void Buffer::retrieve(int len){

    assert(len<=readableBytes());
    if(len<readableBytes()){
        mReadIndex+=len;
    }else{
        retrieveAll();
    }
}