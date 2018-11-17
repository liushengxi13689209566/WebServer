/*************************************************************************
	> File Name: http_conn.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年10月27日 星期六 10时23分18秒
 ************************************************************************/

#ifndef _HTTP_CONN_H
#define _HTTP_CONN_H

#include <sys/stat.h>
#include <sys/sendfile.h>
#include <string> 

#include "./base_function.h"

const char *doc_root = "./index.html";
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
	/* http 请求方法，目前只支持 get */
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
		GET_REQUEST,		/*读取到一个完整请求*/
		POST_REQUEST,
		BAD_REQUEST,	   /*请求有语法错误*/
		FORBIDDEN_REQUEST, /*客户对该资源无权限*/
		NO_RESOURCE,	   /*无此资源，发送404页面*/
		FILE_REQUEST,
		SERVER_ERROR,	 /*服务器出错*/
		CLOSED_CONNECTION /*客户端关闭了连接*/
	};
	/*行的读取状态：有可能一行数据都没有一次性传输完毕，需要继续进行传输*/
	enum LINE_STATUS
	{
		LINE_OK = 0,
		LINE_BAD,
		LINE_NOT_ENOUGH
	};

  public:
	http_conn()
	{
		init();
	}
	~http_conn() {}

  public:
	void init();
	bool read();
	bool write();
	/*write 调用的函数*/
	bool send_header();

	/*处理客户请求*/
	void process();

  private:
	void modfd(int ev);
	void http_close_conn();
	HTTP_CODE process_read();

  private:
	/*process_read 所使用的函数*/
	LINE_STATUS parse_line();
	HTTP_CODE parse_request_line(char *line);
	HTTP_CODE parse_headers(char *line);
	HTTP_CODE parse_content(char *line);
	HTTP_CODE do_get_request();

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
	/*读缓冲区*/
	char http_read_buf[READ_BUFFERSIZE] = {0};
	/*读缓冲区中已经读入的最后一个字节的位置*/
	int http_end_index = 0;
	/*正在分析的字符在读缓冲区中的位置*/
	int http_checked_index = 0;
	/*正在解析的行的起始位置*/
	int http_start_line = 0;
	/*http 头部缓冲区*/
	char http_header_buf[HEADER_BUFFERSIZE] = {0};
	/*构造写缓冲*/
	int http_header_index = 0;
	/*主状态机所处的状态*/
	CHECK_STATE http_check_state = CHECK_STATE_REQUESTLINE;
	/*请求的方法*/
	METHOD http_method = GET;

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

	/*目标文件的状态，通过他判断是否需要发送　404　，以及文件大小等等*/
	struct stat http_file_stat;
};

int http_conn::http_epollfd = -1;

#endif
