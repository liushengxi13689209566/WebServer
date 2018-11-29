/*************************************************************************
	> File Name: http_conn.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年10月27日 星期六 10时23分18秒
 ************************************************************************/

#ifndef _HTTP_CONN_H
#define _HTTP_CONN_H

#include "../base/Http_parse.h"
#include "../base/Socket.h"
#include "./base_function.h"

const char *ok_200_title = "OK";
const char *error_500_title = "Serverr error";
const char *error_500_path = "./500.html";
const char *error_404_path = "./404.html";

/*任务类*/
class http_conn
{
  public:
	/*文件名最大长度*/
	static const int FILENAME_LEN = 200;
	/*读缓冲区大小*/
	static const int READ_BUFFERSIZE = 2048;
	/*写头部缓冲区大小*/
	static const int HEADER_BUFFERSIZE = 1024;

  public:
	http_conn() : http_data_pack(), Sockfd(false)
	{
		init();
	}
	http_conn(const http_conn &conn) = delete;
	http_conn &operator=(const http_conn &) = delete;
	~http_conn()
	{
	}

  public:
	void init()
	{
		Sockfd.SetFd(http_sockfd);
		http_end_index = 0;
		http_header_index = 0;
		memset(http_read_buf, '\0', READ_BUFFERSIZE);
		memset(http_header_buf, '\0', HEADER_BUFFERSIZE);
		http_have_sended = 0;
	}
	bool read();
	bool write();
	/*write 调用的函数*/
	bool send_header();

	/*处理客户请求*/
	void process();

  private:
	void modfd(int ev);
	void http_close_conn();

	/*process_write 所使用的函数*/
	bool process_write(HTTP_CODE ret);
	void add_status_line(int status, const char *title);
	void add_header();

  public:
	/*该连接的sockfd和对方的地址*/
	int http_sockfd = 0;
	sockaddr_in http_address;
	static int http_epollfd;

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

int http_conn::http_epollfd = -1;

void http_conn::modfd(int ev)
{
	epoll_event event;
	event.data.fd = http_sockfd;
	event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
	Epoll_ctl(http_epollfd, EPOLL_CTL_MOD, http_sockfd, &event);
}
/*循环读取客户数据，直到无数据可读或者对方关闭连接*/
bool http_conn::read()
{
	//printf("进入　read 函数 \n");
	//printf("http_end_index ==%d\n", http_end_index);
	if (http_end_index >= READ_BUFFERSIZE)
		return false;
	bool ret = Sockfd.RecvAll(http_read_buf, READ_BUFFERSIZE, http_end_index, SOCK_NONBLOCK);
		return ret ;
	//printf("%s", http_read_buf);
	printf("http_end_index == %d\n", http_end_index);
}

/*
触发　写事件　，进行写的操作．
先发送头部（头部信息已经构建好了［在http_header_buf中］）过去，然后调用 sendfile 将请求所对应的文件发送过去　
考虑是否保存　连接
*/
/*响应请求*/
bool http_conn::write()
{
	printf(" 进入http_conn::write函数\n");
	Sockfd.Sendlen(http_header_buf, strlen(http_header_buf), SOCK_NONBLOCK); /*非阻塞发送*/
	printf("出 send_header 函数\n ");

	printf(" 进入sendfile 函数\n");
	printf(" 进入//////////fielname=%s\n", http_data_pack.GetFileName());

	File file(http_data_pack.GetFileName(), O_RDONLY);
	Sockfd.Sendfile(file.GetFileFd(), &http_have_sended, file.Size() - http_have_sended, http_have_sended);

	printf("  出sendfile 函数\n");

	if (http_data_pack.IsKeep())
	{
		init();
		modfd(EPOLLIN);
		return true;
	}
	else
	{
		modfd(EPOLLIN);
		return false;
	}
}

void http_conn::http_close_conn()
{
	if (http_sockfd != -1)
	{
		Epoll_ctl(http_epollfd, EPOLL_CTL_DEL, http_sockfd, 0);
		Close(http_sockfd);
		http_sockfd = -1;
		//WebServer::sum_user_count--; /*必须解决的问题 */
	}
}

void http_conn::add_status_line(int status, const char *title)
{
	http_header_index = 0;
	//printf("1.addd_status_line : http_header_buf ==%s\n", http_header_buf);
	int ret = snprintf(http_header_buf, HEADER_BUFFERSIZE - 1, "HTTP/1.1 %d %s\r\n", status, title);
	http_header_index += ret;
	//printf("2.addd_status_line : http_header_buf ==%s\n", http_header_buf);
}
void http_conn::add_header()
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
	if (tmp.find(".html") != std::string::npos)
		ret = snprintf(http_header_buf + index,
					   HEADER_BUFFERSIZE - 1 - index,
					   "%s", "Content-Type: text/html\r\n;charset=utf-8\r\n");
	index += ret;

	/*添加空白行*/
	ret = snprintf(http_header_buf + index,
				   HEADER_BUFFERSIZE - 1 - index,
				   "%s", "\r\n");
	index += ret;

	//printf("addd_header : http_header_buf ==%s\n", http_header_buf);
}

bool http_conn::process_write(HTTP_CODE ret)
{
	//printf("进入process_write 函数\n");

	switch (ret)
	{
	case FILE_RE:
	{
		add_status_line(200, ok_200_title);
		add_header();
		/*从这里下来，就构造好了http的头部，但是因为没有修改事件类型，所以没有任何的发送的情况*/
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
void http_conn::process()
{
	HTTP_CODE read_ret = http_data_pack.HttpDataRead(http_read_buf, http_end_index);
	if (read_ret == RE_NOT_ENOUGH)
	{
		modfd(EPOLLIN);
		return;
	}
	bool write_ret = process_write(read_ret);
	if (!write_ret)
	{
		http_close_conn();
	}
	printf("修改事件类型\n");
	modfd(EPOLLOUT); /*这里触发写事件*/
}
#endif
