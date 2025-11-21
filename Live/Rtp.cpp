#include"Rtp.h"
#include<cstring>


RtpPacket::RtpPacket(int maxPayloadSize):
    mPayloadSize(0),
    mMaxPayloadSize(maxPayloadSize)
{

    size_t totalSize = RTP_HEADER_SIZE + maxPayloadSize;
    mBuffer.resize(totalSize);

}

RtpPacket::~RtpPacket(){

}
void RtpPacket::InitRtpHeader(uint8_t version, uint8_t padding, uint8_t extension, 
                            uint8_t csrclen, uint8_t marker, uint8_t payLoadtype, uint16_t seq,
                            uint32_t timestamp, uint32_t ssrc){
    
    RtpHeader* rtpHeader = (RtpHeader*)mBuffer.data();
    rtpHeader->firstByte = (version<<6) | (padding<<5) | (extension<<4) | csrclen;
    rtpHeader->secondByte = (marker<<7) | payLoadtype;                            
    rtpHeader->seq = seq;
    rtpHeader->timestamp = timestamp;
    rtpHeader->ssrc = ssrc;

}

bool RtpPacket::SetPayloadData(const uint8_t* data, size_t size){
    if(size > mMaxPayloadSize) return false;
    mPayloadSize = size;
    uint8_t* payloadPtr = mBuffer.data() + RTP_HEADER_SIZE;
    memcpy(payloadPtr, data, size);
    return true;
}

uint8_t* RtpPacket::data(){
    return mBuffer.data();
}

int RtpPacket::size(){
    return mPayloadSize + RTP_HEADER_SIZE;
}

