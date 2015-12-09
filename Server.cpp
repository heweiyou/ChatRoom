#include <iostream>

#include "Server.h"

using namespace std;


Server::Server(){
    //定义套接字协议族
    serverAddr.sin_family = PF_INET;
	//定义套接字端口号
	//The htons() function converts the unsigned short integer hostshort from  host byte order to network byte order.
    serverAddr.sin_port = htons(SERVER_PORT);
	//定义套接字IP地址
	//The inet_addr() function converts the Internet  host  address  cp  from IPv4  numbers-and-dots notation into binary data in network byte order. 
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    listener = 0;
    
    // epoll fd
    epfd = 0;
}

// 初始化服务端并启动监听
void Server::Init() {
    cout << "Init Server..." << endl;
    
    //Step 1:创建监听socket
    listener = socket(PF_INET, SOCK_STREAM, 0);
    if(listener < 0) { perror("listener"); exit(-1);}
    
    //Step 2:绑定地址
    if( bind(listener, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("bind error");
        exit(-1);
    }

    //Step 3:监听链接
    int ret = listen(listener, 5);
    if(ret < 0) {
        perror("listen error"); 
        exit(-1);
    }

    cout << "Start to listen: " << SERVER_IP << endl;

    //Step 4:创建时间表
    epfd = epoll_create(EPOLL_SIZE);
    
    if(epfd < 0) {
        perror("epfd error");
        exit(-1);
    }

    //Step 5:添加socket fd到epoll fd
    addfd(epfd, listener, true);

}

// 关闭所有打开的文件描述符
void Server::Close() {

    //关闭socket文件描述符
    close(listener);
    
    //关闭epoll文件描述副
    close(epfd);
}

// 发送广播消息给所有客户端
int Server::SendBroadcastMessage(int clientfd)
{
    // buf[BUF_SIZE] 定义缓存消息字符串
    // message[BUF_SIZE] 定义格式化消息字符串
    char buf[BUF_SIZE], message[BUF_SIZE];
    bzero(buf, BUF_SIZE);
    bzero(message, BUF_SIZE);

    // 输出接受到发送消息客户端的文件描述符
    cout << "read from client(clientID = " << clientfd << ")" << endl;
    int len = recv(clientfd, buf, BUF_SIZE, 0);

    // 判断接受消息长度
    if(len == 0) 
    {
        close(clientfd);
        
        // 从客户端列表中一处发送消息客户端的文件描述符
        clients_list.remove(clientfd);
        cout << "ClientID = " << clientfd 
             << " closed.\n now there are " 
             << clients_list.size()
             << " client in the char room"
             << endl;

    } else 
    {
        // 判断客户端列表长度
        if(clients_list.size() == 1) { 
            // 发送警告消息
            send(clientfd, CAUTION, strlen(CAUTION), 0);
            return len;
        }
        // 格式化接受到的消息
        sprintf(message, SERVER_MESSAGE, clientfd, buf);

        // 遍历客户端列表中所有文件描述符并发送消息
        list<int>::iterator it;
        for(it = clients_list.begin(); it != clients_list.end(); ++it) {
           if(*it != clientfd){
                if( send(*it, message, BUF_SIZE, 0) < 0 ) {
                    return -1;
                }
           }
        }
    }
    return len;
}

// 启动服务端
void Server::Start() {

    // epoll事件数组
    static struct epoll_event events[EPOLL_SIZE]; 

    // Step 1:初始化服务端
    Init();

    // Step 2:进入住循环
    while(1)
    {
        //Step 3:获取就绪的事件,并返回事件书epoll_events_count
        int epoll_events_count = epoll_wait(epfd, events, EPOLL_SIZE, -1);

        if(epoll_events_count < 0) {
            perror("epoll failure");
            break;
        }

        cout << "epoll_events_count =\n" << epoll_events_count << endl;

        //Step 4:循环防护里所有就绪事件
        for(int i = 0; i < epoll_events_count; ++i)
        {
            int sockfd = events[i].data.fd;
            //如何是新连接则接受链接并将链接添加到epoll
            if(sockfd == listener)
            {
                struct sockaddr_in client_address;
                socklen_t client_addrLength = sizeof(struct sockaddr_in);
                int clientfd = accept( listener, ( struct sockaddr* )&client_address, &client_addrLength );

                cout << "client connection from: "
                     << inet_ntoa(client_address.sin_addr) << ":"
                     << ntohs(client_address.sin_port) << ", clientfd = "
                     << clientfd << endl;

                addfd(epfd, clientfd, true);

                // 客户端列表添加新客户端文件描述符
                clients_list.push_back(clientfd);
                cout << "Add new clientfd = " << clientfd << " to epoll" << endl;
                cout << "Now there are " << clients_list.size() << " clients int the chat room" << endl;

                // 向新客户端发送欢迎信息 
                cout << "welcome message" << endl;                
                char message[BUF_SIZE];
                bzero(message, BUF_SIZE);
                sprintf(message, SERVER_WELCOME, clientfd);
                int ret = send(clientfd, message, BUF_SIZE, 0);
                if(ret < 0) {
                    perror("send error");
                    Close();
                    exit(-1);
                }
            }else
			{//如果不是新连接则群发消息   
                int ret = SendBroadcastMessage(sockfd);
                if(ret < 0) {
                    perror("error");
                    Close();
                    exit(-1);
                }
            }
        }
    }

    //关闭所有文件描述符号
    Close();
}
