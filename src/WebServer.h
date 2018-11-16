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

#include "./http_conn.h"
#include "./threadPool.h"
#include "./base_function.h"

const int MAX_FD = 65536;
const int MAX_EVENT_NUMBER = 10000;

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
	void Setnonblock(int fd);
	void addfd(int epollfd, int fd, bool one_shot);
	void addsig(int sig, void(handler)(int), bool restart = true);
	void WebServer_init(int epollfd, int connfd, const sockaddr_in &client_address);
	void WebServer_closefd(int epollfd, int connfd);

  private:
	static int listenfd;
	std::unordered_map<int, http_conn> users; /*任务类对象*/

  public:
	static int sum_user_count; /*总用户*/
};

int WebServer::listenfd = 0;
int WebServer::sum_user_count = 0;

#endif
