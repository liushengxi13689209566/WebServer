#ifndef _EPOLL_H
#define _EPOLL_H

#include "./base.hpp"
//#include "./Server_init.h"
const int MAX_EVENTS_NUMBER = 10000;
class Epoll
{
  public:
    Epoll()
    {
        this->epollfd_ = epoll_create(5); /*参数大于0即可*/
        if (this->epollfd_ < 0)
            throw CallFailed("Epoll.hpp 文件：epoll_create function failed !!! at line  ", __LINE__);
    }
    Epoll(const Epoll &) = delete;
    Epoll &operator=(const Epoll &) = delete;

    virtual ~Epoll()
    {
        close(epollfd_); /*不抛异常*/
    }

    inline int Add(struct epoll_event &ev, const int &fd)
    {
        int ret = epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev);
        if (ret < 0)
            throw CallFailed("Epoll.hpp 文件：epoll_ctl function failed !!! at line  ", __LINE__);
        return (ret);
    }
    inline int RemoveEvent(struct epoll_event &ev, int &fd)
    {
        int ret = epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, &ev);
        if (ret < 0)
            throw CallFailed("Epoll.hpp 文件：epoll_ctl function failed !!! at line  ", __LINE__);
        return (ret);
    }
    inline int RemoveFd(int &fd)
    {
        int ret = epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, 0);
        if (ret < 0)
            throw CallFailed("Epoll.hpp 文件：epoll_ctl function failed !!! at line  ", __LINE__);
        return (ret);
    }
    inline int ModifyEvent(struct epoll_event &ev, int &fd)
    {
        int ret = epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &ev);
        if (ret < 0)
            throw CallFailed("Epoll.hpp 文件：epoll_ctl function failed !!! at line  ", __LINE__);
        return (ret);
    }
    inline int Wait()
    {
        /* -1: 代表阻塞　*/
        int ret = epoll_wait(epollfd_, events, MAX_EVENTS_NUMBER, -1);
        /*epoll_wait　出错　*/
        if ((ret < 0) && (errno != EINTR))
            throw CallFailed("Epoll.hpp 文件：epoll_wait function failed !!! at line  ", __LINE__);
        return (ret);
    }
    inline int GetEpollFd() { return epollfd_; }

    inline int GetFdByIndex(const int &index)
    {
        return events[index].data.fd;
    }
    inline struct epoll_event *GetEventAddressByIndex(const int &index)
    {
        return &events[index];
    }
    inline uint32_t GetEventsByIndex(const int &index)
    {
        return events[index].events;
    }

  private:
    int epollfd_ = -1;
    struct epoll_event events[MAX_EVENTS_NUMBER];
};
#endif
