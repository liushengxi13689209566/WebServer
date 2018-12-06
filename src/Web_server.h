/*************************************************************************
	> File Name: WebServer.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年10月27日 星期六 10时42分48秒
 ************************************************************************/

#ifndef _WEB_SERVER_H
#define _WEB_SERVER_H
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <netdb.h>
#include <math.h>
#include <memory>
#include <unordered_map>

#include "../base/Epoll.h"
#include "../base/Thread_pool.h"
#include "../base/Socket.h"

#include "./Http_conn.h"

const int MAX_FD = 65536;

/*服务器类*/
class WebServer
{
  public:
	WebServer() = delete;
	WebServer(const WebServer &) = delete;
	WebServer &operator=(const WebServer &) = delete;

	explicit WebServer(const char *ip, const int port) : serv_socket(ip, port), serv_epollfd() {}
	~WebServer() {}
	int run()
	{
		/*忽略SIGPIPE信号(SIG_IGN表示忽略SIGPIPE那个注册的信号)*/
		AddSig(SIGPIPE, SIG_IGN);

		ThreadPool pool(4);

		serv_socket.SetReuse();

		serv_socket.Bind();
		serv_socket.Listen();

		int listenfd = serv_socket.GetBaseSocket();
		HttpConn::http_epollfd = serv_epollfd.GetEpollFd();

		//int epollfd = serv_epollfd.GetEpollFd();

		struct epoll_event event;
		event.data.fd = listenfd;
		event.events = EPOLLIN | EPOLLRDHUP;
		serv_epollfd.Add(event, listenfd);
		SetNonBlock(listenfd);

		while (true)
		{
			int number = serv_epollfd.Wait();
			for (int i = 0; i < number; i++)
			{
				int sockfd = serv_epollfd.GetFdByIndex(i);

				if (sockfd == listenfd) /*有新的连接到来*/
				{
					printf("尝试新的链接\n");

					struct sockaddr_in client_address;
					socklen_t client_len = sizeof(client_address);
					int connfd = serv_socket.Accept(reinterpret_cast<struct sockaddr *>(&client_address), &client_len);
					// if (sum_user_count >= MAX_FD)
					// {
					//     /*发送一个服务器繁忙的信息过去,待完成*/
					//     continue;
					// }
					/*如果不存在会直接插入 map<connfd,HttpConn> 进去*/
					WebServerInit(connfd, client_address);
				}
				/*出错*/
				else if (serv_epollfd.GetEventsByIndex(i) & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
				{
					/*如果有异常，直接关闭客户连接*/
					perror("发生异常，关闭链接\n");
					WebServerClosefd(sockfd);
				}
				/*读事件*/
				else if (serv_epollfd.GetEventsByIndex(i) & EPOLLIN)
				{
					printf("读事件\n");
					if (users[sockfd].HttpRead())
					{
						printf("读事件　完成！！！\n");
						if (!pool.append(std::bind(&HttpConn::Process, &users[sockfd])))
						{
							printf("append 失败！！！！\n");
						}
					}
					else
					{
						printf("读事件失败\n");
						WebServerClosefd(sockfd);
					}
				}
				/*写事件*/
				else if (serv_epollfd.GetEventsByIndex(i) & EPOLLOUT)
				{
					printf("写事件\n");
					if (!users[sockfd].HttpWrite())
						WebServerClosefd(sockfd);
					printf("*****************写事件完成\n");
				}
				/*其他*/
				else
				{
				}
			}
		}
		return 0;
	}

  private:
	inline void SetNonBlock(const int &fd)
	{
		int old_option = fcntl(fd, F_GETFL);
		int new_option = old_option | O_NONBLOCK;
		fcntl(fd, F_SETFL, new_option);
	}
	inline void AddFd(const int &fd, bool one_shot)
	{

		struct epoll_event event;
		event.data.fd = fd;
		event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
		if (one_shot)
			event.events |= EPOLLONESHOT;
		serv_epollfd.Add(event, fd);
		SetNonBlock(fd);
	}
	void AddSig(int sig, void(handler)(int), bool restart = true)
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

	void WebServerInit(int connfd, const sockaddr_in &client_address)
	{
		int error = 0;
		socklen_t len = sizeof(error);
		/*处理错误*/
		if (getsockopt(connfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
			throw CallFailed("getsockopt function failed !!! at line  ", __LINE__);
		AddFd(connfd, true);
		//WebServer::sum_user_count++;
		users[connfd].http_sockfd = connfd;
		users[connfd].HttpInit();
		users[connfd].http_address = client_address;
	}
	inline void WebServerClosefd(const int &connfd)
	{
		users.at(connfd).HttpClose();
	}

  private:
	ServSocket serv_socket;
	Epoll serv_epollfd;
	std::unordered_map<int, HttpConn> users; /*任务类对象*/

  public:
	//static int sum_user_count; /*总用户*/
};

//int WebServer::sum_user_count = 0;

#endif
