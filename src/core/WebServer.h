/*************************************************************************
	> File Name: WebServer.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年10月27日 星期六 10时42分48秒
 ************************************************************************/

#ifndef _WEBSERVER_H
#define _WEBSERVER_H


#include <memory>
#include <unordered_map>

#include"http_conn.h"

#include "../base/Epoll.h"
#include "../base/Thread_pool.h"

const int MAX_FD = 65536;

/*服务器类*/
class WebServer
{
  public:
	WebServer()
	{
	}
	~WebServer()
	{
		Close(listenfd);
	}
	int run(const char *ip, const int port);

  private:
	void SetNonBlock(int fd);
	void AddFd(int fd, bool one_shot);
	void AddSig(int sig, void(handler)(int), bool restart = true);
	void WebServerInit(int connfd, const sockaddr_in &client_address);
	void WebServer_closefd(int epollfd, int connfd);

  private:
	static int listenfd;
	Epoll serv_epollfd;
	std::unordered_map<int, http_conn> users; /*任务类对象*/

  public:
	static int sum_user_count; /*总用户*/
};

int WebServer::listenfd = 0;
int WebServer::sum_user_count = 0;

void WebServer::SetNonBlock(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
}
void WebServer::AddFd(int fd, bool one_shot)
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
	if (one_shot)
		event.events |= EPOLLONESHOT;
	serv_epollfd.Add(event, fd);
	SetNonBlock(fd);
}


void WebServer::AddSig(int sig, void(handler)(int), bool restart)
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
void WebServer::WebServerInit(int connfd, const sockaddr_in &client_address)
{
	int error = 0;
	socklen_t len = sizeof(error);
	/*处理错误*/
	if (getsockopt(connfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
		throw __LINE__;
	AddFd(connfd, true);
	WebServer::sum_user_count++;
	users[connfd].init();
	users[connfd].http_sockfd = connfd;
	users[connfd].http_address = client_address;
}
void WebServer::WebServer_closefd(int epollfd, int connfd)
{
	if (users[connfd].http_sockfd != -1)
	{
		users[connfd].http_sockfd = -1;
		this->sum_user_count--;
		Epoll_ctl(epollfd, EPOLL_CTL_DEL, connfd, 0);
		Close(connfd);
	}
}
int WebServer::run(const char *ip, const int port)
{
	/*忽略SIGPIPE信号(SIG_IGN表示忽略SIGPIPE那个注册的信号)*/
	AddSig(SIGPIPE, SIG_IGN);

	ThreadPool pool(4);

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

	http_conn::http_epollfd = serv_epollfd.GetEpollFd();
	int epollfd = serv_epollfd.GetEpollFd();
	// epoll_event events[MAX_EVENT_NUMBER];
	// int epollfd = Epoll_create(5);

	// AddFd(epollfd, listenfd, false);
	{
		struct epoll_event event;
		event.data.fd = listenfd;
		event.events = EPOLLIN | EPOLLRDHUP;
		serv_epollfd.Add(event, listenfd);
		//Epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &event);
		SetNonBlock(listenfd);
	}
	//http_conn::http_epollfd = epollfd;

	while (true)
	{
		errno = 0;
		int number = serv_epollfd.Wait();
		perror("epoll_wait");
		for (int i = 0; i < number; i++)
		{
			int sockfd = serv_epollfd.GetFdByIndex(i);

			if (sockfd == listenfd) /*有新的连接到来*/
			{
				printf("尝试新的链接\n");

				struct sockaddr_in client_address;
				socklen_t client_len = sizeof(client_address);
				int connfd = Accept(listenfd, (struct sockaddr *)&client_address, &client_len);

				printf("connfd == %d , sum_user_count == %d\n",connfd,sum_user_count);

				if (sum_user_count >= MAX_FD)
				{
					/*发送一个服务器繁忙的信息过去,待完成*/
					continue;
				}
				/*如果不存在会直接插入 map<connfd,http_conn> 进去*/
				WebServerInit(connfd, client_address);
			}
			/*出错*/
			else if (serv_epollfd.GetEventsByIndex(i) & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
			{
				/*如果有异常，直接关闭客户连接*/
				WebServer_closefd(epollfd, sockfd);
			}
			/*读事件*/
			else if (serv_epollfd.GetEventsByIndex(i) & EPOLLIN)
			{
				printf("读事件\n");
				if (users[sockfd].read())
				{
					printf("读事件　完成！！！\n");
					//if (!pool.append(&users[sockfd]))
					if (!pool.append(std::bind(&http_conn::process, &users[sockfd])))
					{
						printf("append 失败！！！！\n");
					}
				}
				else
				{
					printf("读事件失败\n");
					WebServer_closefd(epollfd, sockfd);
				}
			}
			/*写事件*/
			else if (serv_epollfd.GetEventsByIndex(i) & EPOLLOUT)
			{
				printf("写事件\n");
				if (!users[sockfd].write())
					WebServer_closefd(epollfd, sockfd);
				printf("*****************写事件完成\n");
			}
			/*其他*/
			else
			{
			}
		}
	}
	Close(epollfd);
	return 0;
}
#endif