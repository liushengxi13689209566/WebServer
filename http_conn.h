/*************************************************************************
	> File Name: http_conn.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年10月27日 星期六 10时23分18秒
 ************************************************************************/

#ifndef _HTTP_CONN_H
#define _HTTP_CONN_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>

/*任务类*/
class http_conn
{
  public:
	/*文件名最大长度*/
	static const int FILENAME_LEN = 200;
	/*读与写的缓冲区大小*/
	static const int BUFFERSIZE = 2048;
	/* http 请求方法，目前之支持 get & post*/
	enum METHOD
	{
		GET = 0,
		POST = 0,
		HEAD,
		PUT,
		DELETE,
		TRACE,
		OPTIONS,
		CONNECT,
		PATCH
	};
	/*解析客户的请求时，主状态机所处的几种状态*/
	enum CHECK_STATE
	{
		CHECK_STATE_REQUESTLINE,
		CHECK_STATE_HEADER,
		CHECK_STATE_CONTENT
	};
	/*服务器处理　http 请求的结果 */
	enum HTTP_CODE
	{
		REQUEST_NOT_ENOUGH, /*请求不完整，继续读取客户端*/
		ENOUGH_REQUEST,		/*读取到一个完整请求*/
		BAD_REQUEST,		/*请求有语法错误*/
		FORBIDDEN_REQUEST,  /*客户对该资源无权限*/
		SERVER_ERROR,		/*服务器出错*/
		CLOSED_CONNECTION   /*客户端关闭了连接*/
	};
	/*行的读取状态：有可能一行数据都没有一次性传输完毕，需要继续进行传输*/
	enum LINE_STATUS
	{
		LINE_OK = 0,
		LINE_BAD,
		LINE_NOT_ENOUGH
	};

  public:
	http_conn() {}
	~http_conn() {}

  public:
  	/**/
	void init(int sockfd, const sockaddr_in &addr);
};
void http_conn::init(int sockfd, const sockaddr_in &addr)
{

}
#endif
