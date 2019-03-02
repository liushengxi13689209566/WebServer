/*************************************************************************
	> File Name: http_conn.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年07月23日 星期一 15时43分24秒
 ************************************************************************/

#ifndef _HTTP_CONN_H
#define _HTTP_CONN_H
#include <iostream>
#include <sys/wait.h>
#include <fstream>
#include <vector>
#include <string>
#include <libgen.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <signal.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/sendfile.h>
#include <error.h>
#include <vector>
#include <queue>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <sstream>
#include <sys/stat.h>
#include "ThreadPool.h"

#define BUFFER_SIZE 4096
#define CHAR_MAX_SIZE 512
#define PATH_404 "./HTML/404.html"
#define PATH_501 "./HTML/501.html"
#define PATH_666 "./HTML/666.html"
#define ROOT_PATH "./"
#define PATH_404_PNG "./Picture/404.png"
#define PATH_ICO "./Picture/666.ico"
#define MAX_EVENT_NUMBER 1024
class WebServer
{
  public:
	WebServer(const std::string ip, const int port);
	~WebServer();

  private:
	static int worker(int fd,int epollfd ); //线程函数

	int start(const std::string ip, const int port);
	int conn_fd;
	static int reponse_GET(std::string filename, const int fd);
	static int reponse_POST(std::string filename, std::string dat, const int fd);
	static int reponse_noKnow(const int fd);
	static int make_headers(const int statue, const int fd, std::string filename);

	static int not_found(const int fd, std::string filename);
	static int send_file(const int fd, std::string filename); //调用sendfile函数传送文件
	static std::string Analysis_POST(std::string recv_buffer);

	static void addfd(int epollfd, int fd, bool oneshot);
	static int setnonblocking (int fd);
	static void reset_oneshot(int epollfd,int fd);
};
#endif
