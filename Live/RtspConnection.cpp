#include"RtspConnection.h"
#include"RtspServer.h"
#include<sstream>
#include<iomanip>
//#include<format> //c++20
RtspConnection* RtspConnection::createNew(RtspServer* rtspServer, int clientFd){
    return new RtspConnection(rtspServer, clientFd);
}

RtspConnection::RtspConnection(RtspServer* rtspServer, int clientFd):
    TcpConnection(rtspServer->env(), clientFd),
    mIsRtpOverTcp(false),
    mTrackId(MediaSession::TrackIdNone),
    mRtspServer(rtspServer),
    mSessionId(rand()), //随机创建会话ID
    mStreamPrefix("track")
{
    
    //初始化RtpInstance
    for(int i=0;i<MediaSession::MEDIA_MAX_TRACK_NUM;i++){
        mRtpInstances[i]=nullptr;
        mRtcpInstances[i]=nullptr;
    }
    //获得与套接字关联的IP
    getPeerIp(clientFd, mPeerIp);
}

RtspConnection::~RtspConnection()
{
    LOGI("~RtspConnection()");
    for (int i = 0; i < MediaSession::MEDIA_MAX_TRACK_NUM; ++i)
    {
        if (mRtpInstances[i])
        {

            MediaSession* session = mRtspServer->getSessionManager()->getSession(mSuffix);

            if (!session) {
                session->removeRtpInstance(mRtpInstances[i]);
            }
            delete mRtpInstances[i];
        }

        if (mRtcpInstances[i])
        {
            delete mRtcpInstances[i];
        }
    }
}
void RtspConnection::getPeerIp(int clientFd, std::string& ip){
    struct sockaddr_in addr;
    socklen_t addrLen = sizeof(struct sockaddr_in);
    getpeername(clientFd, (struct sockaddr*)&addr, &addrLen);
    char buf[64]={0};
    inet_ntop(AF_INET, &addr.sin_addr.s_addr, buf, sizeof(buf));
    ip = std::string(buf);
}
// 解析RTSP请求
void RtspConnection::handleReadBytes(){

    /*
    处理RTSP请求的时候，为什么要先判断一下是不是基于Tcp，RTSP请求不都是TCP，只有RTP包的时候才需要区分

    mIsRtpOverTcp变量会在 RTSP 的 SETUP 请求处理阶段被设置。当客户端在 SETUP 请求的 Transport 头中指定了 interleaved 方式时，服务器就会将这个标志设为 true。
    如果这个标志是 false，意味着 RTP 数据将通过独立的 UDP 端口传输。那么这个 TCP 连接上收到的所有数据都必然是 RTSP 控制命令，代码会直接跳到第 4 步去解析。
    如果这个标志是 true，则意味着客户端和服务器协商好了，RTP/RTCP 包也会通过这个 TCP 连接发送。此时，服务器就需要一种方法来区分收到的到底是 RTSP 命令还是 RTP/RTCP 数据。
    */
    if(mIsRtpOverTcp){
        if(mReadBuffer.peek()[0] == '$'){
            return;
        }
    }
    
    if(!parseRequest()){
        LOGE("parseRequest error");
        handleDisconnect();
        return;
    }
    switch (mMethod)
    {
    case OPTIONS:
        if(!handleOptions()) handleDisconnect();
        break;
    case DESCRIBE:
        if(!handleDescribe()) handleDisconnect();
        break;
    case SETUP:
        if(!handleSetup()) handleDisconnect();
        break;
    case PLAY:
        if(!handlePlay()) handleDisconnect();
        break;
    case TEARDOWN:
    if(!handleTeardown()) handleDisconnect();
        break;  
    default:
        handleDisconnect();
        break;
    }
}

bool RtspConnection::parseRequest(){
    
    const char* crlf = mReadBuffer.findCRLF();
    if(crlf == nullptr){
        //当前请求行不完整
        mReadBuffer.retrieveAll();
        return false;
    }
    bool ret = parseRequest1(mReadBuffer.peek(), crlf);
    if(ret == false){
        //解析失败
        mReadBuffer.retrieveAll();
        return false;
    }else{
        // 清除这条【请求行】的缓冲区，本质上就是移动一下可读的指针位置
        mReadBuffer.retrieveUtil(crlf+2);
    }

    //解析请求行之后的所有行
    crlf = mReadBuffer.findLastCRLF();
    if(crlf == nullptr){
        mReadBuffer.retrieveAll();
        return false;
    }
    ret = parseRequest2(mReadBuffer.peek(), crlf);
    if(ret == false){
        mReadBuffer.retrieveAll();
        return false;
    }else{
        mReadBuffer.retrieveUtil(crlf+2);
    }
    return true;
}

