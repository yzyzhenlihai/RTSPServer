#pragma once
#include"Log.h"
#include<string>
#include<fcntl.h>

#ifndef WIN32
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h> //close
#else

#endif
namespace sockets{

    int createTcpSock();//默认创建非阻塞Tcp描述符
    int createUdpSock();
    bool bind(int sockfd, std::string ip,uint16_t port);
    bool listen(int sockfd, int n);
    int accept(int sockfd);
    void close(int sockfd);
    int write(int sockefd, const char* data, int len);

    int sendto(int sockfd, const void* buf, int len, const struct sockaddr* destAddr);
    
    void setReuseAddr(int sockfd, int on);
    void setNonBlockAndCloseOnExec(int sockfd);
    void ignoreSigPipeOnSocket(int sockfd);
    void setMulticastTtl(int sockfd, int ttl);
    void setMulticastLoop(int sockfd, int on);
};