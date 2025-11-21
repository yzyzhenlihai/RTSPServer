#include "RtspServer.h"
#include"H264FileMediaSource.h"
#include"H264FileSink.h"
#include"AACFileMediaSource.h"
#include"AACFileSink.h"
#include"ThreadPool.h"
int main(){

    EventScheduler* eventScheduler = EventScheduler::createNew(EventScheduler::POLLER_SELECT);
    ThreadPool* threadPool = ThreadPool::createNew(1);
    UsageEnvironment* env = UsageEnvironment::createNew(eventScheduler, threadPool);
    Ipv4Address addr = Ipv4Address("0.0.0.0", 1234);
    MediaSessionManager* sessionManager = MediaSessionManager::createNew();
    RtspServer* rtspServer = RtspServer::createNew(env, addr, sessionManager);
    rtspServer->start();

    LOGI("===== Session init start ====");
    MediaSession* session = MediaSession::createNew("test", "0.0.0.0");
    MediaSource* source = H264FileMediaSource::createNew(env, "./resources/daliu.h264");
    Sink* sink = H264FileSink::createNew(env, source);
    session->addSink(MediaSession::TrackId0, sink);

    source = AACFileMediaSource::createNew(env, "./resources/daliu.aac");
    sink = AACFileSink::createNew(env, source);
    session->addSink(MediaSession::TrackId1, sink);
    sessionManager->addSession(session);

    //开启事件循环
    eventScheduler->loop();
    return 0;
}