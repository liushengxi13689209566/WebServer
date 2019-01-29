

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
    sendParams(c, "REQUEST_METHOD", "GET");
    sendParams(c, "CONTENT_LENGTH", "0"); //　0 表示没有　body
    sendParams(c, "CONTENT_TYPE", "text/html");
    sendParams(c, "QUERY_STRING", "a=20&b=10&c=5&d=6");

    sendEndRequestRecord(c);

    printf("end-----\n");

    readFromPhp(c);

    FastCgi_finit(c);
    return 0;
}