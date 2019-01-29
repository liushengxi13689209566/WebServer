

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
    sendStartRequestRecord(c);

    sendParams(c, "SCRIPT_FILENAME", "/home/Shengxi-Liu/WebServer/wwwroot/php/Operation.php");
    sendParams(c, "REQUEST_METHOD", "POST");
    sendParams(c, "CONTENT_LENGTH", "17"); //　17 为body的长度 !!!!
    sendParams(c, "CONTENT_TYPE", "application/x-www-form-urlencoded");

    sendEndRequestRecord(c);

    /*FCGI_Header makeHeader(int type, int requestId,
                       int contentLength, int paddingLength)*/
    //制造头为了发 body
    FCGI_Header t = makeHeader(FCGI_STDIN, c->requestId_, 17, 0); //　17 为body的长度 !!!!
    send(c->sockfd_, &t, sizeof(t), 0);

    /*发送正式的 body */
    send(c->sockfd_, "a=20&b=10&c=5&d=6", 17, 0); //　17 为body的长度 !!!!

    //制造头告诉　body　结束　
    FCGI_Header endHeader;
    endHeader = makeHeader(FCGI_STDIN, c->requestId_, 0, 0);
    send(c->sockfd_, &endHeader, sizeof(endHeader), 0);

    printf("end-----\n");

    readFromPhp(c);

    FastCgi_finit(c);
    return 0;
}
