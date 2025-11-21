#pragma once
#include<string>
#include<sstream>
#include<list>
#include"Sink.h"
#include"RtpInstance.h"
#include"Rtp.h"
class MediaSession
{

public:
    enum TrackId{
        TrackIdNone=-1,
        TrackId0=0,
        TrackId1=1,
    };
    class Track{
    public:
        Sink* mSink;
        int mTrackId;
        bool mIsAlive;
        std::list<RtpInstance*> mRtpInstances; //记录连接到所有这个会话的客户端
    };
    static const int MEDIA_MAX_TRACK_NUM = 2;
    static MediaSession* createNew(std::string sessionName,std::string ip);
    MediaSession(std::string sessionName,std::string ip);
    ~MediaSession();
public:
    
    std::string name(){return mSessionName;}
    std::string generateSDPDescription();
    bool isStartMuticast(){return mIsStartMulticast;}
    bool addSink(MediaSession::TrackId trackId, Sink* sink);  //添加数据生产者
    bool addRtpInstance(MediaSession::TrackId trackId, RtpInstance* rtpInstance); //添加数据消费者

private:

    /* data */
    std::string mSessionName;
    std::string mSdp;
    std::string mIp;
    bool mIsStartMulticast; //是否为多播
    Track mTracks[MEDIA_MAX_TRACK_NUM]; // 保存轨道信息
    

private:
    MediaSession::Track* getTrack(MediaSession::TrackId trackId);
    void handleSendRtpPacket(Track* track, RtpPacket* rtpPacket);
};

