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
	int start(const char *ip, const int port);
	void addsig(int sig, void(handler)(int), bool restart = true);

  private:
	class http_conn users[MAX_FD]; /*预先分配一些任务类对象*/
};

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
	http_conn::sum_user_count = 0;
	struct sockaddr_in address;
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &address.sin_addr);
	address.sin_port = htons(port);

	int listenfd = Socket(AF_INET, SOCK_STREAM, 0);
	/*close()立刻返回，不会发送(未发送完成)的数据，而是通过一个REST包强制的关闭socket描述符，即强制退出。*/
	struct linger tmp = {1, 0};
	Setsockopt(listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));

	/*设置该套接字使之可以重新绑定端口,避免　time_wait　状态，用于调试*/
	int optval = 1;
	Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (int *)&optval, sizeof(int));

	Bind(listenfd, (struct sockaddr *)&address, sizeof(address));
	Listen(listenfd, 5);
	epoll_event events[MAX_EVENT_NUMBER];
	int epollfd = Epoll_create(5);
	
}

#endif