bool RtspConnection::parseRequest1(const char* begin, const char* end){
    /*
    解析请求行
    */
    char method[64] = { 0 };
    char url[512] = { 0 };
    char version[64] = { 0 };
    std::string line = std::string(begin,end);
    if(line.find("OPTIONS") == 0 || line.find("DESCRIBE") == 0 ||
    line.find("SETUP") == 0 || line.find("PLAY")==0 || line.find("TEARDOWN")==0){
        if(sscanf(line.c_str(), "%s %s %s", method, url, version)!=3) return false;
    }
       
    if(!strcmp(method,"OPTIONS")){
        mMethod = OPTIONS;
    }else if(!strcmp(method,"DESCRIBE")){
        mMethod = DESCRIBE;
    }else if(!strcmp(method,"SETUP")){
        mMethod = SETUP;
    }else if(!strcmp(method,"PLAY")){
        mMethod = PLAY;
    }else if(!strcmp(method,"TEARDOWN")){
        mMethod = TEARDOWN;
    }else{
        mMethod = NONE;
        return false;
    }
    mUrl = url;
    mVersion = version;
    //解析url中的资源名称
    int port = 0;
    char ip[64]={0};
    char suffix[64]={0};
    if(sscanf(mUrl.c_str(),"rtsp://%[^:]:%d/%s", ip, &port, suffix)==3){

    }else if(sscanf(mUrl.c_str(), "rtsp://%[^/]/%s", ip, suffix)==2){
        port = 554; // 如果rtsp请求地址中无端口，默认获取的端口为：554
    }else{
        return false;
    }
    mServerIp = std::string(ip);
    mSuffix = std::string(suffix);
    LOGI("method: %s url: %s version:%s\n", method, mUrl.c_str(), mVersion.c_str());
    return true;
}

bool RtspConnection::parseRequest2(const char* begin, const char* end){

    std::string message = std::string(begin,end);

    if(!parseCSeq(message)){
        return false;
    }
    if(mMethod == OPTIONS){
        return true;
    }else if(mMethod==DESCRIBE){
        return parseDescribe(message);
    }else if(mMethod==SETUP){
        return parseSetup(message);
    }else if(mMethod==PLAY){
        return parsePlay(message);
    }else if(mMethod==TEARDOWN){
        return true;
    }else{
        return false;
    }
    
}

bool RtspConnection::parseCSeq(std::string message){
   
    size_t pos = message.find("CSeq:");
    if(pos!=std::string::npos){
        uint32_t cseq = 0;
        sscanf(message.c_str()+pos, "%*[^:]: %d", &cseq);//%*[^:]的意思是匹配不是冒号的所有字符 *表示全部丢弃
        mCSeq = cseq;
        return true;
    }
    return false;
}
bool RtspConnection::parseDescribe(std::string message){
        if ((message.rfind("Accept") == std::string::npos)
        || (message.rfind("sdp") == std::string::npos))
    {
        return false;
    }

    return true;
}
bool RtspConnection::parseSetup(std::string message){

    size_t pos = 0;
    // 解析trackID
    for(int i=0;i<MediaSession::MEDIA_MAX_TRACK_NUM;i++){
        pos = mUrl.find(mStreamPrefix + std::to_string(i));
        if(pos!=std::string::npos){
            if(i==0){
                mTrackId = MediaSession::TrackId0;
            }else if(i==1){
                mTrackId = MediaSession::TrackId1;
            }
            
        }
    }
    if(mTrackId == MediaSession::TrackIdNone){
        return false;
    }
    //解析Transport
    pos = message.find("Transport:");
    if(pos!=std::string::npos){
        
        if(message.find("RTP/AVP/TCP")!=std::string::npos){
            // RTP包基于TCP传输
            uint8_t rtpChannel, rtcpChannel;
            if(sscanf(message.c_str(), "%*[^;];%*[^;];%*[^=]=%d-%d",&rtpChannel, &rtcpChannel)!=2){
                return false;
            }
            mIsRtpOverTcp=true;
            mRtpChannel = rtpChannel;
            
        }else if((pos = message.find("RTP/AVP"))!=std::string::npos){
            // RTP包基于UDP传输
            uint16_t rtpPort, rtcpPort = 0;
            if(message.find("unicast")!=std::string::npos){
                if(sscanf(message.c_str(), "%*[^;];%*[^;];%*[^=]=%d-%d",&rtpPort,&rtcpPort)!=2){
                    return false;
                }
            }else if(message.find("multicast")!=std::string::npos){
                return true;
            }else{
                return false;
            }
            mPeerRtpPort = rtpPort;
            mPeerRtcpPort = rtcpPort;
            
        }else{
            return false;
        }
        return true;
    }
    return false;

}
bool RtspConnection::parsePlay(std::string message){

    //解析出SessionId
    size_t pos=0;
    if((pos=message.find("Session:"))!=std::string::npos){
        uint32_t sessionId = 0;
        if(sscanf(message.c_str()+pos, "%*[^:]: %u",&sessionId)!=1){
            return false;
        }
        mSessionId = sessionId;
        return true;
    }
    return false;
}

