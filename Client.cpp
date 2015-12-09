#include <iostream>

#include "Client.h"

using namespace std;


// 客户端构造函数
Client::Client(){
    
    // 定义套接字协议族
    serverAddr.sin_family = PF_INET;
	// 定义套接字端口
    serverAddr.sin_port = htons(SERVER_PORT);
	// 定义套接字IP地址
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    
    sock = 0;
    pid = 0;
    // 定义客户端在工作
    isClientwork = true;   
    // epool fd
    epfd = 0;
}

//连接服务器
void Client::Connect() {
    cout << "Connect Server: " << SERVER_IP << " : " << SERVER_PORT << endl;
    
    //Step 1:创建socket
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        perror("sock error");
        exit(-1); 
    }

    //Step 2:连接服务端 
    if(connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("connect error");
        exit(-1);
    }

    //Step 3:创建管道,其中fd[0]用于父进程读,fd[1]用于子进程写
    if(pipe(pipe_fd) < 0) {
        perror("pipe error");
        exit(-1);
    }

    //Step 4:创建epoll
    epfd = epoll_create(EPOLL_SIZE);
    
    if(epfd < 0) {
        perror("epfd error");
        exit(-1); 
    }

    //Step 4:将sock和管道读端描述符都添加到内核时间表中
    addfd(epfd, sock, true);
    addfd(epfd, pipe_fd[0], true);

}

// 关闭所有打开的文件描述副
void Client::Close() {

    if(pid){
       //关闭sock和管道读端描述符
        close(pipe_fd[0]);
        close(sock);
    }else{
        //关闭管道写端描述符
        close(pipe_fd[1]);
    }
}

//启动客户端
void Client::Start() {

    //定义epoll时间
    static struct epoll_event events[2];
    
    //Step 1:链接服务器
    Connect();
    
    //Step 2:创建子进程
    pid = fork();
    
    // 子进程传见失败
    if(pid < 0) {
        perror("fork error");
        close(sock);
        exit(-1);
    } else if(pid == 0) {
        //Step 4:进入父进程执行流程
        //关闭管道读端
        close(pipe_fd[0]); 

        // 输出提示信息
        cout << "Please input 'exit' to exit the chat room" << endl;

        // 判断客户端是否在运行
        while(isClientwork){
            bzero(&message, BUF_SIZE);
            fgets(message, BUF_SIZE, stdin);

            // 比较终端输入信息与exit
            if(strncasecmp(message, EXIT, strlen(EXIT)) == 0){
                isClientwork = 0;
            }
            // 向管道中写信息
            else {
                if( write(pipe_fd[1], message, strlen(message) - 1 ) < 0 ) { 
                    perror("fork error");
                    exit(-1);
                }
            }
        }
    } else { 
        //pid > 0 进入子进程
        //关闭管道写端
        close(pipe_fd[1]); 

        //判断客户端是否还在工作
        while(isClientwork) {
			//	获取就绪事件
            int epoll_events_count = epoll_wait( epfd, events, 2, -1 );

            //遍历epoll事件
            for(int i = 0; i < epoll_events_count ; ++i)
            {
                bzero(&message, BUF_SIZE);

                //如果是新的连接
                if(events[i].data.fd == sock)
                {
                    //接受新连接的消息
                    int ret = recv(sock, message, BUF_SIZE, 0);

                    // ret= 0代表服务端断开连接
                    if(ret == 0) {
                        cout << "Server closed connection: " << sock << endl;
                        close(sock);
                        isClientwork = 0;
                    } else {
                        cout << message << endl;
                    }
                }else { 
                    //子进程从管道中读取数据
                    int ret = read(events[i].data.fd, message, BUF_SIZE);

                    // ret = 0
                    if(ret == 0)
                        isClientwork = 0;
                    else {
                        // 将读取信息发送给父进程的sock
                        send(sock, message, BUF_SIZE, 0);
                    }
                }
            }//for
        }//while
    }
    
    //关闭打开的所有文件标识符
    Close();
}
