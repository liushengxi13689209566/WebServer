/*************************************************************************
	> File Name: frist_cgi.cpp
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年07月19日 星期四 17时22分39秒
 ************************************************************************/
#include"myhead.h"
using namespace std;
int main(int argc ,char *argv[]) {
    if(argc <= 2 ){
    	cout << "usage : " << basename(argv[0]) << "   ip_address    port_number   " << endl ;
    	return 1;
    }
    const char *ip = argv[1] ;
    int port = atoi(argv[2]);

    struct sockaddr_in address ;
    bzero(&address,sizeof(address));
    address.sin_family =  AF_INET ;
    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_port =  htons(port) ;

    int socketfd =  socket(PF_INET,SOCK_STREAM, 0);
    assert( socked >= 0 ) ; //  right  expression 
    int ret = bind(socked,(struct sockaddr *)&address,sizeof(address));
    assert(ret !=  -1) ;

    ret = listen(socketfd,5);
    assert(ret != -1);

    struct sockaddr_in client ;
    socklen_t client_addrlength =  sizeof(client) ;
    int connfd =  accept(socked,(struct sockaddr *)&client,&client_addrlength);
    if(connfd <  0 ){
    	cout << " errno is :  " << errno  << endl ;
    }
    else {
    	cout << "liunvjfenvkbfbbbirbeub "<< endl ;
    	close(STDOUT_FILENO); //关闭标准输出文件描述符（STDOUT_FILENO == 1  ）
    	dup(connfd); //复制从conn_fd ,因为dup总是返回系统中最小的一个可用文件描述符，所以就是1 
        //这样一来，服务器输出到标准输出的东西就会输出到链接对应的socket上，而不是输出到服务器程序的终端上
    	printf(" abcd \n" );
    	close(connfd) ; 
    }
    close(socked);
    return 0;
}