bool RtspConnection::handleOptions(){
    std::stringstream ss;
    ss<<"RTSP/1.0 200 OK\r\n";
    ss<<"CSeq: "<<mCSeq<<"\r\n";
    ss<<"Public: OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN\r\n\r\n";
    if(sendMessage(ss.str().c_str(), ss.str().size())<0){
        return false;
    }
    return true;
}
bool RtspConnection::handleDescribe(){

    MediaSession* session = mRtspServer->getSessionManager()->getSession(mSuffix);
    if(session==nullptr){
        LOGE("can't find session:%s", mSuffix.c_str());
        return false;
    }
    std::string sdp = session->generateSDPDescription();
    std::stringstream ss;
    ss<<"RTSP/1.0 200 OK\r\n";
    ss<<"CSeq: "<<mCSeq<<"\r\n";
    ss<<"Content-Base: "<<mUrl<<"\r\n";
    ss<<"Content-type: application/sdp\r\n";
    ss<<"Content-Length: "<<sdp.size()<<"\r\n\r\n";
    ss<<sdp.c_str();
    if(sendMessage(ss.str().c_str(), ss.str().size())<0){
        return false;
    }
    return true;
}
bool RtspConnection::handleSetup(){
    char sessionName[100]={0};
    std::stringstream ss;
    //解析请求的session名
    if(sscanf(mSuffix.c_str(), "%[^/]/", sessionName)!=1) return false; 
    MediaSession* session = mRtspServer->getSessionManager()->getSession(sessionName);
    if(session == nullptr){
        LOGE("handleSetup can't find session %s", sessionName);
        return false;
    }
    //如果请求的轨道号不存在，或者已经连接，则不建立
    if(mTrackId >= MediaSession::MEDIA_MAX_TRACK_NUM || mRtpInstances[mTrackId] || mRtcpInstances[mTrackId]){
        return false;
    }
    if(session->isStartMuticast()){
        //TODO 多播逻辑
        ss<<"RTSP/1.0 200 OK\r\n";
        ss<<"CSeq: "<<mCSeq<<"\r\n";
        ss<<"Transport: RTP/AVP;multicast;";
        ss<<"destination="<<session->getMulticastAddr().c_str()<<";";
        ss<<"source="<<mServerIp.c_str()<<";port="<<session->getMulticastDestRtpPort(mTrackId)<<";";
        ss<<"ttl=255\r\n";
        ss<<"Session: "<<std::setfill('0')<<std::setw(8)<<mSessionId<<"\r\n\r\n";

    }else{
        if(mIsRtpOverTcp){
            //TODO 基于TCP传输
            if(!createRtpRtcpOverTcp(mTrackId)){
                LOGI("failed to createRtpOverTcp");
                return false;
            }
            mRtpInstances[mTrackId]->setSessionId(mSessionId);
            session->addRtpInstance(mTrackId, mRtpInstances[mTrackId]);
            ss<<"RTSP/1.0 200 OK\r\n";
            ss<<"CSeq: "<<mCSeq<<"\r\n";
            ss<<"Transport: RTP/AVP/TCP;unicast;interleaved="<<(int)mRtpChannel<<"-"<<(int)(mRtpChannel+1)<<"\r\n";
            ss<<"Session: "<<std::setfill('0')<<std::setw(8)<<mSessionId<<"\r\n\r\n";

        }else{
            // 基于UDP传输
            if(!createRtpRtcpOverUdp(mTrackId)){
                LOGE("failed to createRtpOverUdp");
                return false;
            }
            mRtpInstances[mTrackId]->setSessionId(mSessionId);
            mRtcpInstances[mTrackId]->setSessionId(mSessionId);
            session->addRtpInstance(mTrackId, mRtpInstances[mTrackId]);

            
            ss<<"RTSP/1.0 200 OK\r\n";
            ss<<"CSeq: "<<mCSeq<<"\r\n";
            ss<<"Transport: RTP/AVP;unicast;client_port="<<mPeerRtpPort<<"-"<<mPeerRtcpPort<<";server_port="<<mRtpInstances[mTrackId]->getLocalPort()<<"-"<<mRtcpInstances[mTrackId]->getLocalPort()<<"\r\n";
            //ss<<"Session: "<<std::format("{:08}\r\n", mSessionId)<<"\r\n";
            ss<<"Session: "<<std::setfill('0')<<std::setw(8)<<mSessionId<<"\r\n\r\n"; //少了\r\n找了好久的bug！

            
        } 
    }

    if(sendMessage(ss.str().c_str(), ss.str().size())<0) return false;
    return true;

}
bool RtspConnection::handlePlay(){

    std::stringstream ss;
    ss<<"RTSP/1.0 200 OK\r\n";
    ss<<"CSeq: "<<mCSeq<<"\r\n";
    ss<<"Range: npt=0.000-\r\n";
    ss<<"Session: "<<std::setfill('0')<<std::setw(8)<<mSessionId<<"\r\n\r\n";

    if(!sendMessage(ss.str().c_str(), ss.str().size())<0) return false;
    //开始播放
    for(int i=0;i<MediaSession::MEDIA_MAX_TRACK_NUM;i++){
        if(mRtpInstances[i]){
            mRtpInstances[i]->setAlive(true);
        }
        if(mRtcpInstances[i]){
            mRtcpInstances[i]->setAlive(true);
        }
    }
    return true;
}
bool RtspConnection::handleTeardown(){

}

