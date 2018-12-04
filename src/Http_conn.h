/*************************************************************************
	> File Name: HttpConn.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年10月27日 星期六 10时23分18秒
 ************************************************************************/

#ifndef _HTTP_CONN_H
#define _HTTP_CONN_H

#include "../base/Http_parse.h"
#include "../base/Socket.h"
#include "../base/Server_init.h"

const char *ok_200_title = "OK";
const char *error_500_title = "Serverr error";
const char *error_500_path = "./500.html";
const char *error_404_path = "./404.html";

/*任务类*/
class HttpConn
{
  public:
	/*读缓冲区大小*/
	static const int READ_BUFFERSIZE = 2048;
	/*写头部缓冲区大小*/
	static const int HEADER_BUFFERSIZE = 1024;

	/*该连接的sockfd和对方的地址*/
	int http_sockfd = 0;
	sockaddr_in http_address;
	static int http_epollfd;

  public:
	HttpConn() : http_data_pack(), Sockfd(false)
	{
		HttpInit();
	}
	HttpConn(const HttpConn &conn) = delete;
	HttpConn &operator=(const HttpConn &) = delete;
	~HttpConn() {}

	inline void HttpInit()
	{
		Sockfd.SetFd(http_sockfd);
		http_end_index = 0;
		http_header_index = 0;
		memset(http_read_buf, '\0', READ_BUFFERSIZE);
		memset(http_header_buf, '\0', HEADER_BUFFERSIZE);
		http_have_sended = 0;
	}
	inline void HttpClose()
	{
		if (http_sockfd != -1)
		{
			int ret = epoll_ctl(http_epollfd, EPOLL_CTL_DEL, http_sockfd, 0);
			if (ret < 0)
				throw CallFailed("Http_conn.cc 文件: epoll_ctl function failed !!! at line  ", __LINE__);
			Sockfd.Close();
			http_sockfd = -1;
			// sum_user_count--; /*关闭一个连接减去一个用户*/
		}
	}

	/*循环读取客户数据，直到无数据可读或者对方关闭连接*/
	inline bool HttpRead()
	{
		//printf("进入　read 函数 \n");
		//printf("http_end_index ==%d\n", http_end_index);
		if (http_end_index >= READ_BUFFERSIZE)
			return false;
		bool ret = Sockfd.RecvAll(http_read_buf, READ_BUFFERSIZE, http_end_index, SOCK_NONBLOCK);
		return ret;
		//printf("%s", http_read_buf);
		//printf("http_end_index == %d\n", http_end_index);
	}
	/*
	触发写事件,响应请求*/
	bool HttpWrite()
	{
		printf(" 进入http_conn::write函数\n");
		Sockfd.Sendlen(http_header_buf, strlen(http_header_buf), SOCK_NONBLOCK); /*非阻塞发送*/
		printf("出 send_header 函数\n ");

		File file(http_data_pack.GetFileName(), O_RDONLY);
		Sockfd.Sendfile(file.GetFileFd(), &http_have_sended, file.Size() - http_have_sended, http_have_sended);

		printf("  出sendfile 函数\n");

		if (http_data_pack.IsKeep())
		{
			HttpInit();
			Modfd(EPOLLIN);
			return true;
		}
		else
		{
			Modfd(EPOLLIN);
			return false;
		}
	}

	/*处理客户请求*/
	void Process()
	{
		HTTP_CODE read_ret = http_data_pack.HttpDataRead(http_read_buf, http_end_index);
		if (read_ret == RE_NOT_ENOUGH)
		{
			Modfd(EPOLLIN);
		}
		else
		{
			bool write_ret = ProcessWrite(read_ret);
			if (!write_ret)
			{
				HttpClose();
			}
			printf("修改事件类型\n");
			Modfd(EPOLLOUT); /*这里触发写事件*/
		}
	}

  private:
	inline void Modfd(const int &ev)
	{
		epoll_event event;
		event.data.fd = http_sockfd;
		event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
		int ret = epoll_ctl(http_epollfd, EPOLL_CTL_MOD, http_sockfd, &event);
		if (ret < 0)
			throw CallFailed("Http_conn.cc 文件: epoll_ctl function failed !!! at line  ", __LINE__);
	}
	void AddStatueLine(int status, const char *title)
	{
		http_header_index = 0;
		//printf("1.addd_status_line : http_header_buf ==%s\n", http_header_buf);
		int ret = snprintf(http_header_buf, HEADER_BUFFERSIZE - 1, "HTTP/1.1 %d %s\r\n", status, title);
		http_header_index += ret;
		//printf("2.addd_status_line : http_header_buf ==%s\n", http_header_buf);
	}
	void AddHeader()
	{
		int &index = http_header_index;
		int ret = snprintf(http_header_buf + index,
						   HEADER_BUFFERSIZE - 1 - index,
						   "Connection: %s\r\n",
						   (http_data_pack.IsKeep() == true)
							   ? "keep-alive"
							   : "close");
		index += ret;
		/*添加文件类型*/
		std::string tmp = http_data_pack.GetFileName();

		std::string key;
		auto idx = tmp.rfind('.');

		if (idx != std::string::npos)
		{
			key = tmp.substr(idx);
			ret = snprintf(http_header_buf + index,
						   HEADER_BUFFERSIZE - 1 - index,
						   "%s\r\n;%s", ConfigValue::Type[key].c_str(), "charset=utf-8\r\n");
		}
		index += ret;

		/*添加空白行*/
		ret = snprintf(http_header_buf + index,
					   HEADER_BUFFERSIZE - 1 - index,
					   "%s", "\r\n");
		index += ret;

		printf("addd_header : http_header_buf ==%s\n", http_header_buf);
	}
	/*ProcessWrite 所使用的函数*/
	bool ProcessWrite(HTTP_CODE ret)
	{
		//printf("进入process_write 函数\n");

		switch (ret)
		{
		case FILE_RE: /*文件请求*/
		{
			Response();
			/*没有修改事件类型，所以没有任何的发送的情况*/
			break;
		}
		case SERVER_ERROR:
		{
			break;
		}
		default:
		{
		}
		}
		//printf("出　process_write 函数\n");
		return true;
	}
	void Response()
	{
		AddStatueLine(200, ok_200_title);
		switch (http_data_pack.GetMethod())
		{
		case (GET):
			ResponseGet(); /*处理静态文件和动态文件*/
			break;
		case (POST):
			ResponsePost();
			break;
		default:
			std::cout << "暂时不支持该方法" << std::endl;
			break;
		}
	}
	void ResponseGet()
	{
		// if (http_data_pack.IsDynamic()) /*fastcgi*/
		// {
		// }
		// else
		// {
		AddHeader();
		// }
	}
	void ResponsePost()
	{
	}

  private:
	/*http 解析类*/
	HttpParse http_data_pack;
	BaseSocket Sockfd;

	/*读缓冲区*/
	char http_read_buf[READ_BUFFERSIZE] = {0};
	/*读缓冲区中已经读入的最后一个字节的位置*/
	int http_end_index = 0;
	char http_header_buf[HEADER_BUFFERSIZE] = {0};
	/*构造写缓冲*/
	int http_header_index = 0;
	/*已经发送了多少字节*/
	ssize_t http_have_sended = 0;
};

int HttpConn::http_epollfd = -1;

#endif
