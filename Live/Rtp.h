#pragma once
#include<stdint.h>
#include<vector>
#include<memory>
 /*
  *    0                   1                   2                   3
  *    7 6 5 4 3 2 1 0|7 6 5 4 3 2 1 0|7 6 5 4 3 2 1 0|7 6 5 4 3 2 1 0
  *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  *   |V=2|P|X|  CC   |M|     PT      |       sequence number         |
  *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  *   |                           timestamp                           |
  *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  *   |           synchronization source (SSRC) identifier            |
  *   +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
  *   |            contributing source (CSRC) identifiers             |
  *   :                             ....                              :
  *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  *
*/

#pragma pack(push,1) //来确保编译器不会添加任何填充字节


struct RtpHeader{
    //byte_0 version, padding, extension, csrcLen
    uint8_t firstByte;
    //byte_1 marker, payLoadType
    uint8_t secondByte;
    
    uint16_t seq;
    uint32_t timestamp;
    uint32_t ssrc;
};

#pragma pack(pop) //恢复默认的对齐方式

class RtpPacket{
public:
    static const int RTP_VERSION = 2;
    static const int RTP_PAYLOAD_TYPE_H264 = 96;
    static const int RTP_PATLOAD_TYPE_AAC = 97;

    RtpPacket(int size);
    ~RtpPacket();
    //初始化rtp头
    void InitRtpHeader(uint8_t version, uint8_t padding, uint8_t extension, 
        uint8_t csrclen, uint8_t marker, uint8_t payLoadtype, uint16_t seq,
        uint32_t timestamp, uint32_t ssrc);
    
    bool SetPayloadData(const uint8_t* data, size_t size);
    
    uint8_t* data();
    int size();
private:

    const int RTP_HEADER_SIZE = 12;
    int mPayloadSize;
    int mMaxPayloadSize;
    std::vector<uint8_t> mBuffer;  //一整个rtp包

};