int RtspConnection::sendMessage(const char* data, int len){

    LOGI("\nServer========>Client\n%s", data);
    int ret;
    mWriteBuffer.append(data, len);
    ret = mWriteBuffer.write(mClientFd);
    mWriteBuffer.retrieveAll();
    return ret;
}

//创建UDP通道
bool RtspConnection::createRtpRtcpOverUdp(MediaSession::TrackId trackId){

    int rtpSockfd, rtcpSockfd;
    uint16_t rtpPort, rtcpPort;
    if(mRtpInstances[trackId]){
        //已经建立过当前轨道传输通道
        return false;
    }
    //建立连接,重试10次
    int cnt;
    for(cnt=0;cnt<10;cnt++){
        //创建文件描述符
        rtpSockfd = sockets::createUdpSock();
        if(rtpSockfd<0) return false;
        rtcpSockfd = sockets::createUdpSock();
        if(rtcpSockfd<0) return false;

        //随机生成端口号
        uint16_t port = rand() & 0xFFFE;
        if(port < 1000) port+=1000;
        rtpPort = port;
        rtcpPort = port+1;
        //绑定IP端口
        if(!sockets::bind(rtpSockfd, "0.0.0.0", rtpPort)){
            sockets::close(rtpSockfd);
            sockets::close(rtcpSockfd);
            continue;
        }
        if(!sockets::bind(rtcpSockfd, "0.0.0.0", rtcpPort)){
            sockets::close(rtpSockfd);
            sockets::close(rtcpSockfd);
            continue;
        }

        break;
    }
    if(cnt==10) return false;

    //创建RTP和RTCP实例
    mRtpInstances[trackId] = RtpInstance::createNewOverUdp(rtpSockfd, rtpPort, mPeerIp, mPeerRtpPort);
    mRtcpInstances[trackId] = RtcpInstance::createNewOverUdp(rtcpSockfd, rtcpPort, mPeerIp, mPeerRtcpPort);
    
    return true;
}

// 创建TCP通道
bool RtspConnection::createRtpRtcpOverTcp(MediaSession::TrackId trackId){

    mRtpInstances[trackId] = RtpInstance::createNewOverTcp(mClientFd, mRtpChannel);
    return true;
}