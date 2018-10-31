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
	http_conn() : http_check_state(CHECK_STATE_REQUESTLINE),
				  http_keep_connect(false),
				  http_method(GET),
				  http_url(nullptr),
				  http_version(nullptr),
				  http_content_length(0),

				  http_host(nullptr),
				  http_start_line(0),
				  http_checked_index(0),
				  http_end_index(0),
				  http_write_index(0),
				  http_read_buf("\0"),
				  http_write_buf("\0"),
				  http_real_file("\0")

	{
	}
	~http_conn() {}

  public:
	bool read();

  public:
	/*该连接的sockfd和对方的地址*/
	int http_sockfd = 0;
	sockaddr_in http_address;

  private:
	/*读缓冲区*/
	char http_read_buf[BUFFERSIZE] = {0};
	/*读缓冲区中已经读入的最后一个字节的位置*/
	int http_end_index = 0;
	/*正在分析的字符在读缓冲区中的位置*/
	int http_checked_index = 0;
	/*正在解析的行的起始位置*/
	int http_start_line = 0;
	/*写缓冲区*/
	char http_write_buf[BUFFERSIZE] = {0};
	/*写缓冲区中待发送的字节数*/
	int http_write_index = 0;

	/*主状态机所处的状态*/
	CHECK_STATE http_check_state;
	/*请求的方法*/
	METHOD http_method;

	/*客户请求的目标文件的完整的路径，＝　root+url */
	char http_real_file[FILENAME_LEN] = {0};
	/*url*/
	char *http_url = nullptr;
	/*版本*/
	char *http_version = nullptr;
	/*主机名*/
	char *http_host = nullptr;
	/*http请求的消息体的长度*/
	int http_content_length = 0;
	/*是否需要保持连接*/
	bool http_keep_connect = false;
};
/*循环读取客户数据，直到无数据可读或者对方关闭连接*/
bool http_conn::read()
{
	if (http_end_index >= BUFFERSIZE)
		return false;
	int readed_bytes = 0;
	while (true)
	{
		readed_bytes = recv(http_sockfd, http_read_buf + http_end_index, BUFFERSIZE - http_end_index, 0);
		if (readed_bytes == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return true; /*无数据可读，返回　true */
			else
				return false;
		}
		else if (readed_bytes == 0)
			return false;
		http_end_index += readed_bytes;
	}
	return true;
}

#endif
