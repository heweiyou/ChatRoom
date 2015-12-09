#ifndef  CHATROOM_COMMON_H
#define CHATROOM_COMMON_H

#include <iostream>
#include <list>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 服务器地址
#define SERVER_IP "127.0.0.1"

// 服务器端口号
#define SERVER_PORT 8888

// int epoll_create(int size)创建一个epoll句柄,参数size为epoll所支持的最大句柄数
// epoll支持最大句柄数
#define EPOLL_SIZE 5000

// 消息缓存大小
#define BUF_SIZE 0xFFFF
    
//服务器端默认的欢迎消息 
#define SERVER_WELCOME "Welcome you join to the chat room! Your chat ID is: Client #%d"

// 服务器接受到的客户消息
#define SERVER_MESSAGE "ClientID %d say >> %s"

// 服务器端默认的退出消息
#define EXIT "EXIT"

// 服务器警告消息
#define CAUTION "There is only one int the char room!"


//将文件描述符fd添加到epollfd标示的内核事件表中， 并注册EPOLLIN和EPOOLET事件，EPOLLIN是数据可读事件；EPOOLET表明是ET工作方式。最后将文件描述符设置非阻塞方式
/**
 * @param epollfd: epoll句柄
 * @param fd: 文件描述符
 * @param enable_et : enable_et = true, 
 * 采用epoll的ET工作方式；否则采用LT工作方式
**/
static void addfd( int epollfd, int fd, bool enable_et )
{
/**
 * int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
 * 函数功能： epoll事件注册函数
 * 参数epfd为epoll的句柄，即epoll_create返回值
 * 参数op表示动作，用3个宏来表示：  
 * 		EPOLL_CTL_ADD(注册新的fd到epfd)， 
 * 		EPOLL_CTL_MOD(修改已经注册的fd的监听事件)，
 * 　　 EPOLL_CTL_DEL(从epfd删除一个fd)；
 *  参数fd为需要监听的标示符；
 *  参数event告诉内核需要监听的事件，event的结构如下：
 *	 struct epoll_event {
 *   __uint32_t events; //Epoll events
 *     epoll_data_t data; //User data variable
 *    };
 *    其中介绍events是宏的集合，本项目主要使用EPOLLIN(表示对应的文件描述符可以读，即读事件发生)，其他宏类型，可以google之！
**/
	struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    if( enable_et )
        ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
    //将文件描述符设置为非阻塞方式（利用fcntl函数）
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0)| O_NONBLOCK);
    printf("fd added to epoll!\n\n");
}

#endif // CHATROOM_COMMON_H
