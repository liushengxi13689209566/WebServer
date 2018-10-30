/*************************************************************************
	> File Name: WebServer.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年10月27日 星期六 10时42分48秒
 ************************************************************************/

#ifndef _WEBSERVER_H
#define _WEBSERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>
#include <signal.h>
#include <memory>
#include <map>

#include "http_conn.h"
#include "threadPool.h"
#include "system_call.h"

const int MAX_FD = 65536;
const int MAX_EVENT_NUMBER = 10000;

/*服务器类*/
class WebServer
{
  public:
	WebServer(const char *ip, const int port)
	{
		start(ip, port);
	}
	virtual ~WebServer()
	{
	}

  private:
	void Setnonblock(int fd);
	void addfd(int fd, bool one_shot);
	void addsig(int sig, void(handler)(int), bool restart = true);
	int start(const char *ip, const int port);

  private:
	static int sum_user_count = 0; /*总用户*/
	static int epollfd = -1;
	static int listenfd = 0;
	std::map<int, http_conn> users; /*预先分配一些任务类对象*/
};
void WebServer::Setnonblock(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
}
void WebServer::addfd(int fd, bool one_shot)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
	if (one_shot)
		event.events |= EPOLLONESHOT;
	Epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	Setnonblock(fd);
}
void WebServer::addsig(int sig, void(handler)(int), bool restart)
{
	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = handler;
	if (restart)
	{
		/*由此信号中断的系统调用是否要再启动*/
		sa.sa_flags |= SA_RESTART;
	}
	/*sigfillset()用来将参数set 信号集初始化, 然后把所有的信号加入到此信号集里*/
	sigfillset(&sa.sa_mask);
	if (sigaction(sig, &sa, NULL) == -1)
		throw __LINE__;
}
int WebServer::start(const char *ip, const int port)
{
	/*忽略SIGPIPE信号(SIG_IGN表示忽略SIGPIPE那个注册的信号)*/
	addsig(SIGPIPE, SIG_IGN);

	threadPool<http_conn> pool(8);
	sum_user_count = 0;
	struct sockaddr_in address;
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &address.sin_addr);
	address.sin_port = htons(port);

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);
	/*close()立刻返回，不会发送(未发送完成)的数据，而是通过一个REST包强制的关闭socket描述符，即强制退出。*/
	struct linger tmp = {1, 0};
	Setsockopt(listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));

	/*设置该套接字使之可以重新绑定端口,避免　time_wait　状态，用于调试*/
	int optval = 1;
	Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (int *)&optval, sizeof(int));

	Bind(listenfd, (struct sockaddr *)&address, sizeof(address));
	Listen(listenfd, 5);
	epoll_event events[MAX_EVENT_NUMBER];
	epollfd = Epoll_create(5);
	addfd(listenfd, false);
	while (true)
	{
		int number = Epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
		for (int i = 0; i < number; i++)
		{
			int sockfd = events[i].data.fd;
			if (sockfd == listenfd) /*有新的连接到来*/
			{
				struct sockaddr_in client_address;
				socklen_t client_len = sizeof(client_address);
				int connfd = Accept(listenfd, (struct sockaddr *)&client_address, &client_len);
				if (sum_user_count >= MAX_FD)
				{
					/*发送一个服务器繁忙的信息过去,待完成*/
					continue;
				}
				/*如果不存在会直接插入 map<connfd,http_conn> 进去*/
				users[connfd].init(connfd, client_address);
			}
			/*出错*/
			else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
			{
				/*如果有异常，直接关闭客户连接*/
				users[sockfd].close_conn();
			}
			/*读事件*/
			else if (events[i].events & EPOLLIN)
			{
				if (users[sockfd].read())
					pool.append(users[sockfd]); /*　users+sockfd 指针偏移*/
				else
					users[sockfd].close_conn();
			}
			/*写事件*/
			else if (events[i].events & EPOLLOUT)
			{
				if ( !users[sockfd].write() )
					users[sockfd].close_conn();
			}
			/*其他*/
			else
			{
			}
		}
	}
	close(epollfd);
	close(listenfd);
	return 0;
}
#endif
