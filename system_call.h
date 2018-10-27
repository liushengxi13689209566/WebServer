/*************************************************************************
	> File Name: system_call.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年10月27日 星期六 16时57分27秒
 ************************************************************************/

#ifndef _SYSTEM_CALL_H
#define _SYSTEM_CALL_H
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <sys/select.h>
#include <strings.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <netdb.h>
#include <math.h>
#include <pthread.h>

/*基本网络编程函数封装*/
int Socket(int family, int type, int protocol);
int Bind(int fd, const struct sockaddr *sa, socklen_t salen);
void Listen(int fd, int backlog);
int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr);
void Close(int fd);
void Setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen);

/*epoll*/
int Epoll_create(int size);
#endif
