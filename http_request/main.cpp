/*************************************************************************
	> File Name: main.cpp
	> Author: Liu Shengxi
	> Mail: 13689209566@163.com
	> Created Time: 2018年07月23日 星期一 11时14分17秒
 ************************************************************************/

#include<iostream>
#include<unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>
using namespace std;
#define BUFFER_SIZE 4096
/*主状态机的两种状态，分别表示 ：当前正在分析请求行，当前正在分析头部数据*/
enum CHECK_STATE { CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER   };
/*读取到一个完整的行，行出错，和行数据不完整 */
enum LINE_STATUS {LINE_OK, LINE_BAD , LINE_OPEN };

enum HTTP_CODE {NO_REQUEST, GET_REQUEST, BAD_REQUEST,
                FORBIDDEN_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION
               };
static const char *szret[] = {"I get a corre  result\n", "Something Wrong !!\n"} ;

//分析请求行
HTTP_CODE parse_requestline(char * temp, CHECK_STATE& checkstate ) {
		//cout << "进入parse_requestline 函数 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<  endl ;
	char * url = strpbrk(temp, " \t"); //如果请求行中没有空白符号和"\t",则HTTP请求有问题
	//strpbrk函数比较字符串str1和str2中是否有相同的字符，如果有，则返回该字符在str1中的位置的指针。
	if ( !url )  return BAD_REQUEST ;
	*url++ = '\0';
	char *method = temp ;
	if (strcasecmp(method, "GET") == 0 ) { //目前只支持 GET 方法
		cout << "This is a GET " << endl ;
	}
	else
		return BAD_REQUEST ;
	url += strspn(url, " \t") ;
	char *version = strpbrk(url, " \t");
	if (!version ) return BAD_REQUEST ;
	*version++ = '\0';
	version += strspn(version, " \t") ;
	/*径支持http/1.1*/
	if (strcasecmp(version, "http/1.1") != 0 ) { //目前只支持 GET 方法
		return BAD_REQUEST;
	}
	if (strncasecmp(url, "http://", 7) == 0 ) {
		url += 7 ;
		url = strchr(url, '/');
	}
	if ( !url || url[0] != '/')
		return BAD_REQUEST;
	cout << "URL == " <<  url << endl ;
	checkstate = CHECK_STATE_HEADER ; //转到头部数据的处理
	return NO_REQUEST ;
}
/*分析头部数据*/
HTTP_CODE parse_headers(char *temp) {
		//cout << "进入parse_header 函数 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<  endl ;
	if (temp[0] == '\0')
		return GET_REQUEST ;
	else if (strncasecmp(temp, "Host:", 5) == 0 ) { //处理一个host头部数据
		temp += 5 ;
		temp += strspn(temp, " \t");
		cout << "The request host is " <<  temp << endl ;
	}
	else { //其他字段不处理
		cout << "其他字段不处理 " <<  endl ;
	}
	return NO_REQUEST ;
}
/*从状态机，用于解析出一行内容*/
LINE_STATUS parse_line(char *buffer, int &checked_index, int &read_index) {
		//cout << "进入parse_line 函数 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<  endl ;
	char temp ;
	for ( ; checked_index < read_index ; ++checked_index ) {
		//cout << "into   loop  ********************* "<< endl ;
		temp =  buffer[checked_index];//得到当前要分析的字节
		if (temp ==  '\r') { // \r是回车符,\n是换行符,读取到一个完整的行
			if ( (checked_index + 1 ) ==  read_index )
				return LINE_OPEN ;
			else if (buffer[checked_index + 1 ] == '\n') {
				buffer[ checked_index++ ] = '\0';
				buffer[ checked_index++ ] = '\0';
				return LINE_OK ;
			}
			return LINE_BAD ; //否则的话，说明客户端发送的http请求存在问题
		}
		else if ( temp == '\n') {
			if ( (checked_index > 1 ) &&  buffer[checked_index-1] == '\r' ) {
				buffer[ checked_index - 1 ] = '\0';
				buffer[ checked_index++ ] = '\0';
				return LINE_OK ;
			}
			return LINE_BAD ;
		}
	}


	cout << "go out   loop  ********************* "<< endl ;


	return LINE_OPEN ;//没有遇到‘\r’字符，则表示还需要继续读取客户端数据才能分析
}
/*分析http请求的入口函数*/
HTTP_CODE parse_content(char *buffer, int &checked_index, CHECK_STATE& checkstate,
                        int &read_index, int &start_line) {


	//cout << "进入parse_content 函数 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<  endl ;
	LINE_STATUS linestatus  = LINE_OK ;// 记录当前行的读取状态
	HTTP_CODE retcode = NO_REQUEST ;//记录http请求的处理结果

	while ((linestatus = parse_line(buffer, checked_index, read_index)) == LINE_OK ) {
		//cout << "parse_line fun end !!!!!!!!!!!!!!!!!!!!!!!!!!" << endl ;
		char *temp = buffer + start_line;
		start_line = checked_index ;
		switch ( checkstate ) {
			case CHECK_STATE_REQUESTLINE:
			{
				retcode = parse_requestline(temp, checkstate);
				if (retcode  ==  BAD_REQUEST)
					return BAD_REQUEST  ;
				break;
			}
			case CHECK_STATE_HEADER:
			{
				retcode = parse_headers(temp);
				if (retcode  ==  BAD_REQUEST)
					return BAD_REQUEST  ;
				else if (retcode == GET_REQUEST )
					return GET_REQUEST  ;
				break ;
			}
			default :
			{
				return INTERNAL_ERROR ;
			}
		}
	}
	//还没有读取到一个完整的行，则表示还需要继续读取客户端数据才能进一步分析
	if (linestatus == LINE_OPEN )
		return NO_REQUEST ;
	else
		return BAD_REQUEST ;
}
int main(int argc, char *argv[]) {
	if (argc <= 2 ) {
		cout << "usage : " << basename(argv[0]) << "   ip_address    port_number   " << endl ;
		return 1;
	}
	const char *ip = argv[1] ;
	int port = atoi(argv[2]);

	struct sockaddr_in address ;
	bzero(&address, sizeof(address));
	address.sin_family =  AF_INET ;
	inet_pton(AF_INET, ip, &address.sin_addr);
	address.sin_port =  htons(port) ;

	int socketfd =  socket(PF_INET, SOCK_STREAM, 0);
	assert( socketfd >= 0 ) ; //  right  expression

    int optval  =  1  ; //设置该套接字使之可以重新绑定端口
    if(setsockopt(socketfd,SOL_SOCKET,SO_REUSEADDR,(int *)&optval,sizeof(int))    < 0)
        printf("setsocketopt failed \n ");

	int ret = bind(socketfd, (struct sockaddr *)&address, sizeof(address));
	assert(ret !=  -1) ;

	ret = listen(socketfd, 5);
	assert(ret != -1);

	struct sockaddr_in client_address  ;
	socklen_t client_addrlength =  sizeof(client_address) ;
	int connfd =  accept(socketfd, (struct sockaddr *)&client_address, &client_addrlength);
	if (connfd <  0 ) {
		cout << " errno is :  " << errno  << endl ;
	}
	else {
		char buffer[BUFFER_SIZE];
		memset(buffer, '\0', BUFFER_SIZE);
		int data_read = 0 ;
		int read_index = 0 ; //已经读取的客户字节数
		int checked_index = 0; //已经分析完的字节数量
		int start_line = 0 ; //行在buffer中的起始位置
		CHECK_STATE checkstate =  CHECK_STATE_REQUESTLINE ;
		while (1) {
			data_read =  recv(connfd, buffer + read_index, BUFFER_SIZE - read_index , 0 ) ;
			if (data_read == -1) {
				cout << "reading failed " << endl ;
				break ;
			}
			else if (data_read == 0 ) {
				cout << "client has closeed connection " <<   endl ;
				break;
			}
			read_index += data_read ;
			//分析目前已经获得的所有客户端数据
			HTTP_CODE result =  parse_content(buffer, checked_index, checkstate, read_index, start_line) ;
			if (result == NO_REQUEST )
				continue ; //得到的http请求不完整
			else if (result ==  GET_REQUEST ) { //得到了一个完整的http请求
				send(connfd, szret[0], strlen(szret[0]), 0);
				break ;
			}
			else {
				send(connfd, szret[1], strlen(szret[1]), 0);
				break ;
			}
		}
		close(connfd);
	}
	close(socketfd);
	return 0;
}

/* 实现效果：
$ ./a.out 127.0.0.1 4589
The request method is GET
The request URL is: /
the request host is: 127.0.0.1:4589
I can not handle this header
I can not handle this header
I can not handle this header
I can not handle this header
I can not handle this header
I can not handle this header
*/