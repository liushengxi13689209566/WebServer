#include "../base_function.h"

int Socket(int &family, int &type, int &protocol)
{
    int n;
    if ((n = socket(family, type, protocol)) < 0)
        throw __LINE__;
    return (n);
}
int Bind(int &fd, const struct sockaddr *sa, socklen_t salen)
{
    if (bind(fd, sa, salen) < 0)
        throw __LINE__;
    return 0;
}
void Listen(int &fd, int &backlog)
{
    char *ptr = NULL;
    if ((ptr = getenv("LISTENQ")) != NULL)
        backlog = atoi(ptr);

    if (listen(fd, backlog) < 0)
        throw __LINE__;
}
int Accept(int &fd, struct sockaddr *sa, socklen_t *salenptr)
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
void Close(int &fd)
{
    if (close(fd) == -1)
        throw __LINE__;
}
ssize_t Sendlen(int &fd, const void *buf, size_t len, int &flags)
{
    ssize_t n = 0;
    size_t sum = 0;
    const char *ptr;
    ptr = (const char *)buf;
    while (sum < len)
    {
        n = send(fd, (void *)ptr, len - sum, flags);
        if (n < 0)
        {
            if (errno == EINTR)
                n = 0;
            else
                throw __LINE__;
        }
        sum += n;
        ptr += n;
    }
    return (sum);
}
void Setsockopt(int &fd, int &level, int &optname, const void *optval, socklen_t optlen)
{
    if (setsockopt(fd, level, optname, optval, optlen) < 0)
        throw __LINE__;
}
/**********************************************************************************************************/
int Epoll_create(int &size)
{
    int ret = 0;
    if ((ret = epoll_create(5)) == -1)
        throw __LINE__;
    return (ret);
}
int Epoll_ctl(int &epfd, int &op, int &fd, struct epoll_event *event)
{
    int ret = 0;
    if ((ret = epoll_ctl(epollfd, op, fd, event)) == -1)
        throw __LINE__;
    return (ret);
}
int Epoll_wait(int &epfd, struct epoll_event *events, int &maxevents, int &timeout)
{
    int ret = epoll_wait(epollfd, events, maxevents, timeout);
    /*epoll_wait　出错　*/
    if ((ret < 0) && (errno != EINTR))
        throw __LINE__;
    return (ret);
}