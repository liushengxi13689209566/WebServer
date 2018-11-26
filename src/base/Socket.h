/*************************************************************************
	> File Name: ServSocket.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年11月18日 星期日 16时14分39秒
 ************************************************************************/

#ifndef _SERVSOCKET_H
#define _SERVSOCKET_H

#include <sys/sendfile.h>
#include "./base.hpp"

class BaseSocket
{
  public:
    BaseSocket()
    {
        base_socket_ = socket(AF_INET, SOCK_STREAM, 0);
        if (base_socket_ < 0)
            throw CallFailed(" Socket.hpp 文件: socket create failed !!! at line  ", __LINE__);
    }
    BaseSocket(bool flag) {}
    BaseSocket(const BaseSocket &rfs) { base_socket_ = rfs.base_socket_; }
    BaseSocket(const int fd) { base_socket_ = fd; }

    void SetFd(const int &fd)
    {
        base_socket_ = fd;
    }
    inline void Close()
    {
        if (close(base_socket_) == -1)
            throw CallFailed(" Socket.hpp 文件: close function failed !!! at line  ", __LINE__);
    }

    BaseSocket &operator=(const BaseSocket &) = delete;

    //ssize_t send(int socket, const void *buffer, size_t length, int flags) ;
    inline int Sendlen(const void *buf, size_t len, int flags = SOCK_NONBLOCK)
    {
        ssize_t n = 0;
        size_t sum = 0;
        const char *ptr;
        ptr = (const char *)buf;
        while (sum < len)
        {
            n = send(base_socket_, (void *)ptr, len - sum, flags);
            if (n < 0)
            {
                if (errno == EINTR)
                    n = 0;
                else
                    throw CallFailed(" Socket.hpp 文件: sendlen function failed !!! at line  ", __LINE__);
            }
            sum += n;
            ptr += n;
        }
        return (sum);
    }
    /*暂时写成循环
	ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count);
    http_have_sended 必须是引用　
	*/
    inline bool Sendfile(int in_fd, off_t *offset, size_t count, ssize_t &http_have_sended)
    {
        ssize_t bytes_have_send = 0;
        ssize_t ret = 0;
        while (bytes_have_send != count)
        {
            if (bytes_have_send > count)
                break;
            ret = sendfile(base_socket_, in_fd, offset, count - bytes_have_send);
            if (ret == -1)
            {
                if (errno == EAGAIN)
                    continue;
                else
                {
                    return false;
                }
            }
            else
            {
                bytes_have_send += ret;
                http_have_sended += ret;
            }
        }
        return true;
    }
    inline int RecvAll(void *buffer, size_t length, int &index, int flags = SOCK_NONBLOCK)
    {
        int readed_bytes = 0;
        char *buffer_pointer = reinterpret_cast<char *>(buffer);
        while (true)
        {
            //printf("开始　recv \n");
            readed_bytes = recv(base_socket_, buffer_pointer + index, length - index, flags);
            if (readed_bytes == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    return true; /*无数据可读，返回　true */
                else
                    return false;
            }
            else if (readed_bytes == 0) /*代表对端关闭*/
                return false;
            index += readed_bytes;
        }
        return (index);
    }
    inline int GetBaseSocket() { return base_socket_; }

  protected:
    int base_socket_ = -1;
};

class ServSocket : public BaseSocket
{
    /*继承了 base_socket_ */
  public:
    ServSocket() = delete;
    ServSocket(const ServSocket &) = delete;
    ServSocket &operator=(const ServSocket &) = delete;

    explicit ServSocket(const char *ip, const int port)
    {
        bzero(&serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        inet_pton(AF_INET, ip, &serv_addr.sin_addr);
        serv_addr.sin_port = htons(port);
    }
    inline void SetReuse()
    {
        int optval = 1;
        if (setsockopt(base_socket_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) < 0)
            throw CallFailed(" Socket.hpp 文件: setsockopt function failed !!! at line  ", __LINE__);
    }
    inline int Bind()
    {
        /* Bind(listenfd, (struct sockaddr *)&address, sizeof(address));**/
        if (bind(base_socket_, reinterpret_cast<struct sockaddr *>(&serv_addr), sizeof(serv_addr)) < 0)
            throw CallFailed(" Socket.hpp 文件: bind function failed !!! at line  ", __LINE__);
        return (base_socket_);
    }
    inline int Listen()
    {
        int backlog = 1024;
        char *ptr = NULL;
        if ((ptr = getenv("LISTENQ")) != NULL)
            backlog = atoi(ptr);

        if (listen(base_socket_, backlog) < 0)
            throw CallFailed(" Socket.hpp 文件: listen function failed !!! at line  ", __LINE__);
        return (base_socket_);
    }
    inline int Accept(struct sockaddr *sa, socklen_t *salenptr)
    {
        int n;
    again:
        if ((n = accept(base_socket_, sa, salenptr)) < 0)
        {
            /*连接在listen后建立，在accept前夭折，此时accept可能会返回错误，对此必须处理,*/
#ifdef EPROTO
            if (errno == EPROTO || errno == ECONNABORTED || errno == EWOULDBLOCK)
#else
            if (errno == ECONNABORTED)
#endif
                goto again;
            else
                throw CallFailed(" Socket.hpp 文件: accept function failed !!! at line  ", __LINE__);
        }
        return (n);
    }

  private:
    struct sockaddr_in serv_addr;
};
#endif
