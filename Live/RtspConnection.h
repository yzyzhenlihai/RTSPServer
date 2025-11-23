#pragma once
#include"TcpConnection.h"

#include<string>
#include<cstring>
#include"MediaSession.h"
#include"RtpInstance.h"
class RtspServer;
class RtspConnection : public TcpConnection
{

public:
    enum Method{
        OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN,
        NONE
    };
    static RtspConnection* createNew(RtspServer* rtspServer, int clientFd);
    RtspConnection(RtspServer* rtspServer, int clientFd);
    ~RtspConnection();

protected:
    virtual void handleReadBytes();
private:
    bool parseRequest();
    bool parseRequest1(const char* begin, const char* end); //解析请求行
    bool parseRequest2(const char* begin, const char* end);
    bool parseCSeq(std::string message);
    bool parseDescribe(std::string message);
    bool parseSetup(std::string message);
    bool parsePlay(std::string message);

    bool handleOptions();
    bool handleDescribe();
    bool handleSetup();
    bool handlePlay();
    bool handleTeardown();

    int sendMessage(const char* data, int len);
    void getPeerIp(int clientFd, std::string& ip);
    bool createRtpRtcpOverUdp(MediaSession::TrackId trackId);
    bool createRtpRtcpOverTcp(MediaSession::TrackId trackId);
private:
    /* data */
    bool mIsRtpOverTcp;
    Method mMethod;
    std::string mUrl;
    std::string mVersion;
    std::string mSuffix; // 保存资源路径 /test /test/track0  /test/track1 不同的RTSP请求都是不同的，会更新
    std::string mStreamPrefix; //流前缀，一般为track
    uint32_t mCSeq;

    MediaSession::TrackId mTrackId;
    uint8_t mRtpChannel;
    uint16_t mPeerRtpPort;//客户端收发RTP端口号
    uint16_t mPeerRtcpPort;
    uint32_t mSessionId;
    RtspServer* mRtspServer;

    RtpInstance* mRtpInstances[MediaSession::MEDIA_MAX_TRACK_NUM];
    RtcpInstance* mRtcpInstances[MediaSession::MEDIA_MAX_TRACK_NUM];
    std::string mPeerIp;
};

