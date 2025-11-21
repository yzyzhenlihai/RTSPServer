#include"MediaSession.h"



MediaSession* MediaSession::createNew(std::string sessionName,std::string ip){
    return new MediaSession(sessionName,ip);
}

MediaSession::MediaSession(std::string sessionName,std::string ip):
    mSessionName(sessionName),
    mIp(ip),
    mIsStartMulticast(false)
{
    //初始化轨道信息
    mTracks[0].mTrackId = TrackId0;
    mTracks[0].mIsAlive = false;
    mTracks[1].mTrackId = TrackId1;
    mTracks[1].mIsAlive = false;


}


MediaSession::~MediaSession()
{
}

std::string MediaSession::generateSDPDescription(){
    if(!mSdp.empty()){
        //表示已经生成过SDP，直接返回
        return mSdp;
    }
    //生成会话级（Session-Level）信息
    std::stringstream ss;
    ss<<"v=0\r\n";
    ss<<"o=- 9"<<time(nullptr)<<" 1 IN IP4 "<<mIp.c_str()<<"\r\n";
    ss<<"t=0 0\r\n";
    ss<<"a=control:*\r\n";
    ss<<"a=type:broadcast\r\n";

    if(isStartMuticast()){
        //如果是多播模式 需要额外添加一行多播属性
        ss<<"a=rtcp-unicast: reflection\r\n";
    }
    //循环构建媒体级描述信息（Media-Level）
    for(int i=0;i<MEDIA_MAX_TRACK_NUM;i++){
        uint16_t port = 0;
        if(!mTracks[i].mIsAlive) continue;

        if(isStartMuticast()){
            //TODO 多播逻辑
        }
        /*
        “责任分离”设计，MediaSession不关心媒体的具体编码信息，只负责组合信息
        */
        ss<<mTracks[i].mSink->getMediaDescription(port).c_str()<<"\r\n"; // m=video 0 RTP/AVP 96\r\n

        if(isStartMuticast()){
            //TODO 多播逻辑
        }else{
            ss<<"c=IN IP4 0.0.0.0\r\n";
        }
        ss<<mTracks[i].mSink->getAttribute().c_str()<<"\r\n";
        ss<<"a=control:track"<<mTracks[i].mTrackId<<"\r\n";
    }
    mSdp = ss.str();
    return mSdp;
}

bool MediaSession::addSink(TrackId trackId, Sink* sink){
    Track* track = getTrack(trackId);
    if(!track) return false;
    track->mIsAlive = true;
    track->mSink = sink;

    //绑定发送RTP包的回调函数
    sink->setSessionPacketCallBack(std::bind(&MediaSession::handleSendRtpPacket, this, track, std::placeholders::_1));
    return true;
}

bool MediaSession::addRtpInstance(MediaSession::TrackId trackId, RtpInstance* rtpInstance){
    Track* track = getTrack(trackId);
    if(!track || !track->mIsAlive) return false;

    track->mRtpInstances.push_back(rtpInstance);
    return true;
}

MediaSession::Track* MediaSession::getTrack(MediaSession::TrackId trackId){
    //根据trackId 获得Track
    for(int i=0;i<MEDIA_MAX_TRACK_NUM;i++){
        if(mTracks[i].mTrackId == trackId){
            return &mTracks[i];
        }
    }
    return nullptr;
}

//发送RTP包
void MediaSession::handleSendRtpPacket(Track* track, RtpPacket* rtpPacket){

    for(std::list<RtpInstance*>::iterator it = track->mRtpInstances.begin();it!=track->mRtpInstances.end();it++){
        RtpInstance* rtpInstance = *it;
        if(rtpInstance->alive()){
            rtpInstance->send(rtpPacket);
        }
    }
}
   