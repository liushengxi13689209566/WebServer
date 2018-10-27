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
/***********************************************************************************************/
int Epoll_create(int size)
{
	if (epoll_create(5) == -1)
		throw __LINE__;
}
