#pragma once
#include"MediaSession.h"
#include<unordered_map>
class MediaSessionManager
{

public:
    static MediaSessionManager* createNew();
    MediaSessionManager();
    ~MediaSessionManager();

public:
    bool addSession(MediaSession* session);
    MediaSession* getSession(std::string sessionName);
private:
    /* data */  
    std::unordered_map<std::string, MediaSession*> mSessionMap;

};


