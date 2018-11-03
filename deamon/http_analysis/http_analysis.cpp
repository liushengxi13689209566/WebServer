/*************************************************************************
	> File Name: http_analysis.cpp
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年10月25日 星期四 17时33分40秒
 ************************************************************************/

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
using namespace std;

static const int BUFFER_SIZE = 4096;
/*主状态机的状态*/
enum CHECK_STATE
{
	CHECK_STATE_REQUESTLINE = 0,
	CHECK_STATE_HEADER,
	CHECK_STATE_CONTENT
};
/*有可能一行数据都没有一次性传输完毕，需要继续进行传输*/
enum LINE_STATUS
{
	LINE_OK = 0,
	LINE_BAD,
	LINE_NOT_ENOUGH
};
/**/
enum HTTP_CODE
{
	REQUEST_NOT_ENOUGH,
	GET_REQUEST,
	BAD_REQUEST,
	FORBIDDEN_REQUEST,
	SERVER_ERROR,
	CLOSED_CONNECTION
};
static const char *szret[] = {"I get a rigth result\n", "Something error\n"};

/*函数原型*/
HTTP_CODE parse_content(char *buffer, int &checked_index, CHECK_STATE &checkstate, int &read_index, int &start_line);
LINE_STATUS parse_line(char *buffer, int &checked_index, int &read_index);
HTTP_CODE parse_requestline(char *temp, CHECK_STATE &checkstate);
HTTP_CODE parse_headers(char *temp);

int main(int argc, char **argv)
{
	if (argc <= 2)
	{
		printf("use %s ip port\n", basename(argv[0]));
		return 0;
	}
	const char *ip = argv[1];
	int port = atoi(argv[2]);

	struct sockaddr_in address;
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;

	inet_pton(AF_INET, ip, &address.sin_addr);
	address.sin_port = htons(port);

	int listenfd = socket(PF_INET, SOCK_STREAM, 0);
	assert(listenfd >= 0);

	int ret = bind(listenfd, (struct sockaddr *)&address, sizeof(address));
	assert(ret != -1);

	ret = listen(listenfd, 5);
	assert(ret != -1);

	struct sockaddr_in client_address;
	socklen_t client_length = sizeof(client_address);
	int fd = accept(listenfd, (struct sockaddr *)&client_address, &client_length);
	if (fd < 0)
	{
		printf("errno == %d\n", errno);
	}
	else
	{
		char buffer[BUFFER_SIZE];
		memset(buffer, '\0', BUFFER_SIZE);
		int data_read = 0;
		int read_index = 0;	//已经读取了多少字节
		int checked_index = 0; //已经分析完了多少字节
		int start_line = 0;	//行在数据中的起始位置

		CHECK_STATE checkstate = CHECK_STATE_REQUESTLINE; //正在分析请求行
		while (1)
		{
			data_read = recv(fd, buffer + read_index, BUFFER_SIZE - read_index, 0);
			if (data_read == -1)
			{
				printf("read errod \n");
				break;
			}
			else if (data_read == 0)
			{
				printf("client closed \n");
				break;
			}
			read_index += data_read;
			HTTP_CODE result = parse_content(buffer, checked_index, checkstate, read_index, start_line);
			if (result == REQUEST_NOT_ENOUGH)
				continue;
			else if (result == GET_REQUEST) /*GET方法*/
			{
				send(fd, szret[0], strlen(szret[0]), 0);
				break;
			}
			else //发生错误或者处理其他方法
			{
				send(fd, szret[1], strlen(szret[1]), 0);
				break;
			}
		}
		close(fd);
	}
	close(listenfd);
	return 0;
}
/*分析　http　请求*/
HTTP_CODE parse_content(char *buffer, int &checked_index, CHECK_STATE &checkstate, int &read_index, int &start_line)
{
	LINE_STATUS linestatus = LINE_OK;		/*记录当前行的状态*/
	HTTP_CODE retcode = REQUEST_NOT_ENOUGH; /*记录 http 请求的处理结果*/

	/*主状态机，用于从　buffer 中取出所有完整的行*/
	while ((linestatus = parse_line(buffer, checked_index, read_index)) == LINE_OK)
	{
		char *temp = buffer + start_line;
		start_line = checked_index; /*下一行的起始位置*/
		switch (checkstate)
		{
			/*分析请求行*/
		case CHECK_STATE_REQUESTLINE:
			retcode = parse_requestline(temp, checkstate);
			if (retcode == BAD_REQUEST)
				return BAD_REQUEST;
			checkstate = CHECK_STATE_HEADER;
			break;

			/*分析头部字段*/
		case CHECK_STATE_HEADER:
			retcode = parse_headers(temp);
			if (retcode == BAD_REQUEST)
				return BAD_REQUEST;
			else if (retcode == GET_REQUEST)
				return GET_REQUEST;
			break;
		default:
			return SERVER_ERROR;
		}
	}
	if (linestatus == LINE_OK)
		return REQUEST_NOT_ENOUGH;
	else
		BAD_REQUEST;
}
/*分析请求行*/
HTTP_CODE parse_requestline(char *temp, CHECK_STATE &checkstate)
{
	char *url = strpbrk(temp, " \t");
	if (url == NULL)
		return BAD_REQUEST;
	*url++ = '\0';
	char *method = temp;
	if (strcasecmp(method, "GET") == 0)
		printf("request is GET!!!!\n");
	else
	{
		/*xxxxxxxxxxx*/
		return BAD_REQUEST;
	}
	url += strspn(url, " \t"); /*跳过分隔符*/

	char *version = strpbrk(url, " \t");
	if (version == NULL)
		return BAD_REQUEST;
	*version++ = '\0';
	version += strspn(version, " \t");

	/*这里可以处理一些http版本的一些信息*/

	/*检查url是否合法*/
	if (strncasecmp(url, "http://", 7) == 0)
	{
		url += 7;
		url = strchr(url, '/');
	}
	if (!url || url[0] != '/')
		return BAD_REQUEST;
	printf("url ==%s\n", url);
	return REQUEST_NOT_ENOUGH;
}
/*分析头部字段*/
HTTP_CODE parse_headers(char *temp)
{
	if (temp[0] == '\0')
		return GET_REQUEST;
	else if (strncasecmp(temp, "HOST:", 5) == 0)
	{
		temp += 5;
		temp += strspn(temp, " \t");
		printf("the host is :%s\n", temp);
	}
	else
	{
		printf("I can't handle the header\n");
	}
	return REQUEST_NOT_ENOUGH;
}

LINE_STATUS parse_line(char *buffer, int &checked_index, int &read_index)
{
	char temp; /*要分析的字节范围是(checked_index～(read_index-1))　*/
	for (; checked_index < read_index; ++checked_index)
	{
		temp = buffer[checked_index];
		if (temp == '\r')
		{
			if ((checked_index + 1) == read_index)
				return LINE_NOT_ENOUGH;
			else if (buffer[checked_index + 1] == '\n')
			{
				buffer[checked_index++] = '\0';
				buffer[checked_index++] = '\0';
				return LINE_OK;
			}
			return LINE_BAD;
		}
		else if (temp == '\n')
		{
			if ((checked_index > 1) && buffer[checked_index - 1] == '\r')
			{
				buffer[checked_index - 1] = '\0';
				buffer[checked_index++] = '\0';
				return LINE_OK;
			}
			return LINE_BAD;
		}
	}
	return LINE_NOT_ENOUGH;
}