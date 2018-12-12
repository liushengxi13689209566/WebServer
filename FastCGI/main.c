
#include <stdio.h>
#include <stdlib.h>
#include "fcgi.h"
#include <sys/types.h>
#include <sys/socket.h>

int main()
{
    FastCgi_t *c;
    c = (FastCgi_t *)malloc(sizeof(FastCgi_t));
    FastCgi_init(c);
    setRequestId(c, 1); /*1 用来表明此消息为请求开始的第一个消息*/
    startConnect(c);

    printf("链接是成功的\n");

    sendStartRequestRecord(c);
    sendParams(c, "SCRIPT_FILENAME", "/home/Shengxi-Liu/WebServer/wwwroot/php/Operation.php");
    sendParams(c, "REQUEST_METHOD", "POST");
    sendParams(c, "CONTENT_LENGTH", "5");
    sendParams(c, "CONTENT_TYPE", "application/x-www-form-urlencoded");

    sendEndRequestRecord(c);
    FCGI_Header t = makeHeader(FCGI_STDIN, c->requestId_, 5, 0);
    //制造头为了发 body
    send(c->sockfd_, &t, sizeof(t), 0);
    send(c->sockfd_, "a=666", 5, 0);

    FCGI_Header endHeader;
    endHeader = makeHeader(FCGI_STDIN, c->requestId_, 0, 0);

    send(c->sockfd_, (char *)&endHeader, FCGI_HEADER_LEN, 0);

    printf("end-----\n");
    readFromPhp(c);

    FastCgi_finit(c);
    return 0;
}

// ret = sendParams(c, "REQUEST_METHOD", "GET");
// if (ret == 1)
//     printf("成功\n");

// // ret = sendParams(c, "QUERY_STRING", "a=20&b=10&c=5&d=6");
// ret = sendParams(c, "QUERY_STRING", "a=2666");

// if (ret == 1)
//     printf("成功\n");