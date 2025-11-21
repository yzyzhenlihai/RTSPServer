#include "SocketsOps.h"

#ifndef WIN32

#else

#endif

int sockets::createTcpSock(){
    /*
    创建TCP文件描述
    */
#ifndef WIN32
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
#else
    //TODO windows下创建监听文件描述符
#endif

    return sockfd;
}
int sockets::createUdpSock(){
    /*
    创建UDP文件描述符
    */
#ifndef WIN32
   int sockfd = ::socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
#else
    //TODO windows下创建监听文件描述符
#endif
    return sockfd;
}

bool sockets::bind(int sockfd, std::string ip,uint16_t port){
    
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr.s_addr);  
    //这里::是作用域解析运算符，如果前面没有任何类名或命名空间，表示强制编译器从全局命名空间中查找这个函数（防御性编程，为了避免命名冲突和歧义）
    if(::bind(sockfd, (struct sockaddr*)(&addr), sizeof(addr))<0){ 
        LOGE("bind error, fd=%d,ip=%s,port=%d", sockfd, ip.c_str(), port);
        return false;
    }

    return true;
}

bool sockets::listen(int sockfd, int n){
    if(::listen(sockfd, n)<0){
        LOGE("::listen error, fd=%d", sockfd);
        return false;
    }
    return true;
}   

int sockets::accept(int fd){
    struct sockaddr_in addr = {0};
    socklen_t addrlen = sizeof(struct sockaddr_in);
    int clientFd = ::accept(fd, (struct sockaddr*)&addr, &addrlen);
    setNonBlockAndCloseOnExec(clientFd);
    ignoreSigPipeOnSocket(clientFd);
    return clientFd;
}

void sockets::close(int sockfd){
#ifndef WIN32
    ::close(sockfd);
#else
    //TODO window下关闭监听描述符
#endif
}

void sockets::setReuseAddr(int sockfd, int on){
    int option = on ? 1 : 0;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
}

void sockets::setNonBlockAndCloseOnExec(int sockfd){
#ifndef WIN32
    //设置文件描述符非阻塞
    int flags = ::fcntl(sockfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    int ret = ::fcntl(sockfd, F_SETFL, flags);

    //设置为执行时关闭
    /*
    （进程级）
    如果主线程开了一个子进程，那么子进程会继承主线程的所有文件描述符，这样是不安全的。
    通过设置FD_CLOEXEC可以避免主进程将描述符传给完全不同的程序（子进程）
    */
    flags = ::fcntl(sockfd, F_GETFD, 0);
    flags |= FD_CLOEXEC;
    ret = ::fcntl(sockfd, F_GETFD, flags);
#endif
}

void sockets::ignoreSigPipeOnSocket(int sockfd){
#ifndef WIN32
    /*
    防止在对一个已关闭的套接字进行写入操作时，程序因收到 SIGPIPE 信号而异常终止。
    通过对指定文件描述符忽略SIGPIPE信号，避免程序崩溃，
    可以在代码中通过检查 write() 的返回值和 errno 的值来判断连接是否已断开，
    然后执行相应的清理逻辑（比如关闭这个sockfd，释放相关资源等），从而优雅地处理这个错误。
    */
    int option = 1;
    setsockopt(sockfd, SOL_SOCKET, MSG_NOSIGNAL, &option, sizeof(option));
#endif
}

int sockets::write(int sockefd, const char* data, int len){
    int ret;
#ifndef WIN32
    ret = ::send(sockefd, data, len, 0);
#else
    ret = ::write(sockefd, data, len);
#endif
    return ret;
}

int sockets::sendto(int sockfd, const void* buf, int len, const struct sockaddr* destAddr){
    socklen_t addrLen = sizeof(struct sockaddr);
    
    return ::sendto(sockfd, buf, len, 0, destAddr, addrLen);
}
