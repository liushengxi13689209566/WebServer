/*************************************************************************
	> File Name: test_client.cpp
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年07月19日 星期四 20时43分05秒
 ************************************************************************/

#include"myhead.h"
using namespace std ;
int main(void)
{
	int conn_fd ;  
    struct sockaddr_in  server_address ;
    bzero( &server_address, sizeof( server_address ) );

    const char *ip ="127.0.0.1" ;

    server_address.sin_family = AF_INET;
    inet_pton( AF_INET, ip, &server_address.sin_addr );
    const int port =  7894 ;
    server_address.sin_port = htons( port );

    conn_fd = socket( AF_INET, SOCK_STREAM, 0 );
    assert( conn_fd >= 0 );

    if ( connect( conn_fd, ( struct sockaddr* )&server_address, sizeof( server_address ) ) < 0 )
    {
        printf( "connection failed\n" );
        close( conn_fd );
    }
    char str[1001];
    while(1){
    	int tt =recv(conn_fd,&str,sizeof(str),0);
    	if(tt == 0 )
    		break ;
    	printf("%s\n",str);
    }
    close(conn_fd);
    return  0 ;
}


