/*************************************************************************
	> File Name: socket.cpp
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年10月27日 星期六 16时59分51秒
 ************************************************************************/

#include "../system_call.h"

int Socket(int family, int type, int protocol)
{
	int n;
	if ((n = socket(family, type, protocol)) < 0)
		throw __LINE__;
	return (n);
}
int Bind(int fd, const struct sockaddr *sa, socklen_t salen)
{
	if (bind(fd, sa, salen) < 0)
		throw __LINE__;
	return 0;
}
void Listen(int fd, int backlog)
{
	char *ptr = NULL;
	if ((ptr = getenv("LISTENQ")) != NULL)
		backlog = atoi(ptr);

	if (listen(fd, backlog) < 0)
		throw __LINE__;
}
int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
	int n;
again:
	if ((n = accept(fd, sa, salenptr)) < 0)
	{
		/*连接在listen后建立，在accept前夭折，此时accept可能会返回错误，对此必须处理,*/
#ifdef EPROTO
		if (errno == EPROTO || errno == ECONNABORTED)
#else
		if (errno == ECONNABORTED)
#endif
			goto again;
		else
			throw __LINE__;
	}
	return (n);
}
/**********************************************************************************************************/
int Epoll_create(int size)
{
	if (epoll_create(5) == -1)
		throw __LINE__;
}
int Epoll_ctl(int epollfd, int op, int fd, struct epoll_event *event)
{
	if (epoll_ctl(epollfd, op, fd, event) == -1)
		throw __LINE__;
}
int Epoll_wait(int epollfd, struct epoll_event *events, int maxevents, int timeout)
{
	int ret = epoll_wait(epollfd, events, maxevents, timeout);
	/*epoll_wait　出错　*/
	if ((ret < 0) && (errno != EINTR))
		throw __LINE__;
}
