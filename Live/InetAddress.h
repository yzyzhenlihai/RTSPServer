#pragma once
#include<string>
#include<iostream>
#ifndef WIN32
// linux环境下
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#else
// windows环境下
#endif

class Ipv4Address{
public:
    Ipv4Address();
    Ipv4Address(std::string ip, uint16_t port);
    ~Ipv4Address();
    std::string getIp();
    uint16_t getPort();
    struct sockaddr* getAddr();
private:
    std::string mIp;
    uint16_t mPort;
    struct sockaddr_in mAddr;
};