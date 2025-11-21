#include"MediaSessionManager.h"

MediaSessionManager* MediaSessionManager::createNew(){
    return new MediaSessionManager();
}

MediaSessionManager::MediaSessionManager()
{

}

MediaSessionManager::~MediaSessionManager()
{
}

bool MediaSessionManager::addSession(MediaSession* session){
    if(mSessionMap.find(session->name())!=mSessionMap.end()){
        return false;
    }

    mSessionMap.insert({session->name(), session});
    return true;
}

MediaSession* MediaSessionManager::getSession(std::string sessionName){
    std::unordered_map<std::string, MediaSession*>::iterator it = mSessionMap.find(sessionName);
    if(it!=mSessionMap.end()){
        return it->second;
    }
    return nullptr;
}   