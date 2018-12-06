
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
#include "../FastCGI/fcgi.h"

const char *ok_200_title = "OK";
const char *error_500_title = "Serverr error";
const char *error_500_path = "./500.final_html";
const char *error_404_path = "./404.final_html";

/*任务类*/
class HttpConn
{
  public:
	/*读缓冲区大小*/
	static const int READ_BUFFERSIZE = 2048;
	/*写头部缓冲区大小*/
	static const int HEADER_BUFFERSIZE = 1024;
	/*由php翻译过来的　html　文件　*/
	static const int WRITE_BUFFERSIZE = 20000;

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
	~HttpConn()
	{
		FastCgi_finit(&fast_cgi);
	}

	inline void HttpInit()
	{
		Sockfd.SetFd(http_sockfd);
		http_end_index = 0;
		http_header_index = 0;
		memset(http_read_buf, '\0', READ_BUFFERSIZE);
		memset(http_header_buf, '\0', HEADER_BUFFERSIZE);
		memset(final_html, '\0', WRITE_BUFFERSIZE);
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
		if (http_end_index >= READ_BUFFERSIZE)
			return false;
		bool ret = Sockfd.RecvAll(http_read_buf, READ_BUFFERSIZE, http_end_index, SOCK_NONBLOCK);
		return ret;
	}
	/*触发写事件,响应请求*/
	bool HttpWrite()
	{
		printf(" 进入http_conn::write函数\n");
		Sockfd.Sendlen(http_header_buf, strlen(http_header_buf), SOCK_NONBLOCK); /*非阻塞发送*/
		printf("出 send_header 函数\n ");

		if (strlen(final_html) != 0)
		{
			/*这里需要考虑一下，翻译出来的html如果很大很大怎么办的问题*/
			Sockfd.Sendlen(final_html, strlen(final_html), SOCK_NONBLOCK); /*非阻塞发送*/
		}
		else
		{
			File file(http_data_pack.GetFileName(), O_RDONLY);
			Sockfd.Sendfile(file.GetFileFd(), &http_have_sended, file.Size() - http_have_sended, http_have_sended);
		}

		// printf(" 发送fina_html 文件    !!!!!!!!!!!!!!!!!!!!!!!!!完毕　\n");

		// printf("  出sendfile 函数\n");

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
	/*接口设计得改变*/
	void AddHeader(std::string content_type = ".html")
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
		std::string key;
		auto idx = content_type.rfind('.');
		if (idx != std::string::npos)
		{
			key = content_type.substr(idx);
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
		printf("进入process_write 函数\n");

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
	/*有三种情况，1（html）　2　(php)　3　(url中带有参数)*/
	void ResponseGet()
	{
		/*单纯的请求了服务器的php文件*/
		if (http_data_pack.IsPhp() && !http_data_pack.IsDynamic())
		{
			FastCgiFun(const_cast<char *>("GET"), false);
			char *full_result;
			char *html;
			full_result = readFromPhp(&fast_cgi);
			html = findStartHtml(full_result);
			auto htmlen = strlen(full_result) - (html - full_result);
			int i = 0;
			for (i = 0; i < htmlen; i++)
			{
				final_html[i] = *(html + i);
			}
			final_html[i] = '\0';
			AddHeader(".html");
		}
		else if (http_data_pack.IsDynamic())
		{
			FastCgiFun(const_cast<char *>("GET"), true);
		}
		else
		{
			/*普通的静态html文件，直接的发送给客户端即可*/
			AddHeader(std::string(http_data_pack.GetFileName()));
		}
	}
	void FastCgiFun(char *method, bool tag)
	{
		FastCgi_init(&fast_cgi);
		setRequestId(&fast_cgi, 1);
		startConnect(&fast_cgi);
		sendStartRequestRecord(&fast_cgi);
		std::cout << "----------------------------------------php 文件" << std::endl;

		printf("http_data_pack,Getfilename ==  %s\n", http_data_pack.GetFileName());

		sendParams(&fast_cgi, const_cast<char *>("SCRIPT_FILENAME"), http_data_pack.GetFileName());
		sendParams(&fast_cgi, const_cast<char *>("REQUEST_METHOD"), method);
		if (tag)
		{
			char Value[200];
			for (auto tmp : NameValue)
			{
				sprintf(Value, "%d", tmp.second);
				sendParams(&fast_cgi, const_cast<char *>(tmp.first.data()), Value);
			}
		}
		sendEndRequestRecord(&fast_cgi);
		FastCgi_finit(&fast_cgi);
	}
	void ResponsePost()
	{
	}

  private:
	/*http 解析类*/
	HttpParse http_data_pack;
	BaseSocket Sockfd;
	FastCgi_t fast_cgi;
	char final_html[WRITE_BUFFERSIZE] = {0};

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
