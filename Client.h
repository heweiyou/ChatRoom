#ifndef CHATROOM_CLIENT_H
#define CHATROOM_CLIENT_H

#include <string>
#include "Common.h"

using namespace std;

// 定义客户端类
class Client {

public:
    // 客户端类构造函数
    Client();

    // 链接服务端
    void Connect();

    // 关闭所有文件描述副
    void Close();

    // 启动客户端
    void Start();

private:

    // 套接字文件描述副
    int sock;

    // 进程id
    int pid;
    
    // epoll_create返回的epoll文件描述符
    int epfd;

    // 管道文件描述符
    int pipe_fd[2];

    // 判断客户端是否在工作
    bool isClientwork;

    // 消息
    char message[BUF_SIZE];

    //套接字地址
    struct sockaddr_in serverAddr;
};



#endif //CHATROOM_CLIENT_H
