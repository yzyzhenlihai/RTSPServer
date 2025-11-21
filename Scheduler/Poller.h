#pragma once
#include"Event.h"
#include<map>
class Poller
{

public:
    
    virtual ~Poller();
    virtual bool addIOEvent(IOEvent* event) = 0;
    virtual bool updateIOEvent(IOEvent* event) = 0;
    virtual bool removeIOEvent(IOEvent* event) = 0;
    virtual void handleEvent() = 0;

protected:
    Poller();
    
protected:
    std::map<int, IOEvent*> mEventMap;
    
};


