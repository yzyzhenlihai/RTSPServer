#include"MediaSession.h"
#include"SocketsOps.h"


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

    for(int i=0;i<MEDIA_MAX_TRACK_NUM;i++){
        mMulticastRtpInstances[i]=nullptr;
        mMulticastRtcpInstances[i]=nullptr;
    }
    
}


MediaSession::~MediaSession()
{
    for(int i=0;i<MEDIA_MAX_TRACK_NUM;i++){
        if(mMulticastRtpInstances[i]){
            this->removeRtpInstance(mMulticastRtpInstances[i]);
            delete mMulticastRtpInstances[i];
        }
        if(mMulticastRtcpInstances[i]){
            delete mMulticastRtcpInstances[i];
        }
    }
    //清除每个轨道关联的sink
    for(int i=0;i<MEDIA_MAX_TRACK_NUM;i++){
        if(mTracks[i].mIsAlive){
            Sink* sink = mTracks[i].mSink;
            delete sink;
        }
    }

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
            port = this->getMulticastDestRtpPort((TrackId)i); 
        }
        /*
        “责任分离”设计，MediaSession不关心媒体的具体编码信息，只负责组合信息
        */
        ss<<mTracks[i].mSink->getMediaDescription(port).c_str()<<"\r\n"; // m=video 0 RTP/AVP 96\r\n

        if(isStartMuticast()){
            //TODO 多播逻辑
            ss<<"c=IN IP4 "<<this->getMulticastAddr().c_str()<<"/255"<<"\r\n";
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

//由Session负责发送RTP包，媒体数据包
void MediaSession::handleSendRtpPacket(Track* track, RtpPacket* rtpPacket){

    for(std::list<RtpInstance*>::iterator it = track->mRtpInstances.begin();it!=track->mRtpInstances.end();it++){
        RtpInstance* rtpInstance = *it;
        if(rtpInstance->alive()){
            rtpInstance->send(rtpPacket);
        }
    }
}

bool MediaSession::startMulticast(){
    /*
    实现多播功能
    随机生成一个多播地址和一组端口号
     */
    // 随机生成多播地址
    
    struct sockaddr_in addr={0};
    uint32_t range = 0xEFFFFFFF - 0xEF000100; 
    addr.sin_addr.s_addr = htonl(0xEF000100 + (rand()) % range); // 239.0.1.0 ~ 239.255.255.255
    char buf[64]={0};
    mMulticastAddr = inet_ntop(AF_INET, &addr.sin_addr, buf, sizeof(buf));
    int rtpSockfd1,rtcpSockfd1;
    int rtpSockfd2,rtcpSockfd2;
    uint16_t rtpPort1, rtcpPort1;
    uint16_t rtpPort2, rtcpPort2;
    // 创建UDP套接字 每个轨道对应两个UDP套接字
    rtpSockfd1 = sockets::createUdpSock();
    rtcpSockfd1 = sockets::createUdpSock();
    rtpSockfd2 = sockets::createUdpSock();
    rtcpSockfd2 = sockets::createUdpSock();

    // 随机生成多播端口号
    uint16_t port = rand() & 0xFFFE; //确保是偶数
    if(port < 10000) port += 10000;
    rtpPort1 = port;
    rtcpPort1 = port + 1;
    rtpPort2 = port + 2;
    rtcpPort2 = port + 3;
    
    // 绑定端口号
    sockets::bind(rtpSockfd1, "0.0.0.0", rtpPort1);
    sockets::bind(rtpSockfd2, "0.0.0.0", rtpPort2);
    sockets::bind(rtcpSockfd1, "0.0.0.0", rtcpPort1);
    sockets::bind(rtcpSockfd2, "0.0.0.0", rtcpPort2);

    // 设置多播TTL和Loop
    sockets::setMulticastTtl(rtpSockfd1, 255);
    sockets::setMulticastLoop(rtpSockfd1, 1);
    sockets::setMulticastTtl(rtpSockfd2, 255);
    sockets::setMulticastLoop(rtpSockfd2, 1);
    
    // 创建Rtp和Rtcp实例
    mMulticastRtpInstances[TrackId0] = RtpInstance::createNewOverUdp(rtpSockfd1, rtpPort1, mMulticastAddr, rtpPort1);
    mMulticastRtpInstances[TrackId1] = RtpInstance::createNewOverUdp(rtpSockfd2, rtpPort2, mMulticastAddr, rtpPort2);

    mMulticastRtcpInstances[TrackId0] = RtcpInstance::createNewOverUdp(rtcpSockfd1, rtcpPort1, mMulticastAddr, rtcpPort1);
    mMulticastRtcpInstances[TrackId1] = RtcpInstance::createNewOverUdp(rtcpSockfd2, rtcpPort2, mMulticastAddr, rtcpPort2);

    // 向轨道中添加实例
    this->addRtpInstance(TrackId0, mMulticastRtpInstances[TrackId0]);
    this->addRtpInstance(TrackId1, mMulticastRtpInstances[TrackId1]);

    mMulticastRtpInstances[TrackId0]->setAlive(true);
    mMulticastRtpInstances[TrackId1]->setAlive(true);

    mIsStartMulticast = true; // 后续SETUP会判断是单播逻辑还是多播逻辑

    LOGI("startMuticast, muticastIp = %s", mMulticastAddr.c_str());
    return true;
}

uint16_t MediaSession::getMulticastDestRtpPort(TrackId trackId){
    if(trackId>MEDIA_MAX_TRACK_NUM || !mMulticastRtpInstances[trackId]) return -1;

    return mMulticastRtpInstances[trackId]->getPeerPort();

}

// 删掉某个Rtp实例
bool MediaSession::removeRtpInstance(RtpInstance* rtpInstance){
    for(int i=0;i<MEDIA_MAX_TRACK_NUM;i++){
        if(mTracks[i].mIsAlive == false) continue;
        std::list<RtpInstance*>::iterator it = std::find(mTracks[i].mRtpInstances.begin(), mTracks[i].mRtpInstances.end(), rtpInstance);
        if(it != mTracks[i].mRtpInstances.end()){
            //找到这个Rtp实例
            mTracks[i].mRtpInstances.erase(it);
            return true;
        }
    }
    return false;
}
   