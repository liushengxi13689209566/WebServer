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
#include "../threadPool.h"
#include "../system_call.h"

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
	~WebServer()
	{
		Close(listenfd);
	}

  private:
	void Setnonblock(int fd);
	void addfd(int epollfd, int fd, bool one_shot);
	void removefd(int epollfd, int fd);
	void addsig(int sig, void(handler)(int), bool restart = true);
	int start(const char *ip, const int port);
	void WebServer_init(int epollfd, int connfd, const sockaddr_in &client_address);
	void WebServer_closefd(int epollfd, int connfd);

  private:
	static int listenfd;
	std::map<int, http_conn> users; /*任务类对象*/
  public:
	static int sum_user_count; /*总用户*/
};
int WebServer::listenfd = 0;
int WebServer::sum_user_count = 0;

void WebServer::Setnonblock(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
}
void WebServer::addfd(int epollfd, int fd, bool one_shot)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
	if (one_shot)
		event.events |= EPOLLONESHOT;
	Epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	Setnonblock(fd);
}
void WebServer::removefd(int epollfd, int fd)
{
	Epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
	Close(fd);
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
void WebServer::WebServer_init(int epollfd, int connfd, const sockaddr_in &client_address)
{
	int error = 0;
	socklen_t len = sizeof(error);
	/*处理错误*/
	if (getsockopt(connfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
		throw __LINE__;
	addfd(epollfd, connfd, true);
	WebServer::sum_user_count++;
	users[connfd].http_sockfd = connfd;
	users[connfd].http_address = client_address;
}
void WebServer::WebServer_closefd(int epollfd, int connfd)
{
	if (users[connfd].http_sockfd != -1)
	{
		removefd(epollfd, connfd);
		users[connfd].http_sockfd = -1;
		this->sum_user_count--;
	}
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

	/*设置该套接字使之可以重新绑定端口 */
	int optval = 1;
	Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (int *)&optval, sizeof(int));

	Bind(listenfd, (struct sockaddr *)&address, sizeof(address));
	Listen(listenfd, 5);
	epoll_event events[MAX_EVENT_NUMBER];
	int epollfd = Epoll_create(5);
	addfd(epollfd, listenfd, false);
	http_conn::http_epollfd = epollfd;

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
				WebServer_init(epollfd, connfd, client_address);
			}
			/*出错*/
			else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
			{
				/*如果有异常，直接关闭客户连接*/
				WebServer_closefd(epollfd, sockfd);
			}
			/*读事件*/
			else if (events[i].events & EPOLLIN)
			{
				printf("读事件\n");
				if (users[sockfd].read()){

					printf("读事件　完成！！！\n");

					if(!pool.append(&users[sockfd])){
						printf("append 失败！！！！\n");
					}
				}
				else
					WebServer_closefd(epollfd, sockfd);
			}
			/*写事件*/
			else if (events[i].events & EPOLLOUT)
			{
				printf("写事件\n");
				if (!users[sockfd].write())
					WebServer_closefd(epollfd, sockfd);
			}
			/*其他*/
			else
			{
			}
		}
	}
	return 0;
}

#endif
