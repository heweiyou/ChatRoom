#ifndef CHATROOM_SERVER_H
#define CHATROOM_SERVER_H

#include <string>

#include "Common.h"

using namespace std;

//定义Server类
class Server {

public:
    // Server构造函数
    Server();

    // 初始化服务端并启动监听
    void Init();

    // 关闭所有文件描述符
    void Close();

    // 启动服务端
    void Start();

private:
    // 发送广播消息给所有客户端
    int SendBroadcastMessage(int clientfd);

    // 服务端地址
    struct sockaddr_in serverAddr;
    
    //socket文件描述符
    int listener;

    // epoll_create创建的epoll文件描述符
    int epfd;
    
    // ¿¿¿¿¿
    list<int> clients_list;
};



#endif //CHATROOM_SERVER_H
