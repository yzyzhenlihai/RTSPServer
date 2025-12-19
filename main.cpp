#include "RtspServer.h"
#include"H264FileMediaSource.h"
#include"H264FileSink.h"
#include"AACFileMediaSource.h"
#include"AACFileSink.h"
#include"ThreadPool.h"
int main(){

    /*rtp over tcp
    ffplay -i -rtsp_transport tcp  rtsp://172.17.187.232:1234/test

    // rtp over udp
    ffplay -i rtsp://172.17.187.232:1234/test
    
    //multicast
    ffplay -i -rtsp_transport udp_multicast rtsp://172.17.187.232:1234/test
    */
    EventScheduler* eventScheduler = EventScheduler::createNew(EventScheduler::POLLER_SELECT);
    ThreadPool* threadPool = ThreadPool::createNew(10);
    UsageEnvironment* env = UsageEnvironment::createNew(eventScheduler, threadPool);
    Ipv4Address addr = Ipv4Address("172.17.187.232", 1234);
    MediaSessionManager* sessionManager = MediaSessionManager::createNew();
    RtspServer* rtspServer = RtspServer::createNew(env, addr, sessionManager);
    rtspServer->start();

    LOGI("===== Session init start ====");
    MediaSession* session = MediaSession::createNew("test", "172.17.187.232");
    MediaSource* source = H264FileMediaSource::createNew(env, "./resources/daliu.h264");
    Sink* sink = H264FileSink::createNew(env, source);
    session->addSink(MediaSession::TrackId0, sink);

    source = AACFileMediaSource::createNew(env, "./resources/daliu.aac");
    sink = AACFileSink::createNew(env, source);
    session->addSink(MediaSession::TrackId1, sink);
    sessionManager->addSession(session);
    session->startMulticast(); // 启动多播
    //开启事件循环
    eventScheduler->loop();
    return 0;
